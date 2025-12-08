/**
 * DTEK-V Hardware Performance Counters - Print Function
 */

#include "performance_counter.h"
#include "dtekv-lib.h"


void print_counters(void) {

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
    
    print("Execution Time (s):  "); 
    print_dec(clk_cycles / 30000000);
  
    
    print("CPI:    "); 
    print_dec(clk_cycles / instructions);
    

    print("IPC:    "); 
    print_dec(instructions / clk_cycles);

    print("D-Cache Miss Ratio:  ");
    print_dec(d_cache_miss / mem_instr);

    print("D-Cache Hit Ratio %:   "); 
    print_dec( 1 - ( d_cache_miss / mem_instr));


    print("I-Cache Miss Ratio:  ");
    print_dec(i_cache_miss / instructions);

    print("I-Cache Hit Ratio %:   "); 
    print_dec(1 - (i_cache_miss / instructions));
    
   
    print("ALU Stall Ratio %:     "); 
    print_dec(alu_stall / clk_cycles);

    print("Memory Intensity %: "); 
    print_dec(mem_instr / instructions);
    
    print("Hazard Stall Ratio %:   ");
    print_dec(data_hazard_stall / clk_cycles);

    print("Cache Misses:   ");
    print_dec(i_cache_miss + d_cache_miss);
}