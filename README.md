# Test program for invalid characters in file systems

This repository contains a simple directory name generator to verify which file names are valid and can be parsed by the file system.

## How to build

<pre>
make all
./invalidchar
</pre>

## How to verify

<pre>
find testdir | wc -l

# The result should be 369372
</pre>


