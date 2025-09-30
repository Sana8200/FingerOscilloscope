

void sevenSegDisplay(void){
    volatile int *segm7 = (volatile int *) 0x04000050;

    *segm7 = 0x19;
    
}


