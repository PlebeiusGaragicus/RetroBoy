#include <rand.h>
#include <gb/gb.h>

#include "sprites.h"




// const BOOLEAN TESTING = TRUE;




int random(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}

void performantdelay(uint8_t numloops)
{
    uint8_t i;
    for(i = 0; i < numloops; i++)
    {
        wait_vbl_done();
    }
}

int16_t reduce_velocity(int16_t vel) {
    return (vel >> 2);  // Multiply by 0.5
}


// uint16_t abs(uint16_t x) { return x > 0 ? x : -x; }
