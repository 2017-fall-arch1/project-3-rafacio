#include <msp430.h>
#include "p2switches.h"

static unsigned char switch_mask;
static unsigned char switches_last_reported;
static unsigned char switches_current;
char state1, state2, state3, state4;

static char
switch_update_interrupt_sense()
{
  char temp = P2IN;
  
  //switches_current = P2IN & switch_mask;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (switches_current);  /* if switch up, sense down */
  P2IES &= (switches_current | ~switch_mask); /* if switch down, sense up */
  return temp;
}

void 
p2sw_init(unsigned char mask)
{
  switch_mask = mask;
  P2REN |= mask;    /* enables resistors for switches */
  P2IE = mask;      /* enable interrupts from switches */
  P2OUT |= mask;    /* pull-ups for switches */
  P2DIR &= ~mask;   /* set switches' bits for input */

  switch_update_interrupt_sense();
}

/* Returns a word where:
 * the high-order byte is the buttons that have changed,
 * the low-order byte is the current state of the buttons
 */
unsigned int 
p2sw_read(){
  /*unsigned int sw_changed = switches_current ^ switches_last_reported;
  switches_last_reported = switches_current;
  return switches_current | (sw_changed << 8);*/
  char temp = switch_update_interrupt_sense();
  char state1 = (temp & SW1) ? 0 : 1;
  char state2 = (temp & SW2) ? 0 : 1;
  char state3 = (temp & SW3) ? 0 : 1;
  char state4 = (temp & SW4) ? 0 : 1;
  if(state1 == 1){return 1;}
  else if(state2 == 1){return 2;}
  else if(state3 == 1){return 3;}
  else if(state4 == 1){return 4;}
  return 0;
    
}

/* Switch on P2 (S1) */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & switch_mask) {  /* did a button cause this interrupt? */
    P2IFG &= ~switch_mask;	/* clear pending sw interrupts */
    switch_update_interrupt_sense();
  }
}
