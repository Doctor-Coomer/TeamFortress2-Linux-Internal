#ifndef CLIENT_STATE_HPP
#define CLIENT_STATE_HPP

#include <cstdint>

struct ClockDriftMgr {
    float m_ClockOffsets[16];
    int m_iCurClockOffset;
    int m_nServerTick;
    int m_nClientTick;
};

class ClientState {
public:
    uint8_t pad0[24];
    int m_Socket;
    void *m_NetChannel; //INetChannel
    unsigned int m_nChallengeNr;
    double m_flConnectTime;
    int m_nRetryNumber;
    char m_szRetryAddress[260];
    char *m_sRetrySourceTag;
    int m_retryChallenge;
    int m_nSignonState;
    double m_flNextCmdTime;
    int m_nServerCount;
    uint64_t m_ulGameServerSteamID;
    int m_nCurrentSequence;
    ClockDriftMgr m_ClockDriftMgr;
    int m_nDeltaTick;
    bool m_bPaused;
    float m_flPausedExpireTime;
    int m_nViewEntity;
    int m_nPlayerSlot;
    char m_szLevelFileName[128];
    uint8_t pad1[132];
    char m_szLevelBaseName[128];
    uint8_t pad2[132];
    int m_nMaxClients;
    void *m_pEntityBaselines[2][0x800]; //PackedEntity
    uint8_t pad3[2068];
    void *m_StringTableContainer;
    bool m_bRestrictServerCommands;
    bool m_bRestrictClientCommands;
    uint8_t pad4[106];
    bool insimulation;
    int oldtickcount;
    float m_tickRemainder;
    float m_frameTime;
    int lastoutgoingcommand;
    int chokedcommands;
    int last_command_ack;
    int command_ack;
    int m_nSoundSequence;
    bool ishltv;
    bool isreplay;
    uint8_t pad5[278];
    int demonum;
    char *demos[32];
    uint8_t pad6[344184];
    bool m_bMarkedCRCsUnverified;
};

inline static ClientState *client_state;

#endif
