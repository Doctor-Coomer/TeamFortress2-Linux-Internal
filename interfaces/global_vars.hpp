#ifndef GLOBAL_VARS_HPP
#define GLOBAL_VARS_HPP

class GlobalVars {
public:
    float realtime;
    int framecount;
    float absolute_frametime;
    float curtime;
    float frametime;
    int max_clients;
    int tickcount;
    float interval_per_tick;
    float interpolation_amount;
    int sim_ticks_this_frame;
    int network_protocol;
    void *save_data; //CSaveRestoreData
    bool client;
    int nTimestampNetworkingBase;
    int nTimestampRandomizeWindow;
};

inline static GlobalVars *global_vars;

#endif
