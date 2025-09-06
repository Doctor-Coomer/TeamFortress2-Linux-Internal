#pragma once

#include <cstdint>

#define md5_digest_length 16
#define md5_bit_length (md5_digest_length * sizeof(unsigned char))

struct md5_value_t {
    unsigned char bits[md5_digest_length];

    void zero();

    bool is_zero() const;

    bool operator==(const md5_value_t &src) const;

    bool operator!=(const md5_value_t &src) const;
};

struct md5_context_t {
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
};

void md5_init(md5_context_t *context);

void md5_update(md5_context_t *context, const unsigned char *buf, uint32_t len);

void md5_final(unsigned char digest[md5_digest_length], md5_context_t *context);

char *md5_print(unsigned char *digest, int hash_len);

void md5_process_single_buffer(const void *p, int len, md5_value_t &md5_result);

uint32_t md5_pseudo_random(uint32_t seed);

bool md5_compare(const md5_value_t &data, const md5_value_t &compare);

inline bool md5_value_t::operator==(const md5_value_t &src) const {
    return md5_compare(*this, src);
}

inline bool md5_value_t::operator!=(const md5_value_t &src) const {
    return !md5_compare(*this, src);
}
