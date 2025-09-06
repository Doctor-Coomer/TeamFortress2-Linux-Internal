#ifndef CONVAR_HPP
#define CONVAR_HPP

class Convar {
public:
    void set_int(int value) {
        *reinterpret_cast<int *>(this + 0x58) = value;
    }

    int get_int() {
        return *reinterpret_cast<int *>(this + 0x58);
    }
};

#endif
