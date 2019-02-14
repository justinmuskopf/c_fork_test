# c_fork_test
A simple program that tests the functionality of the fork() command in C by spawning three processes all writing to the same file.

## Build Instructions
1. execute `gcc fork_sequence_system.c`
2. execute `./a.out [SEQUENCE_FILENAME]`
    - where SEQUENCE_FILENAME is an optional filename of a file that contains a single number.
