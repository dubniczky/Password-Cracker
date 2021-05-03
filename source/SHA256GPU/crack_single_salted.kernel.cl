//===================
//| GPU Kernel code |
//| DO NOT MODIFY!! |
//===================
//
//SHA256 Single Salted Crack Kernel
//Nagy Richard Antal, 2021

//#define USE_DEBUG_DEFAULTS

#include <sha256.cl>

//Kernel
kernel void sha256crack_single_salted_kernel(global char* keys, global uint* result)
{
    //Initialize
    int qua; //Message schedule step modulus
    int mod; //Message schedule step modulus
    uint length; //Message schedule    
    uint A, B, C, D, E, F, G, H; //Compression targets
    uint T1, T2; //Compression temp
    uint globalID; //Global worker id
    global uchar* globalKey; //Global target key location
    char key[KEY_LENGTH + SALT_LENGTH + 1]; //Cache key storage
    uint W[80];
    const uint K[64] =
    {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };


    //Get key
    globalID = get_global_id(0);
    globalKey = keys + globalID * KEY_LENGTH; //Get pointer to key string
    for (length = 0; length < KEY_LENGTH && (globalKey[length] != 0 && globalKey[length] != '\n'); length++)
    {
        key[length] = globalKey[length];
    }

    
    //Append salt
    #pragma unroll
    for (uint i = 0; i < SALT_LENGTH; i++)
    {
        key[length + i] = XSTR(SALT_STRING)[i];
    }
    length += SALT_LENGTH;
    key[length] = 0;


    //Reset algorithm
    #pragma unroll
    for (uint i = 0; i < 80; i++)
    {
        W[i] = 0x00000000;
    }


    //Create message block
    qua = length / 4;
    mod = length % 4;
    for (uint i = 0; i < qua; i++)
    {
        W[i]  = (key[i * 4 + 0]) << 24;
        W[i] |= (key[i * 4 + 1]) << 16;
        W[i] |= (key[i * 4 + 2]) << 8;
        W[i] |= (key[i * 4 + 3]);
    }

    //Pad remaining uint
    switch (mod)
    {
        //l = n * 4 + 0
        case 0:
            W[qua] = 0x80000000;
            break;

        //l = n * 4 + 3
        case 3:
            W[qua]  = (key[qua * 4 + 0]) << 24;
            W[qua] |= (key[qua * 4 + 1]) << 16;
            W[qua] |= (key[qua * 4 + 2]) << 8;
            W[qua] |= 0x80;
            break;

        //l = n * 4 + 2
        case 2:
            W[qua]  = (key[qua * 4 + 0]) << 24;
            W[qua] |= (key[qua * 4 + 1]) << 16;
            W[qua] |= 0x8000;
            break;

        //l = n * 4 + 1
        case 1:
            W[qua]  = (key[qua * 4 + 0]) << 24;
            W[qua] |= 0x800000;
            break;
    }

    W[15] = length * 8; //Add key length


    //Run message schedule
    #pragma unroll
    for (uint i = 16; i < 64; i++)
    {
        W[i] = sig1(W[i - 2]) + W[i - 7] + sig0(W[i - 15]) + W[i - 16];
    }


    //Prepare compression
    A = H0;
    B = H1;
    C = H2;
    D = H3;
    E = H4;
    F = H5;
    G = H6;
    H = H7;


    //Compress
    #pragma unroll
    for (uint i = 0; i < 64; i++)
    {
        //Compress temporary
        T1 = H + csig1(E) + ch(E, F, G) + K[i] + W[i];
        T2 = csig0(A) + maj(A, B, C);

        //Rotate over, override H
        H = G;
        G = F;
        F = E;
        E = D + T1;
        D = C;
        C = B;
        B = A;
        A = T1 + T2;
    }


    //Verify result
    if (A == HASH_0 - H0 && B == HASH_1 - H1 &&
        C == HASH_2 - H2 && D == HASH_3 - H3 &&
        E == HASH_4 - H4 && F == HASH_5 - H5 &&
        G == HASH_6 - H6 && H == HASH_7 - H7)
    {
        *result = globalID + 1;
    }
}