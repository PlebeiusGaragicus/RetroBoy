// TODO: https://forum.gamemaker.io/index.php?threads/converting-sounds-to-gameboy-quality.59663/
//https://gamefaqs.gamespot.com/boards/916375-game-boy/75562098

/*
Perfect Dark did PERFECT voices...

(?) Would I need to ensure that I have a 4MB cartridge due to the large file size?



*/

#include <gb/gb.h>

#include "util.h"
#include "audio.h"

void audio_init() {
    // This enables Sound
    // these registers must be in this specific order!

    // is 1000 0000 in binary and turns on sound
    NR52_REG = 0x80; 
    // sets the volume for both left and right channel just set to max 0x77
    NR50_REG = 0x77;
    // is 1111 1111 in binary, select which chanels we want to use in this case all of them. One bit for the L one bit for the R of all four channels
    NR51_REG = 0xFF;
}





void boop() {
    NR10_REG=0X00;
    NR11_REG=0X81;
    NR12_REG=0X43;
    NR13_REG=0X73;
    NR14_REG=0X86;
    // NR10_REG=0X00;
    // NR11_REG=0X81;
    // NR12_REG=0X43;
    // NR13_REG=0X73;
    // NR14_REG=0X86;
}

void bap() {
    NR10_REG=0x7C;
    NR11_REG=0x41;
    NR12_REG=0x73;
    NR13_REG=0x73;
    NR14_REG=0x86;
    // NR10_REG=0X00;
    // NR11_REG=0X81;
    // NR12_REG=0X43;
    // NR13_REG=0X73;
    // NR14_REG=0X86;
}


void beedledo() {
    NR10_REG=0X00;
    NR11_REG=0X81;
    NR12_REG=0X43;
    NR13_REG=0X73;
    NR14_REG=0X86;

    performantdelay(5);

    NR10_REG=0x7C;
    NR11_REG=0x41;
    NR12_REG=0x73;
    NR13_REG=0x73;
    NR14_REG=0x86;

    performantdelay(5);

    NR10_REG=0x7C;
    NR11_REG=0x41;
    NR12_REG=0x73;
    NR13_REG=0x73;
    NR14_REG=0x86;

    performantdelay(5);
}