# igloo
A password obfuscation manager

![igloo 'logo'](./igloo.png)

You might be asking what that is...

As the name implies igloo is a tool to obscure passwords.

The user can set a sequence of hash algorithms and character permutations to
alter an input password. This sequence of hashing and permuting is called a
profile.

By providing an input password and a profile the user is able to generate an
obscured output password. Ideally, the obscured password prevents various
unfortunate events from revealing the input password, such as:

* Companies selling your data
* A data breach which reveals your password or its hash
* Insecure password communication protocols

Since the output password is not intended to be recorded the profile should be
used to recompute it as needed.

## Process

An input password and profile are provided to the igloo script. After this three
things will happen:

1. The input will be hashed.
2. The hashed input will be befuddled.
3. The befuddled input will be hashed again.

Then the hashed-befuddled-hashed output will be printed. Afterwards, any
variables with sensitive information will be cleared.

## Password Philosophy

By way of its construction igloo changes the assumptions about your password.
Because the output cannot be generated without the input, and because the input
is mostly useless without the profile, it becomes more important to remember the
profile than it does the input password.

Perhaps more interesting though is that an input password can now contain
special characters and it may be as long or short as wanted.

## Profiles

Profiles are by default stored relative to igloo.sh in ./.profile/ and can be
generated with the -o,--output flag or written manually.

### Hash Section

A profile begins with the 'Hash:' section. This section is intended to contain
bash commands which accept input from stdin and output to stdout. As an example,
here is a hash section that would run sha256 and then BLAKE2:

```
Hash:
sha256sum -a 256
b2sum -l 256
```

Techincally, only hash functions should be used here but any command that fits
can be used too.

Every command in the Hash section will be run in sequence any time the hash
functions are iterated over.

### Befuddle Section

The Bufuddle section is identified by "Bufuddle:". This section must follow the
Hash section.

Unlike the Hash section arbitrary commands cannot be inserted here there are,
however, a variety of permutation methods provided by igloo. Use the
`--help befuddle` flag to see the available commands.

```
Befuddle:
cat front ab23\xFF+=
```

The above Befuddle section example shows one permutation command, 'cat'. This
will concatenate the following string to the beginning of the hashed input
password.

Notice the escape sequence, this is covered in the next section.

## Escape Sequences

As mentioned in Password Philosophy and the previous section, input passwords
may contain special characters. One way to do this is with escape sequences.
These follow the standard ANSI escape notation. Therefore \x~~ can be used to
specify any character in the range 0 - 255 using hexadecimal. \n and others may
be used as well.

Escape Sequences may be provided on the command line and within a profile.
