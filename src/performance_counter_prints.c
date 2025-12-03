#include "performance_counter.h"
#include "dtekv-lib.h"

// Print all raw counter values

void perf_print_counters(perf_counters_t *counters) {
    print("\n===== Performance Counters =====\n");
    
    print("Cycles:        ");
    print_dec(counters->cycles);
    
    print("Instructions:  ");
    print_dec(counters->instructions);
    
    print("Mem Instr:     ");
    print_dec(counters->mem_instr);
    
    print("I-Cache Miss:  ");
    print_dec(counters->icache_miss);
    
    print("D-Cache Miss:  ");
    print_dec(counters->dcache_miss);
    
    print("I-Cache Stall: ");
    print_dec(counters->icache_stall);
    
    print("D-Cache Stall: ");
    print_dec(counters->dcache_stall);
    
    print("Hazard Stall:  ");
    print_dec(counters->hazard_stall);
    
    print("ALU Stall:     ");
    print_dec(counters->alu_stall);
    
    print("================================\n");
}


// Print derived metrics

void perf_print_metrics(perf_counters_t *c) {
    print("\n===== Derived Metrics =====\n");
    
    // Execution time in ms: cycles / 30000 (since 30 MHz clock)
    uint32_t exec_time_ms = c->cycles / 30000;
    print("Exec Time (ms): ");
    print_dec(exec_time_ms);
    
    // IPC * 100 (e.g., 73 means IPC = 0.73)
    uint32_t ipc_x100 = 0;
    if (c->cycles > 0) {
        ipc_x100 = (c->instructions * 100) / c->cycles;
    }
    print("IPC x100:       ");
    print_dec(ipc_x100);
    
    // D-Cache hit rate * 100
    uint32_t dcache_hit_x100 = 100;
    if (c->mem_instr > 0) {
        uint32_t miss_rate = (c->dcache_miss * 100) / c->mem_instr;
        dcache_hit_x100 = 100 - miss_rate;
    }
    print("D-Cache Hit %:  ");
    print_dec(dcache_hit_x100);
    
    // I-Cache hit rate * 100
    uint32_t icache_hit_x100 = 100;
    if (c->instructions > 0) {
        uint32_t miss_rate = (c->icache_miss * 100) / c->instructions;
        icache_hit_x100 = 100 - miss_rate;
    }
    print("I-Cache Hit %:  ");
    print_dec(icache_hit_x100);
    
    // D-Cache stall percentage
    uint32_t dcache_stall_pct = 0;
    if (c->cycles > 0) {
        dcache_stall_pct = (c->dcache_stall * 100) / c->cycles;
    }
    print("D-Cache Stall %: ");
    print_dec(dcache_stall_pct);
    
    // Hazard stall percentage
    uint32_t hazard_stall_pct = 0;
    if (c->cycles > 0) {
        hazard_stall_pct = (c->hazard_stall * 100) / c->cycles;
    }
    print("Hazard Stall %: ");
    print_dec(hazard_stall_pct);
    
    // ALU stall percentage
    uint32_t alu_stall_pct = 0;
    if (c->cycles > 0) {
        alu_stall_pct = (c->alu_stall * 100) / c->cycles;
    }
    print("ALU Stall %:    ");
    print_dec(alu_stall_pct);
    
    // Memory intensity
    uint32_t mem_intensity = 0;
    if (c->instructions > 0) {
        mem_intensity = (c->mem_instr * 100) / c->instructions;
    }
    print("Mem Intensity %: ");
    print_dec(mem_intensity);
    
    print("===========================\n");
}