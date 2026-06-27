# Client Synchronization

## Introduction

A 'lag start' is when one or more clients begin the race at different times. Lag starts are undesirable and should be avoided. To avoid this, clients must synchronize with one another before the countdown begins.

In vanilla MKW's peer-to-peer model, players calculate each other's latency during the voting screen. This is then factored into determining when to start the countdown. This approach doesn't translate well to mkw-server's client-server architecture, so client synchronization must be rewritten.

## Approach

The approach is based on mkw-server being the new authority of the starting countdown. Hopefully this can be simple, effective, and has few side-effects (such as incompatibilities with in-game structures).

When clients load into the race and complete the intro camera pan, they will send a message to mkw-server informing they are ready for the countdown to start.

Once mkw-server has heard back from all clients, it will broadcast a message telling clients to start the countdown.

## Latency

Since messages broadcasted at the same time will be received at different times, the start message must be sent to each client at a different time based on their latency.

mkw-server sends a timestamped packet to clients over UDP and the client immediately sends it back. When mkw-server receives it, the one-way latency is approximated as half the elapsed time since the packet was sent. This is repeated several times and averaged for a better approximation.

Like vanilla, latency calculation will still happen during the voting screen.

## Sending Ready

A client will send a ready message to mkw-server two seconds after they have loaded into the race. This gives enough time for the intro camera pan to complete.

mkw-server will send an ack in response to a ready message. This tells the client to stop sending ready messages.

## Sending Start

mkw-server will begin sending start messages to clients once all clients have sent a ready message. Before sending, mkw-server will calculate the max latency of all clients and each player's latency difference from the max latency. This difference is the amount of time mkw-server will wait before sending a start to that client.

mkw-server will send a start message to a client until it [detects](#rh1recordtimesincecountdown) that the client has started the countdown.

## Implementation Notes

### `RH1Record.timeSinceCountdown`

Timer that starts counting up when the countdown begins. mkw-server can use this to detect when a client has started the countdown.

### `RaceManager::m_canCountdownStart`

When set to `true`, MKW will begin the countdown timer. When a client receives a start message from mkw-server, the client will set this flag.

### `DWCi_GT2UnrecognizedMessageCallback()`

Function currently hooked to receive Race packets from mkw-server. An addition will be made to receive ping, start, and ack messages from mkw-server.

### `MiscPacketHandler::updateAsRacer()`

This function sets `RaceManager::m_canCountdownStart` in vanilla. This instruction will be nopped.
