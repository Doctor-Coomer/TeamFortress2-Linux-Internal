#ifndef PREDICTION_HPP
#define PREDICTION_HPP

#include "../vec.hpp"

class MoveHelper;
class MoveData;
class Player;
struct user_cmd;

// https://github.com/rei-2/Amalgam/blob/f246c3dbbe9fc82db015087b752ac91db65a107e/Amalgam/src/SDK/Definitions/Interfaces/Prediction.h#L9
class IPrediction {
public:
    virtual ~IPrediction(void) {
    };

    virtual void init(void) = 0;

    virtual void shutdown(void) = 0;

    virtual void update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;

    virtual void pre_entity_packet_received(int commands_acknowledged, int current_world_update_packet) = 0;

    virtual void post_entity_packet_received(void) = 0;

    virtual void post_network_data_received(int commands_acknowledged) = 0;

    virtual void on_received_uncompressed_packet(void) = 0;

    virtual void get_view_origin(Vec3 &org) = 0;

    virtual void set_view_origin(Vec3 &org) = 0;

    virtual void get_view_angles(Vec3 &ang) = 0;

    virtual void set_view_angles(Vec3 &ang) = 0;

    virtual void get_local_view_angles(Vec3 &ang) = 0;

    virtual void set_local_view_angles(Vec3 &ang) = 0;
};

class Prediction : public IPrediction {
public:
    virtual ~Prediction() {
    };

    virtual void init() = 0;

    virtual void shutdown() = 0;

    virtual void update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;

    virtual void on_received_uncompressed_packet() = 0;

    virtual void pre_entity_packet_received(int commands_acknowledged, int current_world_update_packet) = 0;

    virtual void post_entity_packet_received() = 0;

    virtual void post_network_data_received(int commands_acknowledged) = 0;

    virtual bool is_in_prediction() = 0;

    virtual bool is_first_time_predicted() = 0;

    virtual int get_incoming_packet_number() = 0;

    virtual void get_view_origin(Vec3 &org) = 0;

    virtual void set_view_origin(Vec3 &org) = 0;

    virtual void get_view_angles(Vec3 &ang) = 0;

    virtual void set_view_angles(Vec3 &ang) = 0;

    virtual void get_local_view_angles(Vec3 &ang) = 0;

    virtual void set_local_view_angles(Vec3 &ang) = 0;

    virtual void run_command(Player *player, user_cmd *ucmd, MoveHelper *moveHelper) = 0;

    virtual void setup_move(Player *player, user_cmd *ucmd, MoveHelper *pHelper, MoveData *move) = 0;

    virtual void finish_move(Player *player, user_cmd *ucmd, MoveData *move) = 0;

    virtual void setideal_pitch(Player *player, const Vec3 &origin, const Vec3 &angles, const Vec3 &viewheight) = 0;

    virtual void _update(bool received_new_world_update, bool validframe, int incoming_acknowledged,
                         int outgoing_command) = 0;

public:
    int last_ground;
    bool in_prediction;
    bool first_time_predicted;
    bool old_cl_predict_value;
    bool engine_paused;
    int previous_start_frame;
    int commands_predicted;
    int server_commands_acknowledged;
    int previous_ack_had_errors;
    int incoming_packet_number;
    float ideal_pitch;
};

inline static Prediction *prediction;

#endif
