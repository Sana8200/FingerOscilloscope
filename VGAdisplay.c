
#include <stdint.h>
#include <stdio.h>


int main(){
    volatile char *VGA = (volatile char*) 0x08000000;

    for(int i=0; i < 320*480; i++){
        VGA[i] = i / 320;
    }

    unsigned int y_offset = 0;

    volatile int *VGA_CTRL = (volatile int*) 0x04000100;

    while(1){
        *(VGA_CTRL) = (unsigned int)(VGA + y_offset * 320);

        *(VGA_CTRL + 0) = 0;

        y_offset = (y_offset + 1) % 240;

        for(int i = 0; i < 100000; i++){
            asm volatile("nop");
        }
    }
}

// VGA_init, parameter : uint height, unit width, unit32 color 
