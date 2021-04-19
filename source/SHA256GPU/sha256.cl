//Debug default
#ifdef USE_DEBUG_DEFAULTS

    #define KEY_LENGTH 17
    #define SALT_LENGTH 6
    #define SALT_STRING Hf45DD

    //Key: 'enclosesHf45DD' (8:6)
    #define HASH_0 2627633883
    #define HASH_1 342531070
    #define HASH_2 1941191403
    #define HASH_3 4203335048
    #define HASH_4 2588921030
    #define HASH_5 4270942822
    #define HASH_6 3704920044
    #define HASH_7 3301016226

#endif


//SHA-256 context
#define H0 0x6a09e667
#define H1 0xbb67ae85
#define H2 0x3c6ef372
#define H3 0xa54ff53a
#define H4 0x510e527f
#define H5 0x9b05688c
#define H6 0x1f83d9ab
#define H7 0x5be0cd19

//String convert macro
#define STR(s) #s
#define XSTR(s) STR(s)

//Methods
// << : bitshift left
// >> : bitshift right
// ^  : bitwise XOR
// ~  : bitwise NOT
// &  : bitwise AND
// |  : bitwise OR
inline uint rotr(uint x, int n) //Rotate right
{
    return (x >> n) | (x << (32 - n));
}
inline uint ch(uint x, uint y, uint z) //Choice based on x
{
    return (x & y) ^ (~x & z);
}
inline uint maj(uint x, uint y, uint z) //Majority bits
{
    return (x & y) ^ (x & z) ^ (y & z);
}
inline uint sig0(uint x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}
inline uint sig1(uint x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}
inline uint csig0(uint x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
inline uint csig1(uint x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}