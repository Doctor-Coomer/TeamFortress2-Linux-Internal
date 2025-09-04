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

static KeyValues * (*key_values_constructor_original)(void *, const char *);

class KeyValues {
    KeyValues(const char *name) {
        key_values_constructor_original(this, name);
    }

private:
    int m_iKeyName;
    char *m_sValue;
    wchar_t *m_wsValue;

    union {
        int m_iValue;
        float m_flValue;
        void *m_pValue;
        unsigned char m_Color[4];
    };

    char m_iDataType;
    char m_bHasEscapeSequences;
    char m_bEvaluateConditionals;
    char unused[1];

    KeyValues *m_pPeer;
    KeyValues *m_pSub;
    KeyValues *m_pChain;
};

#endif
