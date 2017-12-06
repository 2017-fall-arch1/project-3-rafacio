#include <msp430.h>
#include "states.h"
#include "p2switches.h"
#include <stdio.h>
char state0, state1, state2, state3, state4; /* effectively boolean */

static char switch_update_interrupt_sense() {
  char p1val = P2IN;
  P2IES |= (p1val & SWITCHES);
  P2IES &= (p1val | ~SWITCHES);
  return p1val; 
}

void switch_init() {
  P2REN |= SWITCHES;
  P2IE = SWITCHES;
  P2OUT |= SWITCHES;
  P2DIR &= ~SWITCHES;
  switch_update_interrupt_sense();
  switch_interrupt_handler();
}

void switch_interrupt_handler() {
  char p1val = switch_update_interrupt_sense();
  state0 = (p1val & SW1) ? 0 : 1;
  state1 = (p1val & SW2) ? 0 : 1;
  state2 = (p1val & SW3) ? 0 : 1;
  state3 = (p1val & SW4) ? 0 : 1;

  state_update();
} 
