# zsuck

Very simple library to convert binary files containing longs
runs of zeros into a single zero followed by a count. This
is like run length encoding (RLE), but only for zeros.

There is an encoder and a decoder, and the decoder can be
run in chunks rather than all at once. (My use case required
that for the decode but not the encode.

## example

The file `test.c` contains an example use of the encoder
and both the all-in-one-go decoder and the chunked decoder.
It also servers as a test, running many encodes and decodes
of various random byte arrays.

## command line encoder

`main.c` is a simple command-line encoder

## building

There are no dependencies, so you can do

gcc -WAll zsuck.c test.c -o test

