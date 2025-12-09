#ifndef PERF_COUNTERS_H
#define PERF_COUNTERS_H

#include <stdint.h>

extern uint32_t clk_cycles;          // mcycle - clock cycles
extern uint32_t instructions;        // minstret - instructions retired
extern uint32_t mem_instr;           // mhpmcounter3 - memory instructions
extern uint32_t i_cache_miss;        // mhpmcounter4 - I-cache misses
extern uint32_t d_cache_miss;        // mhpmcounter5 - D-cache misses
extern uint32_t i_cache_stall;       // mhpmcounter6 - I-cache stall cycles
extern uint32_t d_cache_stall;       // mhpmcounter7 - D-cache stall cycles
extern uint32_t data_hazard_stall;   // mhpmcounter8 - data hazard stalls
extern uint32_t alu_stall;           // mhpmcounter9 - ALU stalls

extern void clear_counters(void);
extern void read_counters(void);

void print_counters(void);

#endif