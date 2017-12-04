/** \file lcddemo.c
 *  \brief A simple demo that draws a string and circle
 */

#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>


/** Initializes everything, clears the screen, draws "hello" and the circle */
void main()
{
  configureClocks();
  lcd_init();
  p2sw_init(15);
  or_sr(0x8);			/* GIE (enable interrupts) */
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_BLUE);
  
    int x = 20;
    int y = 40;
  drawString5x7(10,10, "switches:", COLOR_GREEN, COLOR_BLUE);
  while (1) {
    u_int switches = p2sw_read(), i;
    char str[5];
    for (i = 0; i < 4; i++){
        if(i == 0){
            str[i] = (switches & (1<<i)) ? '-' : '0'+i;
            if(x > 0 && (switches & 1)){
                x -= 10;
            }
        }
        if(i == 1){
            str[i] = (switches & (1<<i)) ? '-' : '1'+i;
            if(x > screenWidth && (switches & 2)){
                x += 10;
            }
        }
        if(i == 2){
            str[i] = (switches & (1<<i)) ? '-' : '2'+i;
            if(y > 0 && (switches & 4)){
                y += 10;
            }
        }
        if(i == 3){
            str[i] = (switches & (1<<i)) ? '-' : '3'+i;
            if(y > screenHeight && (switches & 8)){
                y -= 10;
            }
        }
    }
    
    str[4] = 0;
    clearScreen(COLOR_BLUE);
    drawString5x7(x,y, str, COLOR_GREEN, COLOR_BLUE);
  } 
}
