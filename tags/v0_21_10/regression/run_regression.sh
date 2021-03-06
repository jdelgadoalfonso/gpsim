#!/bin/sh
#
# A meta-script that invokes each of the individual
# regression tests.

RT=./rt.sh

# Instruction set simulation of the mid-range devices
${RT} instructions_14bit sim

#instruction set simulation for the 16bit cores:
${RT} instructions_16bit sim

#instruction set simulation for the 12bit cores:
${RT} instructions_12bit sim

${RT} node_test sim

#${RT} p12c509 it_12bit
#${RT} p12c509 tmr0

${RT} p16f84 sim

${RT} p18f452_ports sim

#${RT} p16f628 p16f628
#${RT} p16f873 p16f873

${RT} digital_stim sim

${RT} register_stim sim

${RT} p12ce518 sim

${RT} interrupts_14bit sim

${RT} macro_test sim

${RT} logic_test sim

${RT} usart_test sim

#${RT} ccp ccp
