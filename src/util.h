#ifndef UTIL_H
#define UTIL_H


#include <rand.h>


int random(int min, int max);
void performantdelay(uint8_t numloops);
// int16_t abs(int16_t x);
#define abs(x) ( (x) > 0 ? (x) : -(x) )


#endif