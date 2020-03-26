# Password Hash Cracking on GPU
by: Richard Nagy

## Roadmap:
1. Implementing standard SHA-256 algorithm on CPU
2. Creating a linear password cracker using a table from file
3. Implementing the algorithm on GPU in OpenCL

## Resources:
[Wikipedia: SHA-2](https://en.wikipedia.org/wiki/SHA-2)
[Have I been pwned: password list](https://haveibeenpwned.com/Passwords)


## Milestone 1: implementing on CPU *(completed)*
Implementing the SHA-256 encryption algorithm on the CPU using exclusively C syntax. It must be capable of receieving a standard c string as input, and generate a 256 bit hash as an output. This means an **n** byte input and a 256 bit = 32 byte = 32 character long string as output.

We have to generate an entirely new stack of variables for each hash, because the values get moved and modified every iteration. I used an object-orinted approach in the first iteration.

Hash generating calls are going to be used in the following way:
```c
const char* hash = sha256("mypassword");
```

## Milestone 2: cracking using password-table *(completed)*
With the algorithm working, we can start implementing the actual cracking. We read one file, with an unknown hash

*target.txt:*
``
b493d48364afe44d11c0165cf470a4164d1e2609911ef998be868d46ade3de4e
``

The file contains the SHA-256 hashed verison of the password: ``banana``. Then we read a list of passwords that are commonly used according to data leaks.

*passwords.txt:*
``
12345
abc123
password
passwd
123456
newpass
...
``

I am going to be using 3 versions in my project.


