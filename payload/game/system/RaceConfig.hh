#pragma once

#include <Common.hh>

#include "game/system/Mii.hh"
#include "game/util/Registry.hh"

#include <array>

// Place-holder.
inline constexpr u8 g_playerIdToReplay = 0;
inline constexpr u8 g_localPlayerIdToReplay = 0;

namespace System {

/// Configuration for the current race.
///
/// For configuration over a set of VS/BT races, see GlobalContext.
class RaceConfig {
public:
    RaceConfig();
    virtual ~RaceConfig();
    virtual void dt(s32 type);

    struct Player {
        u8 _00[0x08 - 0x00];
        Registry::Vehicle vehicle;
        Registry::Character character;
        Registry::PlayerType type;
        Mii mii; // TODO: Read from replay file.
        Registry::Team team;
        Registry::Controller controller;
        u8 _d4[0xd8 - 0xd4];
        u16 points;
        u8 _da[0xde - 0xda];
        u16 rankIconId;
        u8 _e0[0xe1 - 0xe0];
        u8 startingSpot;
        u8 _e2[0xe4 - 0xe2];
        u32 rating;
        u8 _e8[0xf0 - 0xe8];
    };
    static_assert(sizeof(Player) == 0xf0);

    struct Scenario {
        bool isOnline() const;
        bool isOnlineVS() const;
        bool isOnlineBattle() const;
        u8 _000[0x004 - 0x000];
        u8 playerCount;
        u8 hudCount;
        u8 localPlayerCount;
        u8 _007[0x008 - 0x007];
        std::array<Player, 12> players;
        Registry::Course course;
        Registry::EngineClass engineClass;
        Registry::GameMode gameMode;
        u8 _b54[0xb58 - 0xb54];
        Registry::BattleType battleType;
        u8 _b5c[0xb73 - 0xb5c];
        Registry::ModeFlags modeFlags : 8;
        u32 seed1;
        u32 seed2;
        u8 _b7c[0xbf0 - 0xb7c];
    };
    static_assert(sizeof(Scenario) == 0xbf0);

    Scenario &raceScenario();

    // 0x805302c4
    REPLACE void initRace();
    void REPLACED(initRace)();

    // Sets up mkw-server replays from the replay file.
    void initReplay();

    static RaceConfig *Instance();

private:
    u8 _0004[0x0020 - 0x0004];

public:
    Scenario m_raceScenario;
    Scenario m_menuScenario;
    u8 _1800[0x73f0 - 0x1800];

    static RaceConfig *s_instance;
};
static_assert(sizeof(RaceConfig) == 0x73f0);

} // namespace System
