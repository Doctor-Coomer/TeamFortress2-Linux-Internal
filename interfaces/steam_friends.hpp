#ifndef STEAM_FRIENDS_HPP
#define STEAM_FRIENDS_HPP

// https://github.com/rei-2/Amalgam/blob/c1c6bf64d739538b48a301ddc5e1a988cb9b479c/Amalgam/src/SDK/Definitions/Steam/SteamClientPublic.h#L224
enum EAccountType {
    k_EAccountTypeInvalid = 0,
    k_EAccountTypeIndividual = 1, // single user account
    k_EAccountTypeMultiseat = 2, // multiseat (e.g. cybercafe) account
    k_EAccountTypeGameServer = 3, // game server account
    k_EAccountTypeAnonGameServer = 4, // anonymous game server account
    k_EAccountTypePending = 5, // pending
    k_EAccountTypeContentServer = 6, // content server
    k_EAccountTypeClan = 7,
    k_EAccountTypeChat = 8,
    k_EAccountTypeConsoleUser = 9, // Fake SteamID for local PSN account on PS3 or Live account on 360, etc.
    k_EAccountTypeAnonUser = 10,

    // Max of 16 items in this field
    k_EAccountTypeMax
};

// https://github.com/rei-2/Amalgam/blob/c1c6bf64d739538b48a301ddc5e1a988cb9b479c/Amalgam/src/SDK/Definitions/Steam/ISteamFriends.h#L68
enum EFriendFlags {
    k_EFriendFlagNone = 0x00,
    k_EFriendFlagBlocked = 0x01,
    k_EFriendFlagFriendshipRequested = 0x02,
    k_EFriendFlagImmediate = 0x04, // "regular" friend
    k_EFriendFlagClanMember = 0x08,
    k_EFriendFlagOnGameServer = 0x10,
    // k_EFriendFlagHasPlayedWith	= 0x20,	// not currently used
    // k_EFriendFlagFriendOfFriend	= 0x40, // not currently used
    k_EFriendFlagRequestingFriendship = 0x80,
    k_EFriendFlagRequestingInfo = 0x100,
    k_EFriendFlagIgnored = 0x200,
    k_EFriendFlagIgnoredFriend = 0x400,
    // k_EFriendFlagSuggested		= 0x800,	// not used
    k_EFriendFlagChatMember = 0x1000,
    k_EFriendFlagAll = 0xFFFF,
};

// https://github.com/rei-2/Amalgam/blob/c1c6bf64d739538b48a301ddc5e1a988cb9b479c/Amalgam/src/SDK/Definitions/Steam/SteamUniverse.h#L15
enum EUniverse {
    k_EUniverseInvalid = 0,
    k_EUniversePublic = 1,
    k_EUniverseBeta = 2,
    k_EUniverseInternal = 3,
    k_EUniverseDev = 4,
    // k_EUniverseRC = 5,				// no such universe anymore
    k_EUniverseMax
};

// https://github.com/rei-2/Amalgam/blob/c1c6bf64d739538b48a301ddc5e1a988cb9b479c/Amalgam/src/SDK/Definitions/Steam/SteamClientPublic.h#L467
class SteamID {
public:
    SteamID(int unAccountID, unsigned int unAccountInstance, EUniverse eUniverse, EAccountType eAccountType) {
        InstancedSet(unAccountID, unAccountInstance, eUniverse, eAccountType);
    }

    void InstancedSet(int unAccountID, int unInstance, EUniverse eUniverse, EAccountType eAccountType) {
        m_steamid.m_comp.m_unAccountID = unAccountID;
        m_steamid.m_comp.m_EUniverse = eUniverse;
        m_steamid.m_comp.m_EAccountType = eAccountType;
        m_steamid.m_comp.m_unAccountInstance = unInstance;
    }

    union SteamID_t {
        struct SteamIDComponent_t {
            int m_unAccountID: 32; // unique account identifier
            unsigned int m_unAccountInstance: 20; // dynamic instance ID
            unsigned int m_EAccountType: 4;
            // type of account - can't show as EAccountType, due to signed / unsigned difference
            EUniverse m_EUniverse: 8; // universe this account belongs to
        } m_comp;

        unsigned long m_unAll64Bits;
    } m_steamid;
};

class SteamFriends {
public:
    bool has_friend(SteamID steam_friend_id, int friend_flags) {
        void **vtable = *reinterpret_cast<void ***>(this);

        bool (*has_friend_fn)(void *, SteamID, int) = (bool (*)(void *, SteamID, int)) vtable[17];

        return has_friend_fn(this, steam_friend_id, friend_flags);
    }

    bool is_friend(int friend_id) {
        return has_friend({friend_id, 1, k_EUniversePublic, k_EAccountTypeIndividual}, k_EFriendFlagImmediate);
    }
};

inline static SteamFriends *steam_friends;

#endif
