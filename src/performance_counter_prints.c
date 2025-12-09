/**
 * DTEK-V Hardware Performance Counters - Print Functions
 */

#include "performance_counter.h"
#include "dtekv-lib.h"


void print_counters(void) {

    read_counters();

    print("\n========== Raw Counter Values ==========\n");
    print("Cycles:        "); print_dec(clk_cycles);
    print("Instructions:  "); print_dec(instructions);
    print("Mem Instr:     "); print_dec(mem_instr);
    print("I-Cache Miss:  "); print_dec(i_cache_miss);
    print("D-Cache Miss:  "); print_dec(d_cache_miss);
    print("I-Cache Stall: "); print_dec(i_cache_stall);
    print("D-Cache Stall: "); print_dec(d_cache_stall);
    print("Hazard Stall:  "); print_dec(data_hazard_stall);
    print("ALU Stall:     "); print_dec(alu_stall);
    


    print("\n========== Derived Metrics ==========\n");
    
    // Execution time (s) = mcycle / 30,000,000
    print("Execution Time (s):  "); 
    print_dec(clk_cycles / 30000000);
  
    
    // CPI = mcycle / minstret
    print("CPI x100:              "); 
    if (instructions > 0) {
        print_dec((clk_cycles * 100) / instructions);
    } else {
        print_dec(0);
    }
    
    
    // IPC = minstret / mcycle
    print("IPC x100:              "); 
    if (clk_cycles > 0) {
        print_dec((instructions * 100) / clk_cycles);
    } else {
        print_dec(0);
    }


    // D-cache miss ratio = mhpmcounter5 / mhpmcounter3
    print("D-Cache Miss Ratio %:  "); 
    if (mem_instr > 0) {
        print_dec((d_cache_miss * 100) / mem_instr);
    } else {
        print_dec(0);
    }


    // D-cache hit ratio = 1 - (mhpmcounter5 / mhpmcounter3)
    print("D-Cache Hit Ratio %:   "); 
    if (mem_instr > 0) {
        print_dec(100 - ((d_cache_miss * 100) / mem_instr));
    } else {
        print_dec(100);
    }


    // I-cache miss ratio = mhpmcounter4 / minstret
    print("I-Cache Miss Ratio %:  "); 
    if (instructions > 0) {
        print_dec((i_cache_miss * 100) / instructions);
    } else {
        print_dec(0);
    }


    // I-cache hit ratio = 1 - (mhpmcounter4 / minstret)
    print("I-Cache Hit Ratio %:   "); 
    if (instructions > 0) {
        print_dec(100 - ((i_cache_miss * 100) / instructions));
    } else {
        print_dec(100);
    }
    

    // ALU-stall ratio = mhpmcounter9 / mcycle
    print("ALU Stall Ratio %:     "); 
    if (clk_cycles > 0) {
        print_dec((alu_stall * 100) / clk_cycles);
    } else {
        print_dec(0);
    }


    // Memory Intensity = mhpmcounter3 / minstret
    print("Memory Intensity %:    "); 
    if (instructions > 0) {
        print_dec((mem_instr * 100) / instructions);
    } else {
        print_dec(0);
    }
    

    // Hazard-stall ratio = mhpmcounter8 / mcycle
    print("Hazard Stall Ratio %:  "); 
    if (clk_cycles > 0) {
        print_dec((data_hazard_stall * 100) / clk_cycles);
    } else {
        print_dec(0);
    }


    // I-Cache stall ratio = mhpmcounter6 / mcycle */
    print("I-Cache Stall Ratio %: "); 
    if (clk_cycles > 0) {
        print_dec((i_cache_stall * 100) / clk_cycles);
    } else {
        print_dec(0);
    }


    // D-Cache stall ratio = mhpmcounter7 / mcycle */
    print("D-Cache Stall Ratio %: "); 
    if (clk_cycles > 0) {
        print_dec((d_cache_stall * 100) / clk_cycles);
    } else {
        print_dec(0);
    }


    // Cache misses = mhpmcounter4 + mhpmcounter5 */
    print("Total Cache Misses:    "); 
    print_dec(i_cache_miss + d_cache_miss);
}