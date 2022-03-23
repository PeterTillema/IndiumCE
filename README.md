# IndiumCE - a fast TI-BASIC interpreter
This is the repository for IndiumCE, a fast TI-BASIC interpreter. The existing
BASIC interpreter in the OS is very slow, which makes the language itself pretty
slow too. This has a few reasons:
- Floats are stored in custom TI format, rather than IEEE-floats, which makes
  operations on it slow.
- It is interpreted on the flow, which means it has to parse the entire syntax
  over and over again.
- `Goto`'s are awfully slow, as it scans the entire program until it found the
  corresponding label.

IndiumCE approaches the BASIC language entirely different. It first scans the
program and makes an [Abstract syntax tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree)
out of it. This means that syntax checking is done only once. Besides that,
floats are stored in proper IEEE754 format, which means operations are pretty
fast. Variables are stored at a fixed* memory address, no need to look it up
from the VAT every time. Numbers are preparsed, which saves speed as well.

_* Not entirely true, as space is allocated once the variable is alive, but
after then it's fixed._

## Comparison
I ran the examples from [these benchmarking tests](https://www.cemetech.net/forum/viewtopic.php?t=14085)
by TheLastMillennial. Note that not all benchmarks are tested, as some are very
fast, which may cause IndiumCE to be slower, due to the overhead at the start.

// todo

## Compiling
To build this repository from source, the [CE C Toolchain](https://github.com/CE-Programming/toolchain)
is required. After installing the toolchain, clone this repository and run
`make` to compile. The output .8xp can be found in the `bin/` directory. Note that
you also need the transfer both .8xv files from the `src/font/` directory to your
calculator.

## Credits
Thanks RoccoloxPrograms for making the homescreen font usable by fontlibc! You
find the fonts [on Cemetech](https://www.cemetech.net/downloads/files/2143/x2531).