#ifndef STEAM_CLIENT_HPP
#define STEAM_CLIENT_HPP

class SteamClient {
public:
    /* Steam Pipe Handle */
    int create_steam_pipe(void) {
        void **vtable = *reinterpret_cast<void ***>(this);

        int (*create_steam_pipe_fn)(void *) = (int (*)(void *)) vtable[0];

        return create_steam_pipe_fn(this);
    }

    /* Steam User Handle */
    int connect_to_global_user(int steam_pipe) {
        void **vtable = *reinterpret_cast<void ***>(this);

        int (*connect_to_global_user_fn)(void *, int) = (int (*)(void *, int)) vtable[2];

        return connect_to_global_user_fn(this, steam_pipe);
    }

    /* Steam Friends Interface */
    void *get_steam_friends_interface(int steam_user, int steam_pipe, const char *version) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void * (*get_steam_friends_interface_fn)(void *, int, int, const char *) = (void* (*)(
            void *, int, int, const char *)) vtable[8];

        return get_steam_friends_interface_fn(this, steam_user, steam_pipe, version);
    }
};

inline static SteamClient *steam_client;

#endif
