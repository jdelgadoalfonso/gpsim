#!/bin/sh
#
# A meta-script that invokes each of the individual
# regression tests.

if [ $# -gt 1 ]
then
  echo "Usage: `basename $0` [ <gpsim_path> ] "
  exit 0
fi

if [ $# -gt 0 ]
then
  GPSIM_PATH=$1
  export GPSIM_PATH
fi

RT=./rt.sh

# Basic breakpoint test
${RT} breakpoints sim

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

#${RT} p16f873 sim

${RT} digital_stim sim

${RT} register_stim sim

${RT} p12ce518 sim

${RT} eeprom_wide sim

${RT} interrupts_14bit sim

${RT} macro_test sim

${RT} logic_test sim

${RT} resistor sim

${RT} usart_test sim_pir1v1

${RT} usart_test sim_pir1v2

${RT} txisr_test sim

${RT} tmr0_16bit sim

${RT} switch_test sim

${RT} p18f sim

${RT} comparator sim_628

${RT} comparator sim_877a

${RT} a2d sim_71

${RT} a2d sim_871

${RT} a2d sim_873a

${RT} a2d sim_874a

${RT} a2d sim_88

${RT} a2d sim_452

${RT} psp sim_452

${RT} psp sim_871

${RT} ttl sim_377

${RT} ccp sim_877a

${RT} ccp sim_pwm877a

${RT} wavegen sim

${RT} spi sim_88

${RT} spi sim_242

${RT} spi sim_c62

${RT} i2c sim_88

${RT} i2c sim_876

${RT} port_stim sim

