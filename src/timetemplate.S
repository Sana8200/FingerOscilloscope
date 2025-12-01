# timetemplate.S
# Written 2015 by F Lundevall
# Ported 2024/06 by W Szczerek (from MIPS to RISC-V)
# Copyright abandonded - this file is in the public domain.

#############################################################
# Choose the macro syntax for RARS or DTEK-V board.         #
# The syntax for RARS is probably due to its MIPS heritage. #
#############################################################
###################
# Macros for RARS #
###################
#.macro	PUSH (%reg)
#	addi	sp,sp,-4
#	sw	%reg,0(sp) 
#.end_macro

#.macro	POP (%reg)
#	lw	%reg,0(sp)
#	addi	sp,sp,4
#.end_macro
###################
# Macros for DTEK #
###################
#.macro	PUSH reg
#	addi sp,sp,-4
#	sw \reg,0(sp) 
#.endm

#.macro	POP reg
#	lw	\reg,0(sp)
#	addi	sp,sp,4
#.endm
#############################################################


	.data
	.align 2
mytime:	.word 	0x5957
timstr:	.asciz 	"text more text lots of text\0"
	.text
	.globl timetemplate, tick, time2string, delay, hexasc, display_string, delay_cycles_asm
  .size delay_cycles_asm, .-delay_cycles_asm

# Function for displaying a string with a newline at the end	
display_string:	
	li a7,4
	ecall
	li a0, 10
	li a7,11
	ecall
	jr ra
	
timetemplate:
	la	a0, timstr
	jal     display_string
	
	# wait a little
	li	a0, 4000		# ms, 1000, or 3000, this is for getting the delay, a0 which will used in delay subroutine 
	jal	delay
	
	# call tick
	la 	a0, mytime
	jal	tick
	
	# call your function time2string
	la	a0, timstr
	la	t0, mytime
	lw	a1, 0(t0)
	jal	time2string
	
	# go back and do it all again
	j	timetemplate

	
# tick: update time pointed to by $a0
tick:	lw	t0, 0(a0)	# get time
	addi	t0, t0, 1	# increase
	andi	t1, t0, 0xf	# check lowest digit
	sltiu	t2, t1, 0xa	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x6	# adjust lowest digit
	
	andi	t1, t0, 0xf0	# check next digit
	sltiu	t2, t1, 0x60	# if digit < 6, okay
	bnez	t2, tiend
	addi	t0, t0, 0xa0	# adjust digit
	
	li	t3, 0xF
	slli	t3, t3, 0x8
	and	t1, t0, t3	# check minute digit
	addi	t3, x0, 0xA
	slli	t3, t3, 0x8
	slt	t2, t1, t3	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x600	# adjust digit - this one's okay, it's lower than 0x7FF 
	
	li	t3, 0xF
	slli	t3, t3, 0xC
	and	t1, t0, t3	# check last digit
	addi	t3, x0, 0x6
	slli	t3, t3, 0xC
	slt	t2, t1, t3	# if digit < 6, okay
	bnez	t2, tiend
	
	li	t3, 0xA
	slli	t3, t3, 0xC
	add	t0, t0, t3	# adjust last digit
tiend:	sw	t0,0(a0)	# save updated result
	jr	ra		# return

#########################################################
# Place for your functions: time2string, hex2asc, delay.#
#########################################################


# time2string : converts 16-bit NBCD time value into a MM:SS ASCII string                                   
time2string:
       addi sp, sp, -12      
       sw ra, 8(sp)        
       sw s0, 4(sp)          
       sw s1, 0(sp)         
       
       mv s0, a0              
       mv s1, a1            
       
       # 1. First minute , first digit 
       srli a0, s1, 12       
       jal hexasc            
       sb a0, 0(s0)           
       
       # 2. Second minute , second digit 
       srli a0, s1, 8        
       jal hexasc           
       sb a0, 1(s0)         
       
       # 3. Colon character : We will add the colon character, it doens't need to converted each time, 
       li t0, 0x3A           
       sb t0, 2(s0)           
       
       # 4. First second digit 
       srli a0, s1, 4     
       jal hexasc         
       sb a0, 3(s0)           
              
       # 5. Second second digit, last digit of the time, 4 lsb  
       mv a0, s1             
       jal hexasc            
       sb a0, 4(s0)           
       
       # 6. Null byte
       li t0, 0x00          
       sb t0, 5(s0)         
              
       # Restoring hte original register values, restoring is done in the reverse order that we saved them 
       lw s1, 0(sp)          
       lw s0, 4(sp)
       lw ra, 8(sp) 
       addi sp, sp, 12       
          
       jr ra     
       
       
hexasc:
    andi a0, a0, 0xF            
    li   t0, 10                 
    bge  a0, t0, letters         

    addi a0, a0, 0x30           
    jr   ra                    

letters:
    addi a0, a0, 0x37           
    jr   ra                    
    


# Delay function runs a busy loop to make delay in the program, inefficient way to create a delay 
delay:
    blez a0, exit_delay         
                               
while_loop:                                                                                                                                       
    li t1, 3000                
    li t0, 0                 
    
for_loop:                     
    addi t0, t0, 1            
    blt t0, t1, for_loop        
    addi a0, a0, -1            
    bgtz a0, while_loop      

exit_delay:                    
    jr ra                      
  # ret 





# void delay_cycles_asm(uint32_t cycles)
# Arguments: a0 = number of cycles to delay
delay_cycles_asm:
    li t0, 9                    # Load divisor (9 cycles per iteration)
    divu t1, a0, t0             # t1 = cycles / 9 (iterations needed)
    beqz t1, exit_this_delay         # If zero iterations, exit immediately
    
delay_loop:
    addi t1, t1, -1             # Decrement iteration counter
    bnez t1, delay_loop         # Loop if not zero
    
exit_this_delay:
    ret                         # Return to caller

