# assembly implementation of performance counter functions for DTEK-V
# Uses RISC-V CSR instructions to access hardware performance counters

.text
.align 2
.globl perf_clear_counters, perf_read_counters

# perf_clear_counters (no arguments, no returns): Clears all 9 performance counters to zero
perf_clear_counters:
    csrw mcycle, zero
    csrw minstret, zero
    csrw mhpmcounter3, zero
    csrw mhpmcounter4, zero
    csrw mhpmcounter5, zero
    csrw mhpmcounter6, zero
    csrw mhpmcounter7, zero
    csrw mhpmcounter8, zero
    csrw mhpmcounter9, zero
    ret


# =============================================================================
# perf_read_counters (no returns, values stored in struct) : Reads all 9 performance counters into a struct
# Arguments: a0 = pointer to perf_counters_t struct
# Struct layout (each field is 4 bytes):
#   offset 0:  cycles (mcycle)
#   offset 4:  instructions (minstret)
#   offset 8:  mem_instr (mhpmcounter3)
#   offset 12: icache_miss (mhpmcounter4)
#   offset 16: dcache_miss (mhpmcounter5)
#   offset 20: icache_stall (mhpmcounter6)
#   offset 24: dcache_stall (mhpmcounter7)
#   offset 28: hazard_stall (mhpmcounter8)
#   offset 32: alu_stall (mhpmcounter9)
# =============================================================================
perf_read_counters:
    csrr t0, mcycle
    sw t0, 0(a0)
    
    csrr t0, minstret
    sw t0, 4(a0)
    
    csrr t0, mhpmcounter3
    sw t0, 8(a0)
    
    csrr t0, mhpmcounter4
    sw t0, 12(a0)
    
    csrr t0, mhpmcounter5
    sw t0, 16(a0)
    
    csrr t0, mhpmcounter6
    sw t0, 20(a0)
    
    csrr t0, mhpmcounter7
    sw t0, 24(a0)
    
    csrr t0, mhpmcounter8
    sw t0, 28(a0)
    
    csrr t0, mhpmcounter9
    sw t0, 32(a0)
    
    ret