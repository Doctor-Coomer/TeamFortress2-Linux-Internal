#ifndef SNIPER_DOT_HPP
#define SNIPER_DOT_HPP

class SniperDot {
public:
    float get_charge_start_time(void) {
        return *reinterpret_cast<float *>(this + 0x7E0);
    }
};

#endif
