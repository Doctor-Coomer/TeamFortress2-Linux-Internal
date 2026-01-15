#ifndef CONVAR_HPP
#define CONVAR_HPP

class Convar {
public:
  void set_int(int value) {
    *(int*)(this + 0x58) = value;
  }

  int get_int() {
    return *(int*)(this + 0x58);
  }

  void set_float(float value) {
    *(float*)(this + 0x54) = value;
  }

  float get_float(float value) {
    return *(float*)(this + 0x54);
  }

};

#endif
