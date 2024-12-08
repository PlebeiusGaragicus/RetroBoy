
/*

// https://github.com/gbdk-2020/gbdk-2020/issues/219
// Refer to GBDK manual which contains the below snippet
fixed player[2];
...
// Modify player position using its 16 bit representation
player[0].w += player_speed_x;
player[1].w += player_speed_y;
...
// Use only the upper 8 bits for setting the sprite position
move_sprite(0, player[0].h ,player[1].h);

*/


/*
20.72.5.10 sys_time volatile uint16_t sys_time [extern]
Global Time Counter in VBL periods (60Hz)
Increments once per Frame
Will wrap around every∼18 minutes (unsigned 16 bits = 65535 / 60 / 60 = 18.2)
*/
