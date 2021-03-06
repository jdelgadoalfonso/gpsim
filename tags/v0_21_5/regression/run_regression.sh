#!/bin/sh
#
# A meta-script that invokes each of the individual
# regression tests.

RT=./rt.sh
# Instruction set simulation of the mid-range devices

${RT} instructions_14bit instructions_14bit

#instruction set simulation for the 16bit cores:
${RT} instructions_16bit instructions_16bit

#instruction set simulation for the 12bit cores:
${RT} instructions_12bit instructions_12bit

${RT} node_test node_test

#${RT} p12c509 it_12bit
#${RT} p12c509 tmr0

${RT} p16f84 p16f84

#${RT} p16f628 p16f628

#${RT} p16f873 p16f873

${RT} digital_stim digital_stim

${RT} register_stim register_stim

${RT} p12ce518 p12ce518

${RT} interrupts_14bit interrupts_14bit

#${RT} ccp ccp
