# RvRun - A RISC-V emulator

> RvRun is an emulator for the Linux RISC-V 64bit userspace, it is an educational
> project, that attempts to simulate as many instructions and system calls as possible.


## Currently supported:
### Extensions
 * Rv64I (Base ISA)
### System calls
 * openat       (syscall num: 56)
 * close        (syscall num: 57)
 * read         (syscall num: 63)
 * write        (syscall num: 64)
 * exit         (syscall num: 93)
 * exit_group   (syscall num: 94)


## Building

Ensure you have `git`, `make`, and `gcc` installed, then, clone the repo and run:
```bash
$ git clone https://github.com/foo8664/RvRun.git
$ cd RvRun
$ make
```
After that, you should find an `rvrun` executable in your current directory.
The Makefile currently does not have an `install` target, as RvRun is not yet
ready to be used properly. One will be added before the first release.

You can clean object and dependency files with `make clean`, it will also remove
the executable, tho, so copy/move it to somewhere before that.


## Running RISC-V files

You'll need an ELF file for RISC-V 64bit Linux, if you don't have one, install
the [GNU RISC-V toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) and compile an assembly file with -march=rv64i, like below:
```bash
$ riscv64-unknown-linux-gnu-as -march=rv64i example.asm -o example.o
$ riscv64-unknown-linux-gnu-ld example.o -o example
$ ./rvrun example
Hello World
```

RvRun supports several flags, you can find documentation for them by running
`./rvrun --help`. The `--stdin`, `--stdout`, and `--stderr` flags currently
truncate all files that they open, being the equivalent of `<`, `>` and `2>`
on bash, but for the emulated process. An appending option will be added later.

Running C compiled code is not yet póssible, but if you want to try, make sure
to add the `-nostdlib` and `-static` flags to the compiler.
