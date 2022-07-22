# Password Hash Cracking on GPU

*Richard Antal Nagy, 2020/02 - 2021/04*

## Support ❤️

If you find the project useful, please consider supporting, or contributing.

[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/dubniczky)

## Description

We use hashes to protect user password from being exposed in the event of a breach. Servers use algorithms that create so-called hashes, which are impossible to reverse but even so, you can start trying to guess the password normally it takes so long that is not technically feasible with our current technology.

Dictionary attacks however, are quite common. We take the most used passwords that the user might have, and try every single one with a given hash. This increases our chances significantly and happens to be a great job for a GPU.

In this project I will recreate such a hashing solution from scratch with the SHA-256 algorithm. First I will make a demo version on the GPU in C to serve as a benchmark and as a starting point for the transition to GPU, since C code very close to OpenCL kernel code.

## Roadmap

1. Implementing standard SHA-256 algorithm on CPU
2. Creating a linear password cracker using a table from file
3. Implementing salt into the algorithm
4. Implementing the algorithm on GPU in OpenCL
5. Implementing hash compare on GPU
6. Optimizing Kernel Iteration 1
7. Implementing salt compare on GPU
8. Optimizing Kernel Iteration 2
9. **Project complete (current)**

## Resources

- [Wikipedia: SHA-2](https://en.wikipedia.org/wiki/SHA-2) *algorithm*
- [Wikipedia: Base-64](https://en.wikipedia.org/wiki/Base64) *storing in uints*
- [Wikipedia: Salt (Crypgoraphy)](https://en.wikipedia.org/wiki/Salt_(cryptography)) *salt*
- [Have I been pwned](https://haveibeenpwned.com/Passwords) *common password lists*
- [Xorbin hash](https://xorbin.com/tools/sha256-hash-calculator) *verify results*
- [ELTE Computer Graphics](http://cg.elte.hu/index.php/gpgpu/) *custom opencl c++ library*
- [NVIDIA OpenCL](https://www.nvidia.com/content/cudazone/CUDABrowser/downloads/papers/NVIDIA_OpenCL_BestPracticesGuide.pdf) *best practices guide*
- [Radeon GPU Analyzer](https://gpuopen.com/gaming-product/radeon-gpu-analyzer-rga/) kernel disassemble for optimizing

## Baseline hardware

| Component | Baseline |
|---|---|
|**MB**|ASUS Prime X470-PRO|
|**CPU**|Ryzen 7 2700X *8c/16t 4.00Ghz @ base clock*|
|**RAM**|Corsair Vengeance 2x8GB 2400Mhz DDR4 dual channel|
|**GPU**|Nvidia Geforce GTX 1070: *6.463 Teraflops*|
|**SSD**|Samsung 970 EVO 250GB|

*I will use this hardware for all the benchmarking unless stated otherwise.*

## Custom metric (hcps)

We are going to use a custom metric to compare results: **Hash Compare Per Second** (hashcomp/sec).
This of course means the amount of hashes we can compare every given second. This does NOT include the time to start the cracking itself and gathering the data at the end. We are only interested in the hash time itself since the program only starts once, but it can keep running for hours, days or weeks.. until we run out of samples to feed it.

Being a scalar unit, we can even use prefix multipliers:
`1,000,000,000 hcps` = `1,000,000 khcps` = `1,000 Mhcps` = `1 Ghcps`

## Milestone 1: implementing on CPU *(completed)*

Implementing the **SHA-256** encryption algorithm on the CPU using exclusively C syntax for easier port on GPU. It must be capable of receiving a standard C string as input, and generate a 256 bit hash as an output. This means an **n** byte input and a ``256 bit = 32 byte = 64 character base64`` string as output.

We have to generate an entirely new stack of variables for each hash, because the values get moved and modified every iteration. I used an object-orinted approach in the first iteration.

Hash generating calls are going to be used in the following way:

```c
const char* hash = sha256("mypassword");
```

This of course is going to change later, but it helped isolate the variables and code needed exclusively for the hasing algorithm.

I will use the standard way of using this algorithm:

1. Initializing the SHA256 context (*init*)
2. Padding the key to be **n * 2<sup>64</sup>** (*update*)
3. Transforming the blocks (*transform*)
4. Unpacking the digest (*final*)

```c
//Methods
init();
update(const unsigned char* message, unsigned int length);
transform(const unsigned char* message, unsigned int block);
final(unsigned char* digest);

//Printing the answer
char buf[2 * DIGEST_SIZE + 1];
for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
{
    sprintf(buf + i * 2, "%02x", digest[i]);
}
buf[2 * DIGEST_SIZE] = 0;
```

We get the answer in a fixed 8 length **unsigned int** block array, which we convert to hexadecimal string using sprintf.

The definition for the 256 bit context is going to be 8x32 bit integers corresponding to the 32 bits of the fractional parts of the square roots of the first eight prime numbers, which is coincidentally the **base64** form of: `Rosetta Code`

||H0|H1|H2|H3|H4|H5|H6|H7|
|---|---|---|---|---|---|---|---|---|
|0x|6a09e667|bb67ae85|3c6ef372|a54ff53a|510e527f|9b05688c|1f83d9ab|5be0cd19|

Which gets folded into:  
`764FAF5C61AC315F1497F9DFA542713965B785E5CC2F707D6468D7D1124CDFCF`
This will serve as our starting point to the algorithm.

These values can be easily calculated with the following **javascript** code by just pasting it into the browser console or NodeJS rutime:

```c
(() => {
[2,3,5,7,11,13,17,19].forEach((i) => 
    console.log(parseInt((Math.sqrt(i) % 1).toString(2).slice(2, 34), 2).toString(16)))
})()
```

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
|3,721,224|[passwords-4m.txt](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-4m.txt)|

The code can be compiled and run in the following way:

```bash
g++ main.cpp -o crack.exe && .\crack.exe target.txt passwords.txt
```

Example output by running with the **100k** list:

```txt
Matching...
Match found: banana
Checked lines: 436
Search time: 12666 microseconds
```

Let us try the worst case-scenario, when the correct match is not in the file. Then we can make some calculations. I am using the baseline computer for every test.

```txt
Matching...
Match not found
Checked lines: 100000
Search time: 2828609 microseconds
```

So hashing and comparing ``100,000`` entries took ``2,828,609 microseconds`` = ``2.828609 seconds``. As a baseline, we are going to use the custom metric: **hashcomp/sec (hcps)**.
`100,000/2.828609 = 35,353.0658...` so we are at about ``35.353 khcps``.

## Milestone 3: Implementing salt *(completed)*

The current method of cracking seems unnecessary, since we could just pre-calculate the hashes and start comparing every time without needing to do the hashing every time.

This is why [salts](https://en.wikipedia.org/wiki/Salt_(cryptography)) are commonly used in password storing. Salts are a random series of characters, that are attached to the end of the password before being hashed. Here are two a salted hashes for **banana** with 4 byte salts:  

``banana`` => ``bananakQ9wvI9A`` => ``50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee``  
``banana`` => ``bananaJoz1BL1T`` => ``7da2b105a959cff3b2c03c0c15fa11fa124636a21451eeead00cb7654c664f7e``  

The same password can take up multiple forms, thus making so that if a hash is cracked, we are still unable to match every single instance of the same password. This is why it is **essential** that salts remain unique in every set of passwords, for example in a login database. This way no passwords can be equated just by looking at their hashed forms.

Now however, we will need the salt to store the password as well, so we are just going to append it to the start of the hash:  

`kQ9wvI9A` `50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee`  
`kQ9wvI9A50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee`  

We of course know, that this is a 256 bit hash, so there must be 64 characters for the hash, and every single before that is the salt. Te length of the salt can be any number of characters, so we have to be flexible about that.

```c
data_length = { n | n >= 64 }
hash_length = 64
salt_length = data_length - 64
```

Example run of the banana search using salts as well:

```bash
Target: 'kQ9wvI9A50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee'
Salt: 'kQ9wvI9A'
Hash: '50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee'
Reading password table...
Matching...
Match found: banana
Checked lines: 436
Search time: 12955 microseconds
```

We simply remove the salt from the original target, and append it to every single  password in our list. This certainly makes it slower, so let us do our baseline test again with an **8 byte salt**:

```bash
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
`100,000/3.106065 = 32,195.0764...` => `~32.196 khcps`. This method seems about `9%` slower, but keep in mind that it will depend on the size of the salt. That however is almost never bigger than 16 bytes.

|Method|Speed|Relative|
|---|---|---|
|Hash Compare|35.353 khcps|100%|
|Salted Compare|32.196 khcps|91%|

## Milestone 4: Implementing SHA-256 on GPU *(completed)*

In this step, the most difficult part is of course writing the kernel itself. It has to be able to calculate a single hash given a string and its length. The result length is fixed, so there will be no problems with that.

Kernel definition:

```c
//hash_single.kernel.cl
kernel void sha256single_kernel(uint key_length,
                                global char* key,
                                global uint* result)
```

We can then feed the information using global memory buffers. Previously we defined some macros to speed up the code, which is not going to be necessary in this case, since the compiler merges every **inline** method into the kernel.

Example:

```c
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

```c
inline uint sig0(uint x)
{
    return ((2  < 32) ? (x >> 2)  | (x << (32 - 2)  : x) ^
           ((13 < 32) ? (x >> 13) | (x << (32 - 13) : x) ^
           ((22 < 32) ? (x >> 22) | (x << (32 - 22) : x);
}
```

Then simplified into:

```c
inline uint sig0(uint x)
{
    return ((x >> 2)  | (x << (30))) ^
           ((x >> 13) | (x << (19))) ^
           ((x >> 22) | (x << (10)));
}
```

Then inserted into the kernel calls as:

```c
( ((x>>2)|(x<<30))^((x>>13)|(x<<19))^((x>>22)|(x<<10)) )
```

So we are doing this for every single method.

```c
inline uint rotr(uint x, int n)
inline uint ch(uint x, uint y, uint z)
inline uint maj(uint x, uint y, uint z)
inline uint sig0(uint x)
inline uint sig1(uint x)
inline uint ep0(uint x)
inline uint ep1(uint x)
```

After implementing the standard hashing, we will add salt as well. Appending the salt to the end of the password ought to be done on the GPU itself. This way the process will be parallel and less data will be copied between hardware. The new kernel:

```c
//hash_single_salt.kernel.cl
kernel void sha256kernel_salted(uint salt_length,
                                global char* salt,
                                uint key_length,
                                global char* key,
                                global uint* result)
```

Also a useful feature would be to feed in a file of keys to the gpu, which hashes them, then we write them to a file. To do this effectively, we should convert the **unsigned integer** keys to **hex strings**. Since I cannot use sprintf as I did in C, I had to implement a lightweight way of converting on the gpu. This became the end result:

```c
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

0x6a09e667 = `0110 1010 0000 1001 1110 0110 0110 0111`

|6|A|0|9|E|6|6|7|
|---|---|---|---|---|---|---|---|
|0110|1010|0000|1001|1110|0110|0110|0111|

Now we can return a fix character length as result.

```c
//hash_multiple.kernel.cl
kernel void sha256multiple_kernel(uint key_length,
                                  __global char* keys,
                                  __global char* results)
```

We now have an increasing amount of features, so I separated them into their own files and created a smaller parameter switch to easily gain access to each .

```bash
gpu platform
gpu hash single <key>
gpu hash single <key> <salt>
gpu hash multiple <infile> <outfile>
```

## Milestone 5: Implementing hash compare on GPU *(completed)*

Comparing is going to be similar to the multi hash. For optimization purposes I specified a 16 character maximum length limit for the input keys. The vast majority of passwords are less than that, and it is even used as a hard upper limit on numerous websites.

```c
//crack_single.kernel.cl
kernel void sha256crack_single_kernel(uint key_length,
                                      global char* keys,
                                      global uint* hash,
                                      global char* results)
```

In this case however, we pre-calculate the hash in **unsigned integer** form once on the cpu, so conversion on gpu every time is unnecessary. We save about 3 microseconds per hash.

Hashing and comparing `100,000` entries took `568,904 microseconds` = `0.568904 seconds`. `100,000/0.568904 = 175,776.5809...` => `~175.776 khcps`. We can have our first real comparison with the CPU.

|Method|Speed|Relative|
|---|---|---|
|CPU Hash Compare|35.353 khcps|100%|
|CPU Salted Compare|32.196 khcps|91%|
|GPU Hash Compare|175.776 khcps|546%|

This means a **~5.5 times** improvement in the first run, so this proves that cracking on GPU is definitely more potent, at least on a computer like mine.

This version of the program can be accessed with the git commit SHA: `5161a028`
Or you can download it from the tagged releases page: [Release v1.0](https://gitlab.com/richardnagy/passhash/-/tags/v1.0)

## Milestone 6: Optimizing Kernel Iteration 1 *(completed)*

### Summary

| No. | Optimization Attempt | Performance Delta | Conclusion (keep?) |
| --- | --- | --- | --- |
| 1 | Since we pass only one hash in the entire life of the kernel, I tried adding it using pre-compiler directives. This however did not result in a performance delta above margin of error. | ~ 0% | Revert Changes |
| 2 | While the kernel is running, we can already start reading in the next lines from the file. The reading will take longer than the hashing, but we can get a bit of performance by going asynchronous. This of course requires double buffering, which is a minimal additional memory. | ~ +6% | Keep Changes |
| 3 | A majority of the time is taken up by reading the data. Especially using the slower C++ tools compared to standard C. So I rewrote the reading algorithm and redirected the input into the buffer immediately, skipping the string buffer. | ~ +500% | Keep Changes |

### Details

#### 1. Attempt: *preprocessor hash*

Instead of passing in the hash as a **global** buffer, I did it with macros. The compiler seemingly optimizes single bulk read from global buffer well, so this did not improve performance above margin of error.

Chances have been reverted, however the scaffolding for passing in build options for the compiler remained.

#### 2. Attempt: *implementing kernel events*

The kernel events have been synchronous so far, which is a waste of a small amount of time. To combat this, I implemented a **double-buffering**-like workflow, which works the following way:

1. Read the first segment file into the first buffer.
2. Copy first buffer to gpu and start hashing async.
3. Read next segment of the file into second buffer.
4. Wait for kernel to be completed (*it almost always already is at this point*)
5. Check result from kernel.
6. Copy second buffer to gpu and start hashing async.
7. Read next segment of the file into second buffer.
8. ...

This works by having 2 key buffers and switching a pointer between them. This of course means we need more RAM, but that is less of a constraint in this case, since if we crack `256 keys` with a maximum length of `16` at once, we will need `256*(16+1) = 4357 byes = 4.25 Kb` extra memory.

```cpp
//Event logic
cl::vector<Event> eventQueue;

// ... Read buffer (*)

queue.enqueueWriteBuffer(keyBuffer, CL_FALSE, 0,
                         MAX_KEY_SIZE * keyCount, currentBuffer,
                         &eventQueue);
queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                           globalRange, cl::NullRange,
                           NULL, &eventQueue[0]);
queue.enqueueReadBuffer(resultBuffer, CL_FALSE, 0,
                        keyCount, result, &eventQueue);

// ... Switch then read next buffer

eventQueue[0].wait();
                        
// ... Validate result
//Back to (*) until end of file, or found result
```

#### 3. Attempt: *file read optimization*

Currently we waste a lot of time by using **C++ iostream**, since it works with **std::string**s and it makes copying to our buffer really slow. Here are the steps we do with it:

1. It copies the data from **std::ifstream** to **std::stringstream**
2. It converts the data from **std::stringstream** to **std::string**
3. It then returns the data to me and I convert the string to **const char[]**
4. As the last step I copy the items from the arra to the buffer.

```cpp
std::ifstream infile(fileName);

// ...

std::string line;
for (int i = 0; i < hashThreadCount && std::getline(infile, line); i++)
{
    strcpy(&inputBuffer[MAX_KEY_SIZE * i], line.c_str());
}

// ...

infile.close();
```

I rewrote this using a standard C approach:

```c
FILE* infile = fopen(fileName, "r");

// ...

for (int i = 0;
     i < hashThreadCount && fgets(&currentBuffer[MAX_KEY_SIZE * i], MAX_KEY_SIZE, infile) != NULL;
     i++)
     { }

// ...

fclose(infile);
```

This looks very similar, but it actually only makes one simple step. Starts reading the file until a `\n` character into the buffer itself. It does not even remove the end-line character form the key, but we will do that in the kernel. It's faster that way.

The results from this step turned out to be a massive improvement. So at this point, we should do our benchmark again.

Hashing and comparing `100,000` entries took `86,424 microseconds` = `0.086424 seconds`. `100,000/0.086424 = 1,157,085.9946...` => `~1,157.085 khcps`.

|Method|Speed|Relative|
|---|---|---|
| CPU Hash Compare | 35.353 khcps | 100% |
| CPU Salted Compare | 32.196 khcps | 91% |
| GPU Hash Compare | 175.776 khcps | 546% |
| GPU Optimization 1 | 1,157.085 khcps | 3273% |

This of course means about **~33 times** improvement over standard single CPU core. I'm using an SSD to store the password dictionary. This would be significantly lower if I used a HDD instead.

Also keep in mind, that the preparation to start the hashing is longer in case of the GPU kernel. This is not included into the hash speed, since it is only done once in the beginning and it get insignificant in the case of bigger datasets, which this program is designed for.

## Milestone 7: Implementing salt compare on GPU *(completed)*

At this point we have quite a few variables that are constant during the whole life of the kernel:

- Hash
- Salt length
- Key length
- Salt

We can define these with the pre-compiler, to save the time of having to make buffers and pass them as parameters. We use the **uint array** version of the hash to spare a whole set of computations.

```opencl
#DEFINE HASH_0 ...
#DEFINE HASH_1 ...
#DEFINE HASH_2 ...
#DEFINE HASH_3 ...
#DEFINE HASH_4 ...
#DEFINE HASH_5 ...
#DEFINE HASH_6 ...
#DEFINE HASH_7 ...
#DEFINE SALT_LENGTH ...
#DEFINE SALT_STRING ...
#DEFINE KEY_LENGTH ...
```

The problem is that we don't actually know these in the kernel, so we have to add them as parameters for the compiler.

```c
sprintf(buildOptions,
    "-D HASH_0=%u -D HASH_1=%u \
    -D HASH_2=%u -D HASH_3=%u \
    -D HASH_4=%u -D HASH_5=%u \
    -D HASH_6=%u -D HASH_7=%u \
    -D KEY_LENGTH=%d \
    -D SALT_LENGTH=%d \
    -D SALT_STRING=\"%s\"",
    hash[0], hash[1],
    hash[2], hash[3],
    hash[4], hash[5],
    hash[6], hash[7],
    MAX_KEY_SIZE, saltLength, salt
);
```

This still has one problem. You can't actually pass a string as a macro, so I had to convert it with the preprocessor of the kernel.

```c
//Helper macros
#define STR(s) #s      //Takes macro and returns it as a string
#define XSTR(s) STR(s) //Takes macro and passes its value to be stringified

//Usage:
char salt[] = XSTR(SALT_STRING); //Returns the value SALT_STRING as a string
```

This way we can achieve just a tiny overhead when using salts in the gpu, since we only need to append the constant string. Also we know it's length to be a constant, so we can **unroll** the cycle.

```c
#pragma unroll
for (unsigned int j = 0; j < SALT_LENGTH; j++)
{
    key[length + j] = XSTR(SALT_STRING)[j];
}
length += SALT_LENGTH;
```

As you can see, if we disassemble this, the memory is going to be set statically. This makes it nearly instantaneous, so I couldn't detect a performance difference  above margin of error.

```asm
; Disassembled using Radeon GPU Analyzer
v_add_u32_e32       v4, v0, v2
v_mov_b32_e32       v5, 0x67
ds_write_b8         v4, v5
v_mov_b32_e32       v5, 0x6f
ds_write_b8         v4, v5 offset:1
ds_write_b8         v4, v5 offset:2
v_mov_b32_e32       v5, 0x64
ds_write_b8         v4, v5 offset:3
```

One thing that makes is slower however, is having to copy the key string as well. Also longer key means more steps during hashing, so we get some performance penalty here.

Hashing, salting and comparing `100,000` entries took `88,469 microseconds` = `0.088469 seconds`. `100,000/0.088469 = 1,130,339.4409...` => `~1,130.339 khcps`.

Hash compare was `1,130.339 khcps`, so this means about `~2.3%` lost performance if we are using an 8 bit salt.

|Method|Speed|Relative|
|---|---|---|
| CPU Hash Compare | 35.353 khcps | 100% |
| CPU Salted Compare | 32.196 khcps | 91% |
| GPU Hash Compare | 175.776 khcps | 546% |
| GPU Optimization 1 | 1,157.085 khcps | 3273% |
| GPU Salted Compare | 1,130.339 khcps | 3197% |

## Milestone 8: Optimizing Kernel Iteration 2 *(current)*

### GPU Thread Count

So far we are reading and then cracking a fixed amount of `256` keys every iteration. This of course is a low amount for the vast majority of video cards nowadays. We are doing the `copying & hashing` and `reading from file` at the same, but the copy+hash so far takes much shorter than reading. This of course can be fixed be feeding in more data at once.

The optimal thread count is lower than the maximum allocation size of the gpu and around a **sweetspot** we don't actually know yet. So I tested it out:

|Threads| Crack Time (microsec) | Time Delta | Total Time |
|---|---|---|---|
| 256 | 89681 | - | 100% |
| 1024 | 52282| -32% | 58% |
| 4096 | 43537| -9% | 49% |
| 16,384 | 42589| -2% | 47% |
| 32,768 | 40481| -2% | 45% |
| 65,536 | 42880| +3% | 48% |
| 131,072 | 42892| +0% | 48% |

So the optimal size for my setup is somewhere between `32,768` and `65,536`. I continue the trials by halving the intervals and testing which side is minimal. The best result came from `46,960` threads with `39,885` microseconds. Which seems promising, so let's benchmark again.

Hashing, salting and comparing `100,000` entries took `39,885 microseconds` = `0.039885 seconds`. `100,000/0.039885 = 2,507,208.2236...` => `~2,507.208 khcps`.

|Method|Speed|Relative|
|---|---|---|
| CPU Hash Compare | 35.353 khcps | 100% |
| CPU Salted Compare | 32.196 khcps | 91% |
| GPU Hash Compare | 175.776 khcps | 546% |
| GPU Optimization 1 | 1,157.085 khcps | 3273% |
| GPU Salted Compare | 1,130.339 khcps | 3197% |
| GPU Optimization 2 | 2,507.208 khcps | 7092% |

This means the GPU hash cracking is now `71 times` faster than using my single thread CPU.

### Bulk cracking

The comparisons were impressive enough already, but there is one more thing. In the case of the GPU methods I included the time to read the data in the calculations as well. Which is the correct way, but I did not do that back with the CPU version. So let's exclude it for a final comparison.

Hashing, salting and comparing `100,000` entries took `2,575 microseconds` = `0.002575 seconds`. `100,000/0.002575 = 38,834,951.4563...` => `~39 Mhcps` .

|Method|Speed|Relative|
|---|---|---|
| CPU Hash Compare | 35.353 khcps | 100% |
| CPU Salted Compare | 32.196 khcps | 91% |
| GPU Hash Compare | 175.776 khcps | 546% |
| GPU Optimization 1 | 1,157.085 khcps | 3273% |
| GPU Salted Compare | 1,130.339 khcps | 3197% |
| GPU Optimization 2 | 2,507.208 khcps | 7092% |
| GPU O2 *Readtime Excluded* | 38,834.951 khcps | 109 849% |

This means the GPU hash cracking true power is `1098 times` faster than using my single thread CPU, with 39 Mega hash compares per second.

## Project complete *(current)*

This has been a great project and a spectacular learning experience for me in the field of parallel computing. I would like to especially thank **Iván Eichhardt** from *ELTE Computer Graphics department* for being available to help when I hit some roadblocks.

And of course, thank you for reading.

Richard Nagy,
2020/04
