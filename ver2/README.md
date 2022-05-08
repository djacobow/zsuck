# rlez

Very simple library to convert binary files containing longs
runs of zeros into an escape byte (0xaa, in this case) followed
by either a nonzero byte indicating the number of zeros in the
original buffer, or 0, indicating the escape byte was in the
original file.

For example,

010203aaaa04050600000000000000

would become

010203aa00aa00040506aa06

# python

Python routines that do the same thing are also included.

## building

There are no dependencies, so you can do

gcc -WAll rlez.c test.c -o test

