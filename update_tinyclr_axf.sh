#!/bin/bash

# VERY BASIC AND DUMB
# Must be very sure that your binary matches the current tree
# TODO: Hook into build so that you can't screw this up by invoking at the wrong time

AXF_FILE=BuildOutput/THUMB2FP/GCC6.3/le/FLASH/debug/STM32H743NUCLEO/bin/tinyclr.axf
LOG_FILE=binaries/tinyclr.version.txt

cp $AXF_FILE binaries/
date > $LOG_FILE
arm-none-eabi-size $AXF_FILE >> $LOG_FILE
git describe --always --abbrev=0 >> $LOG_FILE
dos2unix $LOG_FILE