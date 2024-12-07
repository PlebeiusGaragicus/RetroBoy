#include <rand.h>

#include <gb/gb.h>
// #include <gb/drawing.h>

#include "sprites.h"



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