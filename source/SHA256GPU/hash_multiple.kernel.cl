//SHA256 Multiple Hash Kernel
//Nagy Richard Antal
//2020


//SHA-256 context
#define H0 0x6a09e667
#define H1 0xbb67ae85
#define H2 0x3c6ef372
#define H3 0xa54ff53a
#define H4 0x510e527f
#define H5 0x9b05688c
#define H6 0x1f83d9ab
#define H7 0x5be0cd19


//Methods
__local inline uint rotr(uint x, int n)
{
    if (n < 32) return (x >> n) | (x << (32 - n));
    return x;
}
inline uint ch(uint x, uint y, uint z)
{
    return (x & y) ^ (~x & z);
}
inline uint maj(uint x, uint y, uint z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}
inline uint sig0(uint x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
inline uint sig1(uint x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}
inline uint ep0(uint x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}
inline uint ep1(uint x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}


//Kernel
kernel void sha256multiple_kernel(uint key_length, __global char* keys, __global char* results)
{
    //Initialize
    int t;
    int gid;
    int msg_pad = 0;
    int cur_pad;
    int stop;
    int mmod;
    uint i;
    uint length;
    uint item;
    uint total;
    uint W[80];
    uint temp;
    uint A, B, C, D, E, F, G, H;
    uint T1, T2;
    uint uiresult[8];
    uint K[64] =
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
    const char hex_charset[] = "0123456789abcdef";


    //Load properties
    uint globalID = get_global_id(0);
    __global char* key = keys + globalID * key_length;
    __global char* result = results + globalID * 65;

    for (length = 0; length < key_length && key[length] != 0; length++) { }
    total = length % 64 >= 56 ? 2 : 1 + length / 64;


    //Reset algorithm
    uiresult[0] = H0;
    uiresult[1] = H1;
    uiresult[2] = H2;
    uiresult[3] = H3;
    uiresult[4] = H4;
    uiresult[5] = H5;
    uiresult[6] = H6;
    uiresult[7] = H7;


    //Hash
    for (item = 0; item < total; item++)
    {
        A = uiresult[0];
        B = uiresult[1];
        C = uiresult[2];
        D = uiresult[3];
        E = uiresult[4];
        F = uiresult[5];
        G = uiresult[6];
        H = uiresult[7];

        #pragma unroll
        for (t = 0; t < 80; t++)
        {
            W[t] = 0x00000000;
        }

        msg_pad = item * 64;
        if (length > msg_pad)
        {
            cur_pad = (length - msg_pad) > 64 ? 64 : (length - msg_pad);
        }
        else
        {
            cur_pad = -1;
        }

        if (cur_pad > 0)
        {
            i = cur_pad;
            stop = i / 4;
            for (t = 0; t < stop; t++)
            {
                W[t] = ((uchar)key[msg_pad + t * 4]) << 24;
                W[t] |= ((uchar)key[msg_pad + t * 4 + 1]) << 16;
                W[t] |= ((uchar)key[msg_pad + t * 4 + 2]) << 8;
                W[t] |= ((uchar)key[msg_pad + t * 4 + 3]);
            }

            mmod = i % 4;
            if (mmod == 3)
            {
                W[t] = ((uchar)key[msg_pad + t * 4]) << 24;
                W[t] |= ((uchar)key[msg_pad + t * 4 + 1]) << 16;
                W[t] |= ((uchar)key[msg_pad + t * 4 + 2]) << 8;
                W[t] |= ((uchar)0x80);
            }
            else if (mmod == 2)
            {
                W[t] = ((uchar)key[msg_pad + t * 4]) << 24;
                W[t] |= ((uchar)key[msg_pad + t * 4 + 1]) << 16;
                W[t] |= 0x8000;
            }
            else if (mmod == 1)
            {
                W[t] = ((uchar)key[msg_pad + t * 4]) << 24;
                W[t] |= 0x800000;
            }
            else
            {
                W[t] = 0x80000000;
            }

            if (cur_pad < 56)
            {
                W[15] = length * 8;
            }
        }
        else if (cur_pad < 0)
        {
            if (length % 64 == 0)
            {
                W[0] = 0x80000000;
            }
            W[15] = length * 8;
        }

        for (t = 0; t < 64; t++)
        {
            if (t >= 16)
            {
                W[t] = ep1(W[t - 2]) + W[t - 7] + ep0(W[t - 15]) + W[t - 16];
            }

            T1 = H + sig1(E) + ch(E, F, G) + K[t] + W[t];
            T2 = sig0(A) + maj(A, B, C);

            H = G;
            G = F;
            F = E;
            E = D + T1;
            D = C;
            C = B;
            B = A;
            A = T1 + T2;
        }

        uiresult[0] += A;
        uiresult[1] += B;
        uiresult[2] += C;
        uiresult[3] += D;
        uiresult[4] += E;
        uiresult[5] += F;
        uiresult[6] += G;
        uiresult[7] += H;

        //Convert uints to hex char array
        #pragma unroll
        for (int j = 0; j < 8; j++)
        {
            uint n = uiresult[j];
            #pragma unroll
            for (int len = 8-1; len >= 0; n >>= 4, --len)
            {
                result[(j*8) + len] = hex_charset[n & 0xf];
            }
        }
        result[64] = '\n';

        //printf("%s\n", result);
    }
}