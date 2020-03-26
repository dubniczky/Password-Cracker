# __Password Hash Cracking on GPU__
by: Richard Nagy

## Roadmap:
1. Implementing standard SHA-256 algorithm on CPU
2. Creating a linear password cracker using a table from file
3. __Implementing salt into the algorithm (current)__ 
4. Implementing the algorithm on GPU in OpenCL

## Resources:
[Wikipedia: SHA-2](https://en.wikipedia.org/wiki/SHA-2)
[Wikipedia: Base-64](https://en.wikipedia.org/wiki/Base64)
[Wikipedia: Salt (Crypgoraphy)](https://en.wikipedia.org/wiki/Salt_(cryptography))
[Have I been pwned: password list](https://haveibeenpwned.com/Passwords)

## Baseline hardware:
* __CPU:__ Ryzen 7 2700X 8c/16t 4.00Ghz base clock
* __RAM:__ 2x8Gb 2400Mhz DDR4 dual channel
* __GPU:__ Nvidia Geforce GTX 1070


## Milestone 1: implementing on CPU *(completed)*
Implementing the SHA-256 encryption algorithm on the CPU using exclusively C syntax. It must be capable of receieving a standard c string as input, and generate a 256 bit hash as an output. This means an **n** byte input and a ``256 bit = 32 byte = 64 character`` long __base64__ string as output.

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

I am going to be using 3 versions in my project:
* [Top: 100](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-100.txt)
* [Top: 100 000](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-100k.txt)
* [Top: 4 000 000](https://gitlab.com/ivolv/passhash/-/blob/master/passwords/passwords-4m.txt)

The code can be compiled and run in the following way:
```bash
g++ main.cpp -o crack.exe && .\crack.exe target.txt passwords.txt
```

Example output by running with the Top: 100k list:
```
Matching...
Match found: banana
Checked lines: 436
Search time: 12666 microseconds
```

Let us try the worst case-scenario, when the correct match is not in the file. Then we can make some calculations. I am using a __Ryzen 7 2700X CPU__ for every test.
```
Matching...
Match not found
Checked lines: 100000
Search time: 2828609 microseconds
```
So hashing and comparing ``100,000`` entries took ``2,828,609 microseconds`` = ``2.828609 seconds``. As a baseline, we are going to use a custom metric: __hashcomp/sec (hcs)__.
`100000/2.828609 = 35,353.0658...` so we are at about ``35.353khcs``.

## Milestone 3: Implementing Salt *(current)*
The current method of cracking seems unnecessary, since we could just pre-calculate the hashes and start comparing every time, without needing to do the hashing every time.

This is why [salts](https://en.wikipedia.org/wiki/Salt_(cryptography)) are commonly used in password storing. Salts are a random series of characters, that are attached to the end of the password before being hashed. Here are two a salted hashes for __banana__ with 4 byte salts:
``banana`` => ``bananakQ9wvI9A`` => ``50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee``
``banana`` => ``bananaJoz1BL1T`` => ``7da2b105a959cff3b2c03c0c15fa11fa124636a21451eeead00cb7654c664f7e``
The same password can take up multiple forms, thus making so that if a hash is cracked, we are still unable to match every single instance of the same password.

Now however, we will need the salt to store the password aswell, so we are just going to append it to the start of the hash:
``kQ9wvI9A`` ``50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee``
``kQ9wvI9A50622ccfa4c8f58bd952b62f7fafe47511fec498985921d6b13ac178cb413aee``
We of course know, that this is a 256 bit hash, so there must be 64 characters for the hash, and every single before that is the salt. Te length of the salt can be any number of characters, so we have to be flexible about that.


