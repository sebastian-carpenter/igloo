# igloo
A password obfuscation manager

![igloo 'logo'](./igloo.png)

You might be asking what that is... so here's a problem to illustrate the goal:

People today have many accounts distributed across many websites. Ideally, each
person maintains a different password for each website and each password is
secure. Non-ideally, but more convenient, is the use of a single secure password
for many websites.

In the ideal case many passwords must be rememebered or written down. But in the
event that an attacker determines a password (via data breach, brute force, or
any number of nefarious methods) then only one password - one account - is
compromised. For the non-ideal user many accounts are compromised.

igloo solves this problem of comprimised passwords by obscuring the password
before it is given to the website. With a proper obfuscation process the user
should be able to use simpler passwords to achieve the same level of security.
Here are a few ways in which a user might achieve this ideal level of security:

* one profile, one simple password (salt required)
* one profile, many simple passwords
* many profiles, one simple password
* many profiles, many simple passwords

__note: a profile in igloo is equivalent to the **obfuscation process**__

one profile, one simple password is given as a separate solution assuming that
the user comes up with a method for salting their password. A possible method
for this would be using the website name as the salt.

## igloo Profiles

As noted above an igloo profile is the process for obscuring one's password. A
profile is composed of a sequence of beffudles, as they are called, that
deterministically alter the input password. That is, if a user provides some
password to some profile they can always expect that operation to return the
same obscured password. Because all operations in igloo are deterministic this
will always be the case.

There may be some undefined behavior based on hardware, but running `./test.sh`
should be a good indicator if things are working correctly.

# Getting Started

Building igloo requires:

* autotools: autoreconf, autoconf, automake, libtool
* wolfSSL (can be disabled, see [Installing igloo])

## Installing wolfSSL

Start by cloning the wolfSSL repo found at
[https://github.com/wolfSSL/wolfssl](https://github.com/wolfSSL/wolfssl).

Then follow these steps:

```
cd wolfssl
./autogen.sh
./configure --enable-sha256 --enable-sha512 --enable-sha3 --enable-blake2s \
        --enable-blake2 --enable-shake128 --enable-shake256
make
sudo make install
```

`autogen.sh` is a script that helps setup the configure script and generates a
precursor to the final Makefile.

`configure` is a script which enables particular options / algorithms in
wolfSSL. In the case of igloo it is used to enable all the hash algorithms which
are desired. If you (the user) do not want some of the hash algorithms present
in the configure feel free to remove their enable. igloo can detect this later
and will omit compilation of the associated code.

`make` will compile wolfSSL with the selected options.

At this points `tests/unit.test` could be run to verify everything is working as
it should.

`sudo make install` will install wolfSSL to a standard location where igloo can
then find it.

## Installing igloo

Installing igloo follows much of the same process since it uses autotools as
well.

```
./autogen.sh
./configure
make
./setup.sh
./test.sh
sudo make install
```

If wolfSSL is not desired then it can be disabled with `--disable-wolfssl` when
configuring (which will remove all hashing capability).

```
./configure --disable-wolfssl
```

`setup.sh` will create ~/.igloo/ for storing a user's profiles. As well as
generating the __default__ profile inside the aforementioned directory.

`test.sh` is a script which should be run to verify the functioning of the
program. The hope is that if something is broken this will find it and serve
as a warning ahead of time.

After running `sudo make install` the `igloo` command should work. If
`igloo --help` does not give any output it may be necessary to add
`/usr/local/lib` to your PATH (or wherever else `igloo` was installed).

# Using igloo

Two scenarios will be explored here: using the default profile, and using a
custom profile.

## Default Profile

The quick and dirty use case for igloo.

```
igloo
```

After running this command a prompt similar to the following should show up.

```
no commands parsed, loading default profile
enter password:
```

During this process terminal echo is turned off, similar to sudo, so do not be
alarmed if keystrokes do not show up. For the moment assume 'hello' is entered
as the password.

```
enter salt:
```

A similar prompt to before shows up. Enter the desired salt, for this example
'salt' will be entered. And the output should be as follows:

```
\Xm^eAxDBk[p&O@?\*wyLowX#dl2\1,x
```

### Truncation

If you desire a password which is only 12 characters long then the `-t` flag can
be passed to igloo with an integer argument. This will restrict how many
password characters are printed.

```
$ igloo -t 12
no commands parsed, loading default profile
enter password: 
enter salt: 
\Xm^eAxDBk[p
```

### Script Usage (the quiet flag)

Because terminal echo is normally turned off for user input, passing information
to igloo can be a bit of a hassle. To avoid this problem use the `-q` flag. Now
strings can be piped to igloo quite easily.

```
$ echo -e "hello\nsalt" | igloo -q
\Xm^eAxDBk[p&O@?\*wyLowX#dl2\1,x
```

### Multiple Passwords (or attempts)

If multiple passwords are going to be input then the `-p` flag can be used along
with an integer argument for the number of passwords to obscure. If 0 is given
then there is no limit on the number of passwords.

```
$ igloo -p 2
no commands parsed, loading default profile
enter password:
enter salt:
\Xm^eAxDBk[p&O@?\*wyLowX#dl2\1,x
enter password:
enter salt:
iT2(i9jZZi!`-4:UD~GAr;<[nsLR/<{-
```

### Output a Password (or multiple)

Using the `-o` flag sets which file to write the obscured password to.

```
$ igloo -o my_pass
no commands parsed, loading default profile
enter password:
enter salt:
$ cat my_pass
\Xm^eAxDBk[p&O@?\*wyLowX#dl2\1,x
```

Note that when outputting an obscured password to stdout (no `-o` flag) a
newline will always be appended, whereas with `-o` no newline will be appended.

If multiple passwords are obscured and `-o` is given then multiple files will be
written to. Each file will be suffixed with a number corresponding to their
order.

```
$ igloo -o my_pass -p 2
no commands parsed, loading default profile
enter password:
enter salt:
enter password:
enter salt:
$ ls
my_pass1 my_pass2
```

## Custom Profile

Using the befuddles in igloo it is possible to create your own profile. And if a
profile isn't desireable then it is possible to parse directly from the command
line instead.

### Parsing igloo Commands

If a command contains any arguments then it is necessary to enclose it in
quotes. Multiple commands can be contained in the same set of quotes by
delimiting them with commas. Here is an example of these concepts:

```
igloo "cat front hello" "cat front a, cat front b"
```

If the password 'jello' is entered then the output would be 'bahellojello'.

Now, taking this a step further. Escape sequences are also supported by igloo.
The two escape sequences supported are: hexadecimal (x, X), and every other
possible character. Hexadecimal sequences will only intepret two characters
(1 byte). Becuase of this igloo is capable of receiving **any string of bytes as
a password**. Though due to parsing, unprintable characters should be reduced to
a hexadecimal escape sequence.

```
igloo 'cat front a\x20b, cat front \,'
```

If the password 'jello' is entered then the output would be 'a b,jello'

### Saving a Profile

With a desirable set of commands just use the `-s` flag to save the profile. If
the profile name is not an absolute path then it will be saved in the
`~/.igloo/` directory. If it is an absolute path then it is saved in the
specified file.

```
igloo -s my_profile "cat front relative"
```

```
igloo -s /home/user/my_absolute_profile "cat front absolute"
```

The first example will be stored in `~/.igloo/my_profile` and the second in
`home/user/my_absolute_profile`. Both profiles will contain the single command.

### Loading a Profile

If there is a profile to load then the `-l` flag will iterpret profile paths in
the same way as `-s`.

```
igloo -l my_profile
```

```
igloo -l /home/user/my_absolute_profile
```

Example one will prefix the password with 'relative' and example two will prefix
the password with 'absolute', based on what was saved in [Saving a Profile].

### Writing a Profile Manually

When igloo saves a profile it will save each command on a newline and there will
be limited whitespace; however, these restrictions are not necessary to follow.

* Leading whitespace is permitted
* Whitespace after a comma is permitted
* Commas may separate commands
* Blank lines are allowed

```
cat front a

    cat front b

    cat front c, cat front d
```

If this were a text file named 'indented_profile' then loading it and running it
with the input 'hello' would produce 'dcbahello'.

# Beffudles

igloo accepts many COMMAND types which may also be referred to as a befuddle.
These befuddles are various algorithms available to the user for obscuring a
password.

A befuddle may accept betwwen 0 and 3 arguments (currently). Each argument has a
type associated with it, these types are: INT, SIZE_T, INDEX, PERCENT, STRING,
MIXED, and NONE.

* INT is some signed number
* SIZE_T is some unsigned number
* INDEX is an alias for SIZE_T, it can be converted to a PERCENT type
    - PERCENT is created when an INDEX arg is given an integer suffixed with a
    percent symbol (i.e., `substr 25% 75%` or `cut 53%`)
* STRING is just an array of characters
* MIXED depends on the befuddle in question
* NONE describes no argument

Following are all the befuddles currently in igloo.

## Standard Befuddles

* cat MIXED STRING
    - concatenate a string to the front or back of a password.
    - MIXED may be 'front' or 'back'
* salt MIXED
    - Similar to cat except STRING is read from stdin
* sub STRING STRING SIZE_T
    - Substitute string1 with string2 up to SIZE_T times (0 is unbounded)
* cut INDEX
    - Split the password into two sections at INDEX then swap both sections
* rev
    - Reverse the password
* xor STRING
    - Perform an xor operation on the password using STRING
* replace STRING INDEX
    - Replace characters starting at INDEX with STRING
* shuffle SIZE_T
    - Shuffle each character in the password like a deck of cards
* ccipher INT
    - Perform a caesar cipher with shift INT
* rcipher SIZE_T
    - Perform a rail cipher with SIZE_T rails
* complement
    - Bitwise complement each password character
* genxor SIZE_T
    - Using an internal PRNG generate a string to xor with the password
    - SIZE_T selects the seed (0 seeds based on the first 8 characters)
* passwordify SIZE_T STRING
    - Similar to genxor but xor a character until it is printable
    - STRING identifies additional characters that should also be excluded

## Meta Befuddles

* fork
    - Operate on a duplicate password which must later be joined
* substr INDEX INDEX
    - Fork a portion of the password from the first INDEX to the second INDEX
* join
    - Join a fork to the main password again
    - Supported methods for joining: CAT, SUB, XOR, REPLACE
        * cat MIXED COMMENT
        * sub STRING COMMENT SIZE_T
        * xor COMMENT
        * replace COMMENT INDEX
    - In each case COMMENT will be ignored so anything can be written
    - See [Join Examples]
* iterate SIZE_T
    - Mark the beginning of a loop that should run for SIZE_T iterations
* iterate_end
    - Mark the end of a loop

## Hashes

* sha2-256
    - 32 byte hash
* sha2-512
    - 64 byte hash
* sha3-256
    - 32 byte hash
* sha3-512
    - 64 byte hash
* blake2s SIZE_T
    - Up to 32 byte hash, SIZE_T selects the digest size
* blake2b SIZE_T
    - Up to 64 byte hash, SIZE_T selects the digest size
* shake128 SIZE_T
    - Unrestricted digest size, SIZE_T selects the digest size
* shake256 SIZE_T
    - Unrestricted digest size, SIZE_T selects the digest size

## Join Examples

Four types of joins are currently supported, these being CAT, SUB, XOR, and
REPLACE. They expect the following arguments:

* cat MIXED COMMENT
* sub STRING COMMENT INDEX
* xor COMMENT
* replace COMMENT INDEX

COMMENT is not a type recognized by the igloo parser. It exists as a
consequence of parsing being reused for the JOIN logic. Consequently, each
COMMENT is actually just the normal STRING type being ignored.

The STRING type can be ignored because the FORK'd string is substituted for what
would normally be expected.

A few examples should make things a bit clearer.

```
igloo fork 'join cat\ front\ FORK1'
```

This duplicates the password and then immediately concatenates it to the front
of the password which was the origin. Notice that the spaces are escaped because
otherwise parsing JOIN would throw an error. Doing it like this allows the
argument to be 'cat front FORK1' which can later be parsed by the JOIN logic.

JOIN will get 'cat front FORK1' and it will parse all of it into a CAT COMMAND.
The COMMENT section is irrelevant to the JOIN logic so it is ignored. Besides
that 'cat' and 'front' tell it to concatenate the forked password to the front
of the password it was duplicated from.

```
igloo fork 'join sub\ a\ FORK1\ 0'
```

Duplicate the password and then immediately JOIN it into the password it was
duplicated from. JOIN it by replacing each instance of 'a' in the original
password with the duplicated password.

```
igloo fork 'join xor\ FORK1'
```

Duplicate the password and then immediately JOIN it into the password it was
duplicated from. JOIN it by xor'ing the original password with the duplicated
password. (This will zero out the password)

```
igloo fork 'join replace\ FORK1\ 1'
```

Duplicate the password and then immediately JOIN it into the password it was
duplicated from. JOIN it by replaceing the original password with the duplicate
password 1 byte after the beginning. (i.e., 'password' -> 'ppassword')

These example don't showcase the true power of JOIN statements so here's a more
complex example.

```
igloo fork "genxor 0, fork" "rcipher 6" 'join cat\ front\ FORK2' complement \
'join cat\ back\ FORK1' 'passwordify 0 \ '
```

Whoa. That's a crazy command. Let's walk through it.

Start off with a password and duplicate it.

Take FORK1 and xor it with a sequence generated by the PRNG, fork the result of
this.

Take FORK2 and rail cipher it with 6 rails then cat it to the front of FORK1.

Bitwise complement all of FORK1 then cat it to the back of FORK0.

With FORK0 genxor everything but make sure the characters end up as password
usable characters.

This could even be taken a step further using ITERATE.

```
igloo "iterate 10" \
fork "genxor 0, fork" "rcipher 6" 'join cat\ front\ FORK2' complement \
'join cat\ back\ FORK1' 'passwordify 0 \ ' \
iterate_end
```

Now this will be done 10 times. Be careful! With a password of length 5 the
obscured password was 295246 characters long. A growth rate of 5 * 3^N.
