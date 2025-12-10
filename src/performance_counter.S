# DTEK-V hardware performance counter access
# Using CSR registers to read/write perfromance counters (we want to read from these hardware counters)

.data

.global  clk_cycles, instructions, mem_instr, i_cache_miss, d_cache_miss, i_cache_stall, d_cache_stall, data_hazard_stall, alu_stall
clk_cycles:         .word 0
instructions:       .word 0
mem_instr:          .word 0
i_cache_miss:       .word 0
d_cache_miss:       .word 0
i_cache_stall:      .word 0
d_cache_stall:      .word 0
data_hazard_stall:  .word 0
alu_stall:          .word 0



.text 
.align 2
.global clear_counters, read_counters


# mcycle - clock cycles - clk_cycles
# minstret - instructions retired - instructions
# mhpmcounter3 - memory instructions - mem_instr
# mhpmcounter4 - I-cache misses - i_cache_miss
# mhpmcounter5 - D-cache misses - d_cache_miss
# mhpmcounter6 - I-cache stall cycles - i_cache_stall
# mhpmcounter7 - D-cache stall cycles - d_cache_stall
# mhpmcounter8 - data hazard stalls - data_hazard_stall
# mhpmcounter9 - ALU stalls - alu_stall



clear_counters:
    csrw mcycle, x0
    csrw minstret, x0
    csrw mhpmcounter3, x0
    csrw mhpmcounter4, x0
    csrw mhpmcounter5, x0
    csrw mhpmcounter6, x0
    csrw mhpmcounter7, x0
    csrw mhpmcounter8, x0
    csrw mhpmcounter9, x0
    ret


read_counters:
    # mcycle -> clk_cycles
    csrr t0, mcycle
    la t1, clk_cycles
    sw t0, 0(t1)
    
    # minstret -> instructions
    csrr t0, minstret
    la t1, instructions
    sw t0, 0(t1)
    
    # mhpmcounter3 -> mem_instr
    csrr t0, mhpmcounter3
    la t1, mem_instr
    sw t0, 0(t1)
    
    # mhpmcounter4 -> i_cache_miss
    csrr t0, mhpmcounter4
    la t1, i_cache_miss
    sw t0, 0(t1)
    
    # mhpmcounter5 -> d_cache_miss
    csrr t0, mhpmcounter5
    la t1, d_cache_miss
    sw t0, 0(t1)
    
    # mhpmcounter6 -> i_cache_stall
    csrr t0, mhpmcounter6
    la t1, i_cache_stall
    sw t0, 0(t1)
    
    # mhpmcounter7 -> d_cache_stall
    csrr t0, mhpmcounter7
    la t1, d_cache_stall
    sw t0, 0(t1)
    
    # mhpmcounter8 -> data_hazard_stall
    csrr t0, mhpmcounter8
    la t1, data_hazard_stall
    sw t0, 0(t1)
    
    # mhpmcounter9 -> alu_stall
    csrr t0, mhpmcounter9
    la t1, alu_stall
    sw t0, 0(t1)
    
    ret
