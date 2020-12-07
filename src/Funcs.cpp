#include "Funcs.h"


const uint32_t K[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const uint32_t H[] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19
};

boost::shared_ptr<vargroup> helper_full_sha256(uint32_t* inp, uint32_t nonce, boost::shared_ptr<vargroup>& knownMessageIntermediaryH){
    return helper_full_sha256(inp, inp+16, nonce, knownMessageIntermediaryH);
}
boost::shared_ptr<vargroup> helper_full_sha256(uint32_t* inp1, uint32_t* inp2, uint32_t nonce, boost::shared_ptr<vargroup>& knownMessageIntermediaryH){
    if(!knownMessageIntermediaryH){
        knownMessageIntermediaryH = sha256_createMessage(inp1);
    }
    boost::shared_ptr<uint32_t[]> tempW = sha256_expander1(inp2,nonce);
    boost::shared_ptr<vargroup> intermediaryH = sha256_compressor1(tempW.get(), (uint32_t*) knownMessageIntermediaryH.get());
    return sha256_round2((uint32_t*) intermediaryH.get());
}

boost::shared_ptr<vargroup> sha256_createMessage(uint32_t* M){
    uint32_t W[64];
    for(uint32_t t = 0; t < 16; t++){
        W[t] = M[t];
    }

    for(uint32_t t = 16; t < 64; t++){
        W[t] = Sigma1(W[t-2])+W[t-7]+Sigma0(W[t-15])+W[t-16];
    }

    boost::shared_ptr<vargroup> varsPtr(new vargroup{H[0],H[1],H[2],H[3],H[4],H[5],H[6],H[7]});
    vargroup& vars = *(varsPtr.get());

    for(uint32_t t = 0; t < 64; t++){
        uint32_t t1 = T1(t,W,vars.e,vars.f,vars.g,vars.h);
        uint32_t t2 = T2(vars.a,vars.b,vars.c);
        vars.h = vars.g;
        vars.g = vars.f;
        vars.f = vars.e;
        vars.e = vars.d+t1;
        vars.d = vars.c;
        vars.c = vars.b;
        vars.b = vars.a;
        vars.a = t1+t2;
    }

    vars.a += H[0];
    vars.b += H[1];
    vars.c += H[2];
    vars.d += H[3];
    vars.e += H[4];
    vars.f += H[5];
    vars.g += H[6];
    vars.h += H[7];

    return varsPtr;
}

boost::shared_ptr<uint32_t[]> sha256_expander1(uint32_t* M, uint32_t nonce){
    boost::shared_ptr<uint32_t[]> W(new uint32_t[64]);
    for(uint32_t t = 0; t < 3; t++){
        W[t] = M[t];
    }
    W[3] = nonce;
    W[4] = 0x80000000;
    for(uint32_t t = 5; t < 15; t++){
        W[t] = 0x00;
    }
    W[15] = 0x280;

    for(uint32_t t = 16; t < 64; t++){
        W[t] = Sigma1(W[t-2])+W[t-7]+Sigma0(W[t-15])+W[t-16];
    }

    return W;
}

boost::shared_ptr<vargroup> sha256_compressor1(uint32_t* W, const uint32_t* const intermediaryH){
    boost::shared_ptr<vargroup> varsPtr(new vargroup{
                                        intermediaryH[0],intermediaryH[1],intermediaryH[2],intermediaryH[3],
                                        intermediaryH[4],intermediaryH[5],intermediaryH[6],intermediaryH[7]
                                        });
    vargroup& vars = *(varsPtr.get());
    for(uint32_t t = 0; t < 64; t++){
        uint32_t t1 = T1(t,W,vars.e,vars.f,vars.g,vars.h);
        uint32_t t2 = T2(vars.a,vars.b,vars.c);
        vars.h = vars.g;
        vars.g = vars.f;
        vars.f = vars.e;
        vars.e = vars.d+t1;
        vars.d = vars.c;
        vars.c = vars.b;
        vars.b = vars.a;
        vars.a = t1+t2;
    }
    vars.a += intermediaryH[0];
    vars.b += intermediaryH[1];
    vars.c += intermediaryH[2];
    vars.d += intermediaryH[3];
    vars.e += intermediaryH[4];
    vars.f += intermediaryH[5];
    vars.g += intermediaryH[6];
    vars.h += intermediaryH[7];

    return varsPtr;
}

boost::shared_ptr<vargroup> sha256_round2(uint32_t* prevH){
    uint32_t M[16] = {
        prevH[0],   prevH[1],   prevH[2],   prevH[3],
        prevH[4],   prevH[5],   prevH[6],   prevH[7],
        0x80000000, 0x00,       0x00,       0x00,
        0x00,       0x00,       0x00,       0x100,
    };

    return sha256_createMessage(M);
}
