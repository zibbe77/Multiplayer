#!/bin/bash
if cc program.c $(pkg-config --libs --cflags raylib) -pthread -o run -O2
then
    echo "Compiled"
    ./run
fi