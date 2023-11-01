#!/bin/sh

gcc -O2 -Wall -Werror -o gen50 gen50.c -lgpiod -lrt
gcc -O2 -Wall -Werror -o cnt50 cnt50.c -lgpiod
