#ifndef DTEKV_LIB_H
#define DTEKV_LIB_H

void printc(char );
void print(char *);
void print_dec(unsigned int);
void print_hex32 ( unsigned int);
// void handle_exception (unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5, unsigned mcause, unsigned syscall_num );
// int nextprime( int inval );


// These functions are globally visible in the assembly files 
extern void display_string(char*);
//extern void delay(int);
//extern void time2string(char*, int);
//extern void tick(int*);

#endif /* DTEKV_LIB_H */








