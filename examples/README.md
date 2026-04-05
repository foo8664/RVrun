# Example programs

> These programs are meant to be used as examples for the emulator. The .elf files
> are purposefully left so that one does not need a cross compiler just to test it.

The programs are simpler recreations of classical UNIX utilities.


## Compiling them

While there is no need to compile them in your machine (the binaries are already there),
you can do so by running:
```bash
$ make all
```
To compile all programs

```bash
$ make $FILE.elf
```
To compile the $FILE program

```bash
$ make clean
```
To delete all of the object files and binaries


## Current examples
 * echo
 * cat
 * xxd

These programs should be available at any modern linux system, with man pages!
The current examples cannot take any flags to modify behaviour, but they do take arguments
