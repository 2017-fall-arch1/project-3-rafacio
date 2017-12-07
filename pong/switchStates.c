#include <p2switches.h>
#include "switchStates.h"

int switchStates(){
    if (p2sw_read() == 1){
        return 1;
    }
    else if (p2sw_read() == 2){
        return 2;
    }
    else if (p2sw_read() == 3){
        return 3;
    }
    else if (p2sw_read() == 4){
        return 4;
    }
    return 0;
} 
