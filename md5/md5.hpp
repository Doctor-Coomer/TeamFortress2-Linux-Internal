#ifndef MD5_HPP
#define MD5_HPP

#include <cstdint>
#include <cstring>

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

#define f1(x, y, z) (z ^ (x & (y ^ z)))
#define f2(x, y, z) f1(z, x, y)
#define f3(x, y, z) (x ^ y ^ z)
#define f4(x, y, z) (y ^ (x | ~z))

#define md5_step(f, w, x, y, z, data, s) ( w += f(x, y, z) + data,  w = (w << s) | (w >> (32 - s)),  w += x )

static void md5_transform(uint32_t buf[4], const uint32_t in[16]) {
    uint32_t a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    md5_step(f1, a, b, c, d, in[0] + 0xd76aa478, 7);
    md5_step(f1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    md5_step(f1, c, d, a, b, in[2] + 0x242070db, 17);
    md5_step(f1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    md5_step(f1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    md5_step(f1, d, a, b, c, in[5] + 0x4787c62a, 12);
    md5_step(f1, c, d, a, b, in[6] + 0xa8304613, 17);
    md5_step(f1, b, c, d, a, in[7] + 0xfd469501, 22);
    md5_step(f1, a, b, c, d, in[8] + 0x698098d8, 7);
    md5_step(f1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    md5_step(f1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    md5_step(f1, b, c, d, a, in[11] + 0x895cd7be, 22);
    md5_step(f1, a, b, c, d, in[12] + 0x6b901122, 7);
    md5_step(f1, d, a, b, c, in[13] + 0xfd987193, 12);
    md5_step(f1, c, d, a, b, in[14] + 0xa679438e, 17);
    md5_step(f1, b, c, d, a, in[15] + 0x49b40821, 22);

    md5_step(f2, a, b, c, d, in[1] + 0xf61e2562, 5);
    md5_step(f2, d, a, b, c, in[6] + 0xc040b340, 9);
    md5_step(f2, c, d, a, b, in[11] + 0x265e5a51, 14);
    md5_step(f2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    md5_step(f2, a, b, c, d, in[5] + 0xd62f105d, 5);
    md5_step(f2, d, a, b, c, in[10] + 0x02441453, 9);
    md5_step(f2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    md5_step(f2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    md5_step(f2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    md5_step(f2, d, a, b, c, in[14] + 0xc33707d6, 9);
    md5_step(f2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    md5_step(f2, b, c, d, a, in[8] + 0x455a14ed, 20);
    md5_step(f2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    md5_step(f2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    md5_step(f2, c, d, a, b, in[7] + 0x676f02d9, 14);
    md5_step(f2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    md5_step(f3, a, b, c, d, in[5] + 0xfffa3942, 4);
    md5_step(f3, d, a, b, c, in[8] + 0x8771f681, 11);
    md5_step(f3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    md5_step(f3, b, c, d, a, in[14] + 0xfde5380c, 23);
    md5_step(f3, a, b, c, d, in[1] + 0xa4beea44, 4);
    md5_step(f3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    md5_step(f3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    md5_step(f3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    md5_step(f3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    md5_step(f3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    md5_step(f3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    md5_step(f3, b, c, d, a, in[6] + 0x04881d05, 23);
    md5_step(f3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    md5_step(f3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    md5_step(f3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    md5_step(f3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    md5_step(f4, a, b, c, d, in[0] + 0xf4292244, 6);
    md5_step(f4, d, a, b, c, in[7] + 0x432aff97, 10);
    md5_step(f4, c, d, a, b, in[14] + 0xab9423a7, 15);
    md5_step(f4, b, c, d, a, in[5] + 0xfc93a039, 21);
    md5_step(f4, a, b, c, d, in[12] + 0x655b59c3, 6);
    md5_step(f4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    md5_step(f4, c, d, a, b, in[10] + 0xffeff47d, 15);
    md5_step(f4, b, c, d, a, in[1] + 0x85845dd1, 21);
    md5_step(f4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    md5_step(f4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    md5_step(f4, c, d, a, b, in[6] + 0xa3014314, 15);
    md5_step(f4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    md5_step(f4, a, b, c, d, in[4] + 0xf7537e82, 6);
    md5_step(f4, d, a, b, c, in[11] + 0xbd3af235, 10);
    md5_step(f4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    md5_step(f4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

static void md5_init(md5_context_t *context) {
    context->buf[0] = 0x67452301;
    context->buf[1] = 0xefcdab89;
    context->buf[2] = 0x98badcfe;
    context->buf[3] = 0x10325476;

    context->bits[0] = 0;
    context->bits[1] = 0;
}

static void md5_update(md5_context_t *context, const unsigned char *buf, uint32_t len) {
    uint32_t t = context->bits[0];

    if ((context->bits[0] = t + (len << 3)) < t)
        context->bits[1]++;

    context->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;

    if (t) {
        unsigned char *p = context->in + t;

        t = 64 - t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        md5_transform(context->buf, reinterpret_cast<uint32_t *>(context->in));
        buf += t;
        len -= t;
    }

    while (len >= 64) {
        memcpy(context->in, buf, 64);
        md5_transform(context->buf, reinterpret_cast<uint32_t *>(context->in));
        buf += 64;
        len -= 64;
    }

    memcpy(context->in, buf, len);
}

static void md5_final(unsigned char digest[md5_digest_length], md5_context_t *context) {
    unsigned count = (context->bits[0] >> 3) & 0x3F;
    unsigned char *p = context->in + count;
    *p++ = 0x80;

    count = 64 - 1 - count;

    if (count < 8) {
        memset(p, 0, count);
        md5_transform(context->buf, reinterpret_cast<uint32_t *>(context->in));
        memset(context->in, 0, 56);
    } else {
        memset(p, 0, count - 8);
    }

    reinterpret_cast<uint32_t *>(context->in)[14] = context->bits[0];
    reinterpret_cast<uint32_t *>(context->in)[15] = context->bits[1];

    md5_transform(context->buf, reinterpret_cast<uint32_t *>(context->in));
    memcpy(digest, context->buf, md5_digest_length);
    memset(context, 0, sizeof(*context));
}

static uint32_t md5_pseudo_random(uint32_t seed) {
    md5_context_t ctx;
    unsigned char digest[md5_digest_length];

    memset(&ctx, 0, sizeof(ctx));

    md5_init(&ctx);
    md5_update(&ctx, reinterpret_cast<unsigned char *>(&seed), sizeof(seed));
    md5_final(digest, &ctx);

    return *reinterpret_cast<uint32_t *>(digest + 6);
}

static bool md5_compare(const md5_value_t &data, const md5_value_t &compare) {
    return memcmp(data.bits, compare.bits, md5_digest_length) == 0;
}

inline void md5_value_t::zero() {
    memset(bits, 0, sizeof(bits));
}

inline bool md5_value_t::operator==(const md5_value_t &src) const {
    return md5_compare(*this, src);
}

inline bool md5_value_t::operator!=(const md5_value_t &src) const {
    return !md5_compare(*this, src);
}

#endif
