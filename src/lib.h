/* lib.h
 * This header file declares all functions written before for the utilities 
 */

 
#ifndef LIB_H
#define LIB_H

// These functions are globally visible in the assembly files 

extern void display_string(char*);
extern void time2string(char*, int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);

#endif /* LIB_H */
