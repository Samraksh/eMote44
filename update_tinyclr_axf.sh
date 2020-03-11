#!/bin/bash

# VERY BASIC AND DUMB
# Must be very sure that your binary matches the current tree
# TODO: Hook into build so that you can't screw this up by invoking at the wrong time

AXF_FILE_BASE=tinyclr.axf
AXF_FILE=BuildOutput/THUMB2FP/GCC6.3/le/FLASH/debug/STM32H743NUCLEO/bin/tinyclr.axf
LOG_FILE=binaries/tinyclr.version.txt

cp $AXF_FILE binaries/
date > $LOG_FILE
arm-none-eabi-size binaries/$AXF_FILE_BASE >> $LOG_FILE
git describe --always --abbrev=0 >> $LOG_FILE
gzip -9 -f binaries/$AXF_FILE_BASE
dos2unix $LOG_FILE
