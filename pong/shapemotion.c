/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include <p2switches.h>
#include "buzzer.h"

/**
 * 
 * Mine
 * 
 */

#define GREEN_LED BIT6

int p1Score = 0;
int p2Score = 0;
char scoreBoard[3];

AbRect net = {abRectGetBounds, abRectCheck, {55, 0.5}};
AbRectOutline paddle = {abRectOutlineGetBounds, abRectOutlineCheck, {2, 20}};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 12, screenHeight/2 - 12}
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2 + 5},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  0
};

Layer layer3 = {		/**< Layer with middle line because it looks good :) */
  (AbShape *)&net,
  {screenWidth/2 , screenHeight/2}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  &fieldLayer,
};

Layer layer2 = {		/**< Layer with user paddle */
  (AbShape *)&paddle,
  {screenWidth/2 + 45, screenHeight/2}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ROYAL_BLUE,
  &layer3,
};

Layer layer1 = {		/**< Layer with CPU paddle */
  (AbShape *)&paddle,
{screenWidth/2 - 45, screenHeight/2}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer2,
};

Layer layer0 = {		/**< Layer with the ball */
  (AbShape *)&circle4,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */


MovLayer ml2 = { &layer2, {2,0}, 0 };
MovLayer ml1 = { &layer1, {2,0}, 0 };
MovLayer ml0 = { &layer0, {2,1}, 0 }; 

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, MovLayer *p1, MovLayer *p2, Region *fence)
{
    scoreBoard[1] = '|';
	Vec2 newPos;
	u_char axis;
	Region shapeBoundary;
    
    Region paddle;
    Vec2 newPosP1;
	u_char axisP1;
	Region p1Boundary;
    
    Vec2 newPosP2;
	u_char axisP2;
	Region p2Boundary;
    
    paddle.topLeft.axes[0] = fence->topLeft.axes[0] + 7;
    paddle.topLeft.axes[1] = fence->topLeft.axes[1];
    paddle.botRight.axes[0] = fence->botRight.axes[0] - 7;
    paddle.botRight.axes[1] = fence->botRight.axes[1];
    
	for (; ml; ml = ml->next) {
        buzzer_set_period(0);
		vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
		abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
        
		vec2Add(&newPosP1, &p1->layer->posNext, &p1->velocity);
		abShapeGetBounds(p1->layer->abShape, &newPosP1, &p1Boundary);
        
        vec2Add(&newPosP2, &p2->layer->posNext, &p2->velocity);
		abShapeGetBounds(p2->layer->abShape, &newPosP2, &p2Boundary);
        
        for (axis = 0; axis < 2; axis ++){
            //added for p1
            if((shapeBoundary.topLeft.axes[axis] < paddle.topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > paddle.botRight.axes[axis])){
                if(shapeBoundary.topLeft.axes[1] > p2Boundary.topLeft.axes[1] && shapeBoundary.botRight.axes[1] < p2Boundary.botRight.axes[1] && shapeBoundary.topLeft.axes[0] > (screenWidth/2)){
                     int velocity = ml->velocity.axes[0] = -ml->velocity.axes[0];
                    buzzer_set_period(5000);
                    newPos.axes[0] += (2*velocity);
                    break;
                }
                if(shapeBoundary.topLeft.axes[1] > p1Boundary.topLeft.axes[1] && shapeBoundary.botRight.axes[1] < p1Boundary.botRight.axes[1] && shapeBoundary.topLeft.axes[0] < (screenWidth/2)){
                     int velocity = ml->velocity.axes[0] = -ml->velocity.axes[0];
                    buzzer_set_period(4000);
                    newPos.axes[0] += (2*velocity);
                    break;
                }
            }
            if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])) {
               
                int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                    buzzer_set_period(500);
                    newPos.axes[axis] += (2*velocity);
			} /**< for axis */
			/*if((ml->layer->posNext.axes[1] >= 134) && (ml->layer->posNext.axes[0] <=  orange->layer->posNext.axes[0] + 18 && ml->layer->posNext.axes[0] >= orange->layer->posNext.axes[0] - 18)) {

                int velocity = ml->velocity.axes[0] = -ml->velocity.axes[0];
                velocity = ml->velocity.axes[1] = -ml->velocity.axes[1];
                //ml->velocity.axes[0] += 1;
                newPos.axes[axis] += (2*velocity);
                //buzzer_set_period(1000);
                int redrawScreen = 1;
            }*/

            if (shapeBoundary.topLeft.axes[0] < fence->topLeft.axes[0]) {
				newPos.axes[0] = screenWidth/2;
                newPos.axes[1] = screenHeight/2;
                ml->velocity.axes[0] = 2;
                ml->layer->posNext = newPos;
                p1Score++;
                drawString5x7(3,5, "Player 1", COLOR_BLUE, COLOR_WHITE);
                //buzzer_set_period(1000);
                int redrawScreen = 1;
                break;
                
			}
			
			else if (shapeBoundary.botRight.axes[0] > fence->botRight.axes[0]) {
				newPos.axes[0] = screenWidth/2;
                newPos.axes[1] = screenHeight/2;
                ml->velocity.axes[0] = -2;
                ml->layer->posNext = newPos;
                p2Score++;
                drawString5x7(80,5, "Player 2", COLOR_BLUE, COLOR_WHITE);
                //buzzer_set_period(1000);
                int redrawScreen = 1;
                break;
			}
			
			
		} /**< for ml */
		int redrawScreen = 1;
		ml->layer->posNext = newPos;

		if ( p1Score > 9 || p2Score > 9){
                p1Score = 0;
                p2Score = 0;
        }
        scoreBoard[0] = '0' + p1Score;
        scoreBoard[2] = '0' + p2Score;
	}
	int redrawScreen = 1;
	drawString5x7(55,5, scoreBoard , COLOR_BLACK, COLOR_WHITE);
    drawString5x7(3,5, "Player 1", COLOR_RED, COLOR_WHITE);
    drawString5x7(80,5, "Player 2", COLOR_RED, COLOR_WHITE);
}

void p1Left(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,5};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void p1Right(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel ={0,-5};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void p2Left(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,5};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void p2Right(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,-5};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

u_int bgColor = COLOR_WHITE;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  buzzer_init();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);
  
  layerGetBounds(&fieldLayer, &fieldFence);
  drawString5x7(55,5, scoreBoard, COLOR_BLACK, COLOR_WHITE); //update to use variables


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml0, &layer0);
    movLayerDraw(&ml1, &layer0);
    movLayerDraw(&ml2, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    
    mlAdvance(&ml0, &ml1, &ml2, &fieldFence);
    
    if (p2sw_read() == 1)
        p1Left(&layer1);
    if (p2sw_read() == 2)
        p1Right(&layer1);
    if (p2sw_read() == 3)
        p2Left(&layer2);
    if (p2sw_read() == 4)
        p2Right(&layer2);
    
    redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
