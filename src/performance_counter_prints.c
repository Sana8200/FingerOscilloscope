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

    uint32_t cycles_div100 = clk_cycles / 100;
    uint32_t instr_div100 = instructions / 100;
    uint32_t mem_div100 = mem_instr / 100;

    print("CPI x100:              ");
    if (instr_div100 > 0){
        print_dec(clk_cycles / instr_div100);
    } else{
        print_dec(0);
    }

    print("IPC x100:              ");
    if (cycles_div100 > 0){
        print_dec(instructions / cycles_div100);
    } else{
        print_dec(0);
    }

    print("D-Cache Miss Ratio %:  ");
    if (mem_div100 > 0){
        print_dec(d_cache_miss / mem_div100);
    } else{
        print_dec(0);
    }

    print("I-Cache Miss Ratio %:  ");
    if (instr_div100 > 0){
        print_dec(i_cache_miss / instr_div100);
    } else{
        print_dec(0);
    }

    print("I-Cache Stall %:       ");
    if (cycles_div100 > 0){
        print_dec(i_cache_stall / cycles_div100);
    } else{
        print_dec(0);
    }

    print("D-Cache Stall %:       ");
    if (cycles_div100 > 0){
        print_dec(d_cache_stall / cycles_div100);
    } else{
        print_dec(0);
    }

    print("Hazard Stall %:        ");
    if (cycles_div100 > 0){
        print_dec(data_hazard_stall / cycles_div100);
    } else{
        print_dec(0);
    }

    print("ALU Stall %:           ");
    if (cycles_div100 > 0){
        print_dec(alu_stall / cycles_div100);
    } else{
        print_dec(0);
    }

    print("Memory Intensity %:    ");
    if (instr_div100 > 0){
        print_dec(mem_instr / instr_div100);
    } else{
        print_dec(0);
    }

    print("Total Cache Misses:    ");
    print_dec(i_cache_miss + d_cache_miss);
}