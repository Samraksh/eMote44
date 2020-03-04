#!/bin/bash

# VERY BASIC AND DUMB
# Must be very sure that your binary matches the current tree
# TODO: Hook into build so that you can't screw this up by invoking at the wrong time

LOG_FILE=binaries/tinyclr.version.txt

cp BuildOutput/THUMB2FP/GCC6.3/le/FLASH/debug/STM32H743NUCLEO/bin/tinyclr.axf binaries/
echo "BuildOutput/THUMB2FP/GCC6.3/le/FLASH/debug/STM32H743NUCLEO/bin/tinyclr.axf" > $LOG_FILE
date >> $LOG_FILE
git describe --always --abbrev=0 >> $LOG_FILE
