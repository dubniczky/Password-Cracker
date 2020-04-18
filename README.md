# Password Hash Cracking on GPU
*by: Richard Nagy, 2020*

## Quick Description

We use hashes to protect user password from being exposed in the event of a breach. Servers use algorithms that create so-called hashes, which are impossible to reverse but even so, you can start trying to guess the password normally it takes so long that is not technically feasible with our current technology.

Dictionary attacks however, are quite common. We take the most used passwords that the user might have, and try every single one with a given hash. This increases our chances significantly and happens to be a great job for a GPU.

In this project I will recreate such a hashing solution from scratch with the SHA-256 algorithm. First I will make a demo version on the GPU in C to serve as a benchmark and as a starting point for the transition to GPU, since C code very close to OpenCL kernel code.


## Roadmap:
1. Implementing standard SHA-256 algorithm on CPU
2. Creating a linear password cracker using a table from file
3. Implementing salt into the algorithm
4. Implementing the algorithm on GPU in OpenCL
5. Implementing hash compare on GPU
6. **Optimizing Kernel (current)**


## Resources:
* [Wikipedia: SHA-2](https://en.wikipedia.org/wiki/SHA-2) *algorithm*
* [Wikipedia: Base-64](https://en.wikipedia.org/wiki/Base64) *data storage type*
* [Wikipedia: Salt (Crypgoraphy)](https://en.wikipedia.org/wiki/Salt_(cryptography)) *salt*
* [Have I been pwned](https://haveibeenpwned.com/Passwords) *common passwords*
* [Xorbin hash](https://xorbin.com/tools/sha256-hash-calculator) *verify results*
* [ELTE Computer Graphics](http://cg.elte.hu/index.php/gpgpu/) *custom opencl c++ library*
* [NVIDIA OpenCL](https://www.nvidia.com/content/cudazone/CUDABrowser/downloads/papers/NVIDIA_OpenCL_BestPracticesGuide.pdf) *best practices guide*

## Baseline hardware:
| Component | Baseline |
|---|---|
|__MB__|ASUS Prime X470-PRO|
|__CPU__|Ryzen 7 2700X *8c/16t 4.00Ghz @ base clock*|
|__RAM__|Corsair Vengeance 2x8GB 2400Mhz DDR4 dual channel|
|__GPU__|Nvidia Geforce GTX 1070: *6.463 Teraflops*|
|__SSD__|Samsung 970 EVO 250GB|

*I will use this hardware for all the benchmarking unless stated otherwise.*

## Custom metric (hcps)

We are going to use a custom metric to compare results: **Hash Compare Per Second** (hashcomp/sec). 
This of course means the amount of hashes we can compare every given second. This does NOT include the time to start the cracking itself and gathering the data at the end. We are only interested in the hash time itself since the program only starts once, but it can keep running for hours, days or weeks.. until we run out of samples to feed it.

Being a scalar unit, we can even use prefix multipliers:
`1,000,000,000 hcps` = `1,000,000 khcps` = `1,000 mhcps` = `1 ghcps`

## Milestone 1: implementing on CPU *(completed)*
Implementing the SHA-256 encryption algorithm on the CPU using exclusively C syntax for easier port on GPU. It must be capable of receieving a standard c string as input, and generate a 256 bit hash as an output. This means an **n** byte input and a ``256 bit = 32 byte = 64 character`` long __base64__ string as output.

We have to generate an entirely new stack of variables for each hash, because the values get moved and modified every iteration. I used an object-orinted approach in the first iteration.

Hash generating calls are going to be used in the following way:
```c
const char* hash = sha256("mypassword");
```
This of course is going to change later, but it helped isolate the variables and code needed exclusively for the hasing algorithm.


## Milestone 2: Cracking using password-table *(completed)*
With the algorithm working, we can start implementing the actual cracking. We read one file, with an unknown hash

*target.txt:* ``b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e``

The file contains the SHA-256 hashed verison of the password: ``banana``. Then we read a list of passwords that are commonly used according to data leaks.

*passwords.txt:* ``12345 abc123 password passwd 123456 newpass ...``

There are 3 main sample files in the project:

|Password Count|File Name|
|---|---|
|100|[passwords-100.txt](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-100.txt)|
|100,000|[passwords-100k.txt](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-100k.txt)|
|4,000,000|[passwords-4m.txt](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-4m.txt)|

The code can be compiled and run in the following way:
```bash
g++ main.cpp -o crack.exe && .\crack.exe target.txt passwords.txt
```

Example output by running with the __100k__ list:
```
Matching...
Match found: banana
Checked lines: 436
Search time: 12666 microseconds
```

Let us try the worst case-scenario, when the correct match is not in the file. Then we can make some calculations. I am using the baseline computer for every test.
```
Matching...
Match not found
Checked lines: 100000
Search time: 2828609 microseconds
```
So hashing and comparing ``100,000`` entries took ``2,828,609 microseconds`` = ``2.828609 seconds``. As a baseline, we are going to use the custom metric: __hashcomp/sec (hcps)__.
`100,000/2.828609 = 35,353.0658...` so we are at about ``35.353 khcps``.


## Milestone 3: Implementing salt *(completed)*

The current method of cracking seems unnecessary, since we could just pre-calculate the hashes and start comparing every time, without needing to do the hashing every time.

This is why [salts](https://en.wikipedia.org/wiki/Salt_(cryptography)) are commonly used in password storing. Salts are a random series of characters, that are attached to the end of the password before being hashed. Here are two a salted hashes for __banana__ with 4 byte salts:  

``banana`` => ``bananakQ9wvI9A`` => ``50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee``  
``banana`` => ``bananaJoz1BL1T`` => ``7da2b105a959cff3b2c03c0c15fa11fa124636a21451eeead00cb7654c664f7e``  

The same password can take up multiple forms, thus making so that if a hash is cracked, we are still unable to match every single instance of the same password. This is why it is **essential** that salts remain unique in every set of passwords, for example in a login database. This way no passwords can be equated just by looking at their hashed forms.

Now however, we will need the salt to store the password as well, so we are just going to append it to the start of the hash:  
`kQ9wvI9A` `50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee`  
`kQ9wvI9A50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee`  

We of course know, that this is a 256 bit hash, so there must be 64 characters for the hash, and every single before that is the salt. Te length of the salt can be any number of characters, so we have to be flexible about that.
```
data_length = { n | n >= 64 }
hash_length = 64
salt_length = data_length - 64
```

Example run of the banana search using salts as well:
```
Target: 'kQ9wvI9A50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee'
Salt: 'kQ9wvI9A'
Hash: '50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee'
Reading password table...
Matching...
Match found: banana
Checked lines: 436
Search time: 12955 microseconds
```
We simply remove the salt from the original target, and append it to every single  password in our list. This certainly makes it slower, so let us do our baseline test again with an __8 byte salt__:
```
Target: '9Slcgkejw8nPUkI48e852ffc1b4f8f19849ad7b1072a3db4265780924154549d4b1ba9792c75c359'
Salt: '9Slcgkejw8nPUkI4'
Hash: '8e852ffc1b4f8f19849ad7b1072a3db4265780924154549d4b1ba9792c75c359'
Reading password table...
Matching...
Match not found
Checked lines: 100000
Search time: 3106065 microseconds
```
So hashing, salting and comparing `100,000` entries took `3,106,065 microseconds` = `3.106065 seconds`.
`100,000/3.106065 = 32,195.0764...` => `32.196 khcps`. This method seems about `9%` slower, but keep in mind that it will depend on the size of the salt. That however is almost never bigger than 16 bytes.

|Method|Speed|Relative|
|---|---|---|
|Hash Compare|35.353 khcps|100%|
|Salted Compare|32.196 khcps|91%|


## Milestone 4: Implementing SHA-256 on GPU *(completed)*
In this step, the most difficult part is of course writing the kernel itself. It has to be able to calculate a single hash given a string and its length. I decided against doing string operations on the GPU too much, so the result is going to be an **unsigned int** array. The result length is fixed, so there will be no problems with that.

Kernel definition: 
```opencl
//hash_single.kernel.cl
kernel void sha256single_kernel(uint key_length,
                                __global char* key,
                                __global uint* result)
```
We can then feed the information using global memory buffers. Previously we defined some macros to speed up the code, which is not going to be necessary in this case, since the compiler merges every **inline** method into the kernel.

Example:
```opencl
inline uint rotr(uint x, int n)
{
    if (n < 32) return (x >> n) | (x << (32 - n));
    return x;
}
inline uint sig0(uint x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
```
Gets merged into:
```opencl
inline uint sig0(uint x)
{
    return ((2  < 32) ? (x >> 2)  | (x << (32 - 2)  : x) ^
           ((13 < 32) ? (x >> 13) | (x << (32 - 13) : x) ^
           ((22 < 32) ? (x >> 22) | (x << (32 - 22) : x);
}
```
Then simplified into:
```opencl
inline uint sig0(uint x)
{
    return ((x >> 2)  | (x << (30))) ^
           ((x >> 13) | (x << (19))) ^
           ((x >> 22) | (x << (10)));
}
```
Then inserted into the kernel calls as:
```opencl
( ((x>>2)|(x<<(30)))^((x>>13)|(x<<(19)))^((x>>22)|(x<<(10))) )
```

So we are doing this for every single method.
```opencl
inline uint rotr(uint x, int n)
inline uint ch(uint x, uint y, uint z)
inline uint maj(uint x, uint y, uint z)
inline uint sig0(uint x)
inline uint sig1(uint x)
inline uint ep0(uint x)
inline uint ep1(uint x)
```
But the definition for the 256bit context is still going to be done using the preprocessor with 8 32bit integers corresponding to the **Rosetta Code**

|H0|H1|H2|H3|H4|H5|H6|H7|
|---|---|---|---|---|---|---|---|
|0x6a09e667|0xbb67ae85|0x3c6ef372|0xa54ff53a|0x510e527f|0x9b05688c|0x1f83d9ab|0x5be0cd19|

Which gets folded into:  
`764FAF5C61AC315F1497F9DFA542713965B785E5CC2F707D6468D7D1124CDFCF` 
This will serve as our starting point to the algorithm.

After implementing the standard hashing, we will add salt as well. Appending the salt to the end of the password ought to be done on the GPU itself. This way the process will be parallel and less data will be copied between hardware. New kernel:

```opencl
//hash_single_salt.kernel.cl
kernel void sha256kernel_salted(uint salt_length,
                                __global char* salt,
                                uint key_length,
                                __global char* key,
                                __global uint* result)
```

Also a useful feature would be to feed in a file of keys to the gpu, which hashes them, then we write them to a file. To do this effectively, we should convert the __unsigned integer__ keys to __hex strings__. Since I cannot use sprintf, I had to implement a lightweight way of converting on the gpu. This became the end result:

```opencl
const char hex_charset[] = "0123456789abcdef";   
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
result[64] = 0;
```

As you can see, we **unroll** every cycle to keep the instruction flow constant. We push the unsigned integer bistream by 4 bits every time, since every 4 bits represent a single hex character.
For example:

`0110 1010 0000 1001 1110 0110 0110 0111` -> __0x6a09e667__

|6|A|0|9|E|6|6|7|
|---|---|---|---|---|---|---|---|
|0110|1010|0000|1001|1110|0110|0110|0111|

Now we can return a fix character length as result.

```opencl
//hash_multiple.kernel.cl
kernel void sha256multiple_kernel(uint key_length,
                                  __global char* keys,
                                  __global char* results)
```

We now have an increasing amount of features, so I separated them into their own files and created a smaller parameter switch to easily gain access to each .

```
gpu platform
gpu hash single <key>
gpu hash single <key> <salt>
gpu hash multiple <infile> <outfil>
```

## Milestone 5: Implementing hash compare on GPU (current)*

Comparing is going to be similar to the multi hash. For optimization purposes I specified a 16 character maximum length limit for the input keys. The vast majority of passwords are less than that, and it is even used as a hard upper limit on numerous websites.

```opencl
//crack_single.kernel.cl
kernel void sha256crack_single_kernel(uint key_length,
                                      __global char* keys,
                                      __global uint* hash,
                                      __global char* results)
```

In this case however, we pre-calculate the hash in __unsigned integer__ form once on the cpu, so conversion on gpu every time is unnecessary. We save about 3 microseconds per hash.

Hashing, salting and comparing `100,000` entries took `568,904 microseconds` = `0.568904 seconds`. `100,000/0.568904 = 175,776.5809...` => `175.776 khcps`. We can have our first real comparison with the CPU.

|Method|Speed|Relative|
|---|---|---|
|CPU Hash Compare|35.353 khcps|100%|
|CPU Salted Compare|32.196 khcps|91%|
|GPU Hash Compare|175.776 khcps|546%|

This means a __5.5 times__ improvement in the first run, so this proves that cracking on GPU is definitely more potent, at least on a computer like mine.

This version of the program can be accessed with the git key: 

## **Milestone 6: Optimizing kernel (current)**