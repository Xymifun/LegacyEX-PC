#include "platform/pc/PcEd25519.hpp"

#include <stdint.h>
#include <string.h>

#include <vector>

namespace
{

const uint64_t kSha512K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL,
    0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
    0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL, 0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
    0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL, 0x983e5152ee66dfabULL,
    0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
    0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL,
    0x53380d139d95b3dfULL, 0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
    0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL, 0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
    0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL,
    0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL, 0xca273eceea26619cULL,
    0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
    0x113f9804bef90daeULL, 0x1b710b35131c471bULL, 0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL,
    0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL};

uint64_t Rotr(uint64_t value, int bits)
{
    return (value >> bits) | (value << (64 - bits));
}

void Sha512Block(uint64_t state[8], const unsigned char* block)
{
    uint64_t w[80];
    for (int i = 0; i < 16; ++i)
    {
        w[i] = 0;
        for (int b = 0; b < 8; ++b)
            w[i] = (w[i] << 8) | block[i * 8 + b];
    }
    for (int i = 16; i < 80; ++i)
    {
        const uint64_t s0 = Rotr(w[i - 15], 1) ^ Rotr(w[i - 15], 8) ^ (w[i - 15] >> 7);
        const uint64_t s1 = Rotr(w[i - 2], 19) ^ Rotr(w[i - 2], 61) ^ (w[i - 2] >> 6);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint64_t e = state[4], f = state[5], g = state[6], h = state[7];
    for (int i = 0; i < 80; ++i)
    {
        const uint64_t t1 = h + (Rotr(e, 14) ^ Rotr(e, 18) ^ Rotr(e, 41)) + ((e & f) ^ (~e & g)) + kSha512K[i] + w[i];
        const uint64_t t2 = (Rotr(a, 28) ^ Rotr(a, 34) ^ Rotr(a, 39)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

typedef int64_t gf[16];

const gf kZero = {0};
const gf kOne = {1};
const gf kD = {0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070,
               0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203};
const gf kI = {0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43,
               0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83};
const gf kD2 = {0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0,
                0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406};
const gf kBaseX = {0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c,
                   0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169};
const gf kBaseY = {0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666,
                   0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666};
const int64_t kL[32] = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7,
                        0xa2, 0xde, 0xf9, 0xde, 0x14, 0,    0,    0,    0,    0,    0,
                        0,    0,    0,    0,    0,    0,    0,    0,    0,    0x10};

void Set(gf r, const gf a)
{
    for (int i = 0; i < 16; ++i)
        r[i] = a[i];
}

void Carry(gf o)
{
    for (int i = 0; i < 16; ++i)
    {
        o[i] += 65536;
        const int64_t c = o[i] >> 16;
        if (i < 15)
            o[i + 1] += c - 1;
        else
            o[0] += 38 * (c - 1);
        o[i] -= c * 65536;
    }
}

void Select(gf p, gf q, int b)
{
    const int64_t mask = ~(static_cast<int64_t>(b) - 1);
    for (int i = 0; i < 16; ++i)
    {
        const int64_t t = mask & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

void Pack25519(unsigned char* out, const gf n)
{
    gf m, t;
    Set(t, n);
    Carry(t);
    Carry(t);
    Carry(t);
    for (int j = 0; j < 2; ++j)
    {
        m[0] = t[0] - 0xffed;
        for (int i = 1; i < 15; ++i)
        {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        const int b = static_cast<int>((m[15] >> 16) & 1);
        m[14] &= 0xffff;
        Select(t, m, 1 - b);
    }
    for (int i = 0; i < 16; ++i)
    {
        out[2 * i] = static_cast<unsigned char>(t[i] & 0xff);
        out[2 * i + 1] = static_cast<unsigned char>(t[i] >> 8);
    }
}

int Parity(const gf a)
{
    unsigned char d[32];
    Pack25519(d, a);
    return d[0] & 1;
}

void Add(gf o, const gf a, const gf b)
{
    for (int i = 0; i < 16; ++i)
        o[i] = a[i] + b[i];
}

void Sub(gf o, const gf a, const gf b)
{
    for (int i = 0; i < 16; ++i)
        o[i] = a[i] - b[i];
}

void Mul(gf o, const gf a, const gf b)
{
    int64_t t[31];
    for (int i = 0; i < 31; ++i)
        t[i] = 0;
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
            t[i + j] += a[i] * b[j];
    for (int i = 0; i < 15; ++i)
        t[i] += 38 * t[i + 16];
    for (int i = 0; i < 16; ++i)
        o[i] = t[i];
    Carry(o);
    Carry(o);
}

void Square(gf o, const gf a)
{
    Mul(o, a, a);
}

void Invert(gf o, const gf in)
{
    gf c;
    Set(c, in);
    for (int a = 253; a >= 0; --a)
    {
        Square(c, c);
        if (a != 2 && a != 4)
            Mul(c, c, in);
    }
    Set(o, c);
}

void Unpack25519(gf o, const unsigned char* n)
{
    for (int i = 0; i < 16; ++i)
        o[i] = n[2 * i] + (static_cast<int64_t>(n[2 * i + 1]) << 8);
    o[15] &= 0x7fff;
}

bool Equal25519(const gf a, const gf b)
{
    unsigned char c[32], d[32];
    Pack25519(c, a);
    Pack25519(d, b);
    return memcmp(c, d, 32) == 0;
}

void Pow2523(gf o, const gf in)
{
    gf c;
    Set(c, in);
    for (int a = 250; a >= 0; --a)
    {
        Square(c, c);
        if (a != 1)
            Mul(c, c, in);
    }
    Set(o, c);
}

void PointAdd(gf p[4], gf q[4])
{
    gf a, b, c, d, t, e, f, g, h;
    Sub(a, p[1], p[0]);
    Sub(t, q[1], q[0]);
    Mul(a, a, t);
    Add(b, p[0], p[1]);
    Add(t, q[0], q[1]);
    Mul(b, b, t);
    Mul(c, p[3], q[3]);
    Mul(c, c, kD2);
    Mul(d, p[2], q[2]);
    Add(d, d, d);
    Sub(e, b, a);
    Sub(f, d, c);
    Add(g, d, c);
    Add(h, b, a);
    Mul(p[0], e, f);
    Mul(p[1], h, g);
    Mul(p[2], g, f);
    Mul(p[3], e, h);
}

void PointSwap(gf p[4], gf q[4], int b)
{
    for (int i = 0; i < 4; ++i)
        Select(p[i], q[i], b);
}

void PointPack(unsigned char r[32], gf p[4])
{
    gf tx, ty, zi;
    Invert(zi, p[2]);
    Mul(tx, p[0], zi);
    Mul(ty, p[1], zi);
    Pack25519(r, ty);
    r[31] ^= static_cast<unsigned char>(Parity(tx) << 7);
}

void ScalarMult(gf p[4], gf q[4], const unsigned char* s)
{
    Set(p[0], kZero);
    Set(p[1], kOne);
    Set(p[2], kOne);
    Set(p[3], kZero);
    for (int i = 255; i >= 0; --i)
    {
        const int b = (s[i / 8] >> (i & 7)) & 1;
        PointSwap(p, q, b);
        PointAdd(q, p);
        PointAdd(p, p);
        PointSwap(p, q, b);
    }
}

void ScalarBase(gf p[4], const unsigned char* s)
{
    gf q[4];
    Set(q[0], kBaseX);
    Set(q[1], kBaseY);
    Set(q[2], kOne);
    Mul(q[3], kBaseX, kBaseY);
    ScalarMult(p, q, s);
}

bool UnpackNegative(gf r[4], const unsigned char p[32])
{
    gf t, chk, num, den, den2, den4, den6;
    Set(r[2], kOne);
    Unpack25519(r[1], p);
    Square(num, r[1]);
    Mul(den, num, kD);
    Sub(num, num, r[2]);
    Add(den, r[2], den);
    Square(den2, den);
    Square(den4, den2);
    Mul(den6, den4, den2);
    Mul(t, den6, num);
    Mul(t, t, den);
    Pow2523(t, t);
    Mul(t, t, num);
    Mul(t, t, den);
    Mul(t, t, den);
    Mul(r[0], t, den);
    Square(chk, r[0]);
    Mul(chk, chk, den);
    if (!Equal25519(chk, num))
        Mul(r[0], r[0], kI);
    Square(chk, r[0]);
    Mul(chk, chk, den);
    if (!Equal25519(chk, num))
        return false;
    if (Parity(r[0]) == (p[31] >> 7))
        Sub(r[0], kZero, r[0]);
    Mul(r[3], r[0], r[1]);
    return true;
}

void ModL(unsigned char* r, int64_t x[64])
{
    for (int i = 63; i >= 32; --i)
    {
        int64_t carry = 0;
        int j;
        for (j = i - 32; j < i - 12; ++j)
        {
            x[j] += carry - 16 * x[i] * kL[j - (i - 32)];
            carry = (x[j] + 128) >> 8;
            x[j] -= carry * 256;
        }
        x[j] += carry;
        x[i] = 0;
    }
    int64_t carry = 0;
    for (int j = 0; j < 32; ++j)
    {
        x[j] += carry - (x[31] >> 4) * kL[j];
        carry = x[j] >> 8;
        x[j] &= 255;
    }
    for (int j = 0; j < 32; ++j)
        x[j] -= carry * kL[j];
    for (int i = 0; i < 32; ++i)
    {
        x[i + 1] += x[i] >> 8;
        r[i] = static_cast<unsigned char>(x[i] & 255);
    }
}

void Reduce(unsigned char r[64])
{
    int64_t x[64];
    for (int i = 0; i < 64; ++i)
        x[i] = r[i];
    for (int i = 0; i < 64; ++i)
        r[i] = 0;
    ModL(r, x);
}

void ExpandSeed(const unsigned char seed[32], unsigned char expanded[64])
{
    PcSha512(seed, 32, expanded);
    expanded[0] &= 248;
    expanded[31] &= 127;
    expanded[31] |= 64;
}
}

void PcSha512Begin(PcSha512Context* context)
{
    static const uint64_t kInitial[8] = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL,
                                         0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
                                         0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
    memcpy(context->state, kInitial, sizeof(kInitial));
    context->used = 0;
    context->total = 0;
}

void PcSha512Add(PcSha512Context* context, const unsigned char* data, size_t size)
{
    context->total += size;
    if (context->used > 0)
    {
        const size_t take = size < 128 - context->used ? size : 128 - context->used;
        memcpy(context->buffer + context->used, data, take);
        context->used += take;
        data += take;
        size -= take;
        if (context->used < 128)
            return;
        Sha512Block(context->state, context->buffer);
        context->used = 0;
    }
    for (; size >= 128; data += 128, size -= 128)
        Sha512Block(context->state, data);
    memcpy(context->buffer, data, size);
    context->used = size;
}

void PcSha512End(PcSha512Context* context, unsigned char digest[64])
{
    unsigned char tail[256];
    memset(tail, 0, sizeof(tail));
    const size_t rest = context->used;
    memcpy(tail, context->buffer, rest);
    tail[rest] = 0x80;
    const size_t blocks = rest < 112 ? 1 : 2;
    const uint64_t bits = context->total * 8;
    for (int b = 0; b < 8; ++b)
        tail[blocks * 128 - 1 - b] = static_cast<unsigned char>(bits >> (8 * b));
    for (size_t block = 0; block < blocks; ++block)
        Sha512Block(context->state, tail + block * 128);
    for (int i = 0; i < 8; ++i)
        for (int b = 0; b < 8; ++b)
            digest[i * 8 + b] = static_cast<unsigned char>(context->state[i] >> (56 - 8 * b));
}

void PcSha512(const unsigned char* data, size_t size, unsigned char digest[64])
{
    PcSha512Context context;
    PcSha512Begin(&context);
    PcSha512Add(&context, data, size);
    PcSha512End(&context, digest);
}

void PcEd25519PublicKey(const unsigned char seed[32], unsigned char publicKey[32])
{
    unsigned char expanded[64];
    ExpandSeed(seed, expanded);
    gf p[4];
    ScalarBase(p, expanded);
    PointPack(publicKey, p);
}

bool PcEd25519Verify(const unsigned char publicKey[32], const unsigned char* message, size_t size,
                     const unsigned char signature[64])
{
    for (int i = 31; i >= 0; --i)
    {
        if (signature[32 + i] < kL[i])
            break;
        if (signature[32 + i] > kL[i] || i == 0)
            return false;
    }
    gf q[4];
    if (!UnpackNegative(q, publicKey))
        return false;
    if (size > static_cast<size_t>(-1) - 64)
        return false;
    std::vector<unsigned char> buffer(64 + size);
    memcpy(&buffer[0], signature, 32);
    memcpy(&buffer[32], publicKey, 32);
    if (size != 0)
        memcpy(&buffer[64], message, size);
    unsigned char h[64];
    PcSha512(&buffer[0], buffer.size(), h);
    Reduce(h);
    gf p[4];
    ScalarMult(p, q, h);
    ScalarBase(q, signature + 32);
    PointAdd(p, q);
    unsigned char check[32];
    PointPack(check, p);
    unsigned char diff = 0;
    for (int i = 0; i < 32; ++i)
        diff |= static_cast<unsigned char>(check[i] ^ signature[i]);
    return diff == 0;
}

void PcEd25519Sign(const unsigned char seed[32], const unsigned char* message, size_t size,
                   unsigned char signature[64])
{
    unsigned char expanded[64];
    ExpandSeed(seed, expanded);
    unsigned char publicKey[32];
    {
        gf p[4];
        ScalarBase(p, expanded);
        PointPack(publicKey, p);
    }

    if (size > static_cast<size_t>(-1) - 64)
        return;
    std::vector<unsigned char> storage(64 + size);
    unsigned char* buffer = storage.data();
    memcpy(buffer + 32, expanded + 32, 32);
    if (size != 0)
        memcpy(buffer + 64, message, size);
    unsigned char r[64];
    PcSha512(buffer + 32, 32 + size, r);
    Reduce(r);
    gf p[4];
    ScalarBase(p, r);
    PointPack(signature, p);

    memcpy(buffer, signature, 32);
    memcpy(buffer + 32, publicKey, 32);
    unsigned char h[64];
    PcSha512(buffer, 64 + size, h);
    Reduce(h);
    int64_t x[64];
    for (int i = 0; i < 64; ++i)
        x[i] = 0;
    for (int i = 0; i < 32; ++i)
        x[i] = r[i];
    for (int i = 0; i < 32; ++i)
        for (int j = 0; j < 32; ++j)
            x[i + j] += h[i] * static_cast<int64_t>(expanded[j]);
    ModL(signature + 32, x);
}
