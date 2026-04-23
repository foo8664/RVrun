# Example programs

## Compiling

You will need a cross compiler for the RISC-V GNU toolchain for this, pre-compiled
versions should be available on releases, tho.

To compile these programs, run one of the following commands:

```bash
$ make all
```
To compile all programs

```bash
$ make <program>.elf
```
To compile the <program> program

```bash
$ make clean
```
To delete all of the object files and binaries


## Current examples
floating point inputs MUST have a decimal part (e.g. 1 should be written as 1.0)
 * echo (takes an indefinite amount of arguments, outputs them separated by a space)
 * cat  (takes a filename as an argument, outputs it's contents)
 * xxd  (takes a filename as an argument, outputs it's contents in hexadecimal)
 * fib  (takes a natural number n as input, outputs the n-th fibonacci number)
 * gcd  (takes two natural numbers as input, outputs their greatest common divisor)
 * sin  (takes a floating point input in radians, outputs it's sine)

Some of these programs are recreations of famous UNIX utilities (echo, cat, and xxd).
They do not take any flags, but Linux/UNIX system's man pages might be useful in providing
a greater description of their expected behaviour
