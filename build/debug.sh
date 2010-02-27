#!/bin/bash

make
cd ../bin
gdb --args ./eqOgre --eq-config 2-window.eqc
