#include "DWCTransport.h"

#include <sp/net/mkw_server/MKWServer.h>

#include <game/net/MiscPacketHandler.h>

#include <string.h>

#define START_MESSAGE "ST"
#define PING_MESSAGE "PI"
#define READY_ACK_MESSAGE "RA"

bool isReadyAckPacket(const u8 *message, s32 len) {
    return len == 2 &&
            strncmp((const char *)message, READY_ACK_MESSAGE, strlen(READY_ACK_MESSAGE)) == 0;
}

bool isStartPacket(const u8 *message, s32 len) {
    return len == 2 && strncmp((const char *)message, START_MESSAGE, strlen(START_MESSAGE)) == 0;
}

bool isPingPacket(const u8 *message, s32 len) {
    return len == 2 && strncmp((const char *)message, PING_MESSAGE, strlen(PING_MESSAGE)) == 0;
}

BOOL DWCi_GT2UnrecognizedMessageCallback(GT2Socket socket, u32 ip, u16 port, const u8 *message,
        s32 len) {
    if (message == NULL || len == 0) {
        SP_LOG("GT2 Unrecognized : Null message or zero length.");
        return GT2False;
    }

    if (verifySearchIdMagic(message, len)) {
        handleSearchIdPacket(message, len);
        return GT2True;
    }

    if (isReadyAckPacket(message, len)) {
        SP_LOG("Received ready ACK packet from mkw-server!");
        MiscPacketHandler_setAckReady();
        return GT2True;
    }

    if (isStartPacket(message, len)) {
        SP_LOG("Received Start packet from mkw-server!");
        startCountdown();
        return GT2True;
    }

    if (isPingPacket(message, len)) {
        sendPong();
        return GT2True;
    }

    MKWServerPacketType messageType = message[0];
    switch (messageType) {
    case MKW_SERVER_RACE_PACKET:
        if (s_dwcTransport == NULL || s_dwcTransport->userRecvCallback == NULL) {
            return FALSE;
        }

        // the aid is in offset 0x1
        u8 aid = message[1];

        s_dwcTransport->userRecvCallback(aid, message, len);
        return TRUE;

    default:
        return REPLACED(DWCi_GT2UnrecognizedMessageCallback)(socket, ip, port, message, len);
    }
    return FALSE;
}
