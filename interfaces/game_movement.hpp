#ifndef GAME_MOVEMENT_HPP
#define GAME_MOVEMENT_HPP

#include "../vec.hpp"

class Player;

class MoveData {
public:
    bool m_bFirstRunOfFunctions: 1;
    bool m_bGameCodeMovedPlayer: 1;

    int m_nPlayerHandle; // edict index on server, client entity handle on client

    int m_nImpulseCommand; // Impulse command issued.
    Vec3 m_vecViewAngles; // Command view angles (local space)
    Vec3 m_vecAbsViewAngles; // Command view angles (world space)
    int m_nButtons; // Attack buttons.
    int m_nOldButtons; // From host_client->oldbuttons;
    float m_flForwardMove;
    float m_flOldForwardMove;
    float m_flSideMove;
    float m_flUpMove;

    float m_flMaxSpeed;
    float m_flClientMaxSpeed;

    // Variables from the player edict (sv_player) or entvars on the client.
    // These are copied in here before calling and copied out after calling.
    Vec3 m_vecVelocity; // edict::velocity		// Current movement direction.
    Vec3 m_vecAngles; // edict::angles
    Vec3 m_vecOldAngles;

    // Output only
    float m_outStepHeight; // how much you climbed this move
    Vec3 m_outWishVel; // This is where you tried
    Vec3 m_outJumpVel; // This is your jump velocity

    // Movement constraints	(radius 0 means no constraint)
    Vec3 m_vecConstraintCenter;
    float m_flConstraintRadius;
    float m_flConstraintWidth;
    float m_flConstraintSpeedFactor;

    void SetAbsOrigin(const Vec3 &vec);

    const Vec3 &GetAbsOrigin() const;

private:
    Vec3 m_vecAbsOrigin; // edict::origin
};


class GameMovement {
public:
    void process_movement(Player *player, MoveData *move) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*process_movement_fn)(void *, Player *, MoveData *) = (void (*)(void *, Player *, MoveData *)) vtable[1];

        process_movement_fn(this, player, move);
    }
};

inline static GameMovement *game_movement;

#endif
