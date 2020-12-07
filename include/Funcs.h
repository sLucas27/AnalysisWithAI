#ifndef FUNCS_H
#define FUNCS_H

#include <x86intrin.h>
#include <stdint.h>

#include <boost/shared_ptr.hpp>

extern const uint32_t K[64];
extern const uint32_t H[8];

struct block{
    uint32_t M1[32];
    uint32_t M2[3];
};

struct vargroup{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f;
    uint32_t g;
    uint32_t h;
};

inline uint32_t ROTL(uint32_t x, uint32_t n){
    #ifdef __GNUC__
    return __rold(x,n);
    #else
    return (x<<n) | (x>>(32-n));
    #endif // __GCC__
}

inline uint32_t ROTR(uint32_t x, uint32_t n){
    #ifdef __GNUC__
    return __rord(x,n);
    #else
    return (x>>n) | (x<<(32-n));
    #endif // __GCC__
}

inline uint32_t SHR(uint32_t x, uint32_t n){
    return x >> n;
}

inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) ^ (~x & z);
}

inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z){
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t Sum0(uint32_t x){
    return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22);
}

inline uint32_t Sum1(uint32_t x){
    return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25);
}

inline uint32_t Sigma0(uint32_t x){
    return ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3);
}

inline uint32_t Sigma1(uint32_t x){
    return ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10);
}

inline uint32_t T1(uint32_t t, uint32_t* W, uint32_t e, uint32_t f, uint32_t g, uint32_t h){
    return h + Sum1(e) + Ch(e,f,g) + K[t] + W[t];
}

inline uint32_t T2(uint32_t a, uint32_t b, uint32_t c){
    return Sum0(a) + Maj(a,b,c);
}

boost::shared_ptr<vargroup> helper_full_sha256(uint32_t* inp, uint32_t nonce, boost::shared_ptr<vargroup>& knownMessageIntermediaryH);
boost::shared_ptr<vargroup> helper_full_sha256(uint32_t* inp1, uint32_t* inp2, uint32_t nonce, boost::shared_ptr<vargroup>& knownMessageIntermediaryH);

boost::shared_ptr<vargroup> sha256_createMessage(uint32_t* M);
boost::shared_ptr<uint32_t[]> sha256_expander1(uint32_t* M, uint32_t nonce);
boost::shared_ptr<vargroup> sha256_compressor1(uint32_t* W, const uint32_t* const iH);
boost::shared_ptr<vargroup> sha256_round2(uint32_t* prevH);
#endif // FUNCS_H
