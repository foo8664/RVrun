#!/usr/bin/env bash

for f in $(find . -type f -and -name '*.asm' -and -not -name 'common.asm'); do
	gmake -B $(echo "$f" | sed 's/.asm$/.elf/')
done
