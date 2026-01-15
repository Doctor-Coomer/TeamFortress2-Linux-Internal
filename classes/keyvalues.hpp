#ifndef KEYVALUES_HPP
#define KEYVALUES_HPP

#include <string.h>

enum types_t {
  TYPE_NONE = 0,
  TYPE_STRING,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_PTR,
  TYPE_WSTRING,
  TYPE_COLOR,
  TYPE_UINT64,
  TYPE_NUMTYPES,
};

class KeyValues;
static KeyValues* (*key_values_constructor_original)(void*, const char*);
static void (*key_values_set_int_original)(void*, const char*, int);
static bool (*key_values_load_from_buffer_original)(void*, const char*, const char*, void*, const char*);

void key_values_set_int_hook(void* me, const char* key, int value) {

  /*
  if (strstr(key, "class")) {
    value = 3;
    }*/
  
  key_values_set_int_original(me, key, value);
}

class KeyValues {
public:
  KeyValues(const char* name) {
    key_values_constructor_original(this, name);
  }

  KeyValues() {
    
  }

  void set_int(const char* key_name, int value) {
    key_values_set_int_original(this, key_name, value);
  }

  bool load_from_buffer(const char* resource_name, const char* buffer) {
    return key_values_load_from_buffer_original(this, resource_name, buffer, nullptr, nullptr);
  }
  
  
private:
  int m_iKeyName;
  char* m_sValue;
  wchar_t* m_wsValue;

  union {
    int m_iValue;
    float m_flValue;
    void* m_pValue;
    unsigned char m_Color[4];
  };

  char m_iDataType;
  char m_bHasEscapeSequences;
  char m_bEvaluateConditionals;
  char unused[1];

  KeyValues* m_pPeer;
  KeyValues* m_pSub;
  KeyValues* m_pChain;
};

#endif
