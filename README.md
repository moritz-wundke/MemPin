## MemPin

A pin tool for memory and instruction optimization for Linux. This pin tool is MPI ready and will create an .csv file for each thread.

For more information about pin tools visit its [home page](http://software.intel.com/en-us/articles/pintool).


# Building

To build the pintool just execute the make file:

    $ ./make all

To rebuild the project use:

    $ ./make clean

The output directory can be different from 64 and 32 bit systems.

# Usage

MemPin comes with 4 predefined tools:

 * 1: Basic instruction counting (per thread)
 * 2: Extended instruction counting (per thread)
 * 3: Procedure analysis (global)
 * 4: Malloc analysis (per thread)

The pid will be appended to the output file so that is is prepared
for environments such as MPI.

The following example will analyse the common programm 'ls' and store the output in MemPin.csv

    pin -t mempin.so -o MemPin.csv -tool 1 -- /bin/ls

# License

BSD Licencse - Copyright (c) 2012, Moritz Wundke