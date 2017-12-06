#include <msp430.h>
#include "buzzer.h"
#include "p2switches.h"

void decisecond() 
{
  static char cnt = 0;		/* # deciseconds/frequecy change */
  if (++cnt > 2) {		
    cnt = 0;
  }
}

void
__interrupt_vec(WDT_VECTOR) WDT(){	/* 250 interrupts/sec */
  static char second_count = 0, decisecond_count = 0;
  if (++decisecond_count == 25) {
    buzzer_advance_frequency();
  }
  state_update();
} 
