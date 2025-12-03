#ifndef PERF_COUNTERS_H
#define PERF_COUNTERS_H

#include <stdint.h>

/**
 * Performance counter data structure
 * Holds all 9 hardware performance counters from the DTEK-V board
 */
typedef struct {
    uint32_t cycles;          // mcycle - Clock cycles elapsed
    uint32_t instructions;    // minstret - Instructions retired
    uint32_t mem_instr;       // mhpmcounter3 - Memory instructions (load/store)
    uint32_t icache_miss;     // mhpmcounter4 - I-cache misses
    uint32_t dcache_miss;     // mhpmcounter5 - D-cache misses
    uint32_t icache_stall;    // mhpmcounter6 - I-cache stall cycles
    uint32_t dcache_stall;    // mhpmcounter7 - D-cache stall cycles
    uint32_t hazard_stall;    // mhpmcounter8 - Data hazard stalls
    uint32_t alu_stall;       // mhpmcounter9 - ALU stalls (division)
} perf_counters_t;


void perf_clear_counters(void);
void perf_read_counters(perf_counters_t *counters);
void perf_print_counters(perf_counters_t *counters);
void perf_print_metrics(perf_counters_t *counters);
void run_performance_test(void);

#endif /* PERF_COUNTERS_H */