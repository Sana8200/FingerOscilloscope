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
	.globl timetemplate, tick, time2string, delay, hexasc, display_string 

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
# a0 memory address where the output string will be stored 
# a1 16-bit time value 
# no return value 
  
                                     
time2string:
       
       # save registers that will modify, save values in the return address ra, and any saved registers s0-s11 that we use 
       addi sp, sp, -12       # Adjust stack pointer to make space for 3 registers 
       sw ra, 8(sp)           # Saveing return address, so we can return to timetemplate
       sw s0, 4(sp)           # Saving orginial value of s0
       sw s1, 0(sp)           # Saving original value of s1 
       
       
       mv s0, a0              # Copying destination address a0 into s0, saviing string address 
       mv s1, a1              # Copying time value a1 into s1, saving time value
       
       # 1. First minute , first digit 
       srli a0, s1, 12        # Shift right by 12 bits to isolate the first digit
       jal hexasc             # Convert first digit to its ASCII character 
       sb a0, 0(s0)           # Store the ASCII character of the first digit at the start of the string into a0 memory address for the string 
       
       # 2. Second minute , second digit 
       srli a0, s1, 8         # Shift right by 8 bits 
       jal hexasc             # Convert the lowest 4 bits to its ASCII character 
       sb a0, 1(s0)           # Store the ASCII character at the second position in the string which has index 1 
       
       # 3. Colon character : We will add the colon character, it doens't need to converted each time, 
       li t0, 0x3A            # Loading the ASCII code for : into the temporary register t0
       sb t0, 2(s0)           # Storing the colon at the third position in the memory in the String ( index 2)
       
       # 4. First second digit 
       srli a0, s1, 4         # Shift right by 4 bits 
       jal hexasc             # Convert to ASCII
       sb a0, 3(s0)           # Store the first digit of second at 4th position in the memory string 
       
       
       # 5. Second second digit, last digit of the time, 4 lsb  
       mv a0, s1              # We don't need any shift, the digit is already in the lowest 4 bit 
       jal hexasc             # Converting to ASCII
       sb a0, 4(s0)           # Storing ASCII character at the 5th position in the string in the memory 
       
       # 6. Null byte
       li t0, 0x00            # Loding the value for null byte which is 0 
       sb t0, 5(s0)           # Store the null byte ro mark the end of the string which is 6th position, index 5
       
       
       # Restoring hte original register values, restoring is done in the reverse order that we saved them 
       lw s1, 0(sp)          
       lw s0, 4(sp)
       lw ra, 8(sp) 
       addi sp, sp, 12        # Deallocate the space on the stack 
       
       
       jr ra                 # Return to the calling funciton timetemplate 
       
       
 

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

# a0   number of milliseconds (ms)   a0 = ms 
# t0  counter for the inner loop ( i )     t0 = i 
# t1 = upper limit for i (t0), constant 4711

# Constant 4711 which is the upper limit for inner loop counter i in the c code, determines how long the inner loop runs.
# We can change and adjust it by testing to make one pass of the outer_loop takes approximately 1 milliseconds of simulated time 


delay:
    blez a0, exit_delay          # Branch to exit_delay if a0 <= 0 which corresponds to ms <= 0, first check for the C code, while(ms>0)
  # bne a0, x0, exit_delay     # If a0 is zero or negative, we don't need to do anything, we jump straight to the end  
      
                                                            
### While loop is the main counter, keeping track of how many 1 miilisecond interval you still need to wait, a0 which is ms holds this count 
### If we call delay(1000) the a0 starts at 1000 go through the inner for loop and decrement by one each time until it's 0, it's gonna stop when a0 > 0 or ms > 0                                 
while_loop:                      # outer loop runs once for each millisecond (a0), Outer loop is while loop's body 
                              
################## 1 ms timer                                                                                              
    # for loop start (i = 0 ; i < 4711 ; i = i + 1)                         
    li t1, 3000                  # constant upper limit for i in the inner for loop     t1 = 4711
    li t0, 0                     #  i, t0 = 0 , reseting the inner loop counter i to 0 
    
for_loop:                        # inner loop is the for loop's body which counts up to the limit to waste time and make delay. 

    addi t0, t0, 1               # incrementing the counter i = i + 1 
    blt t0, t1, for_loop         # if i < limit , if t0 < t1 then continure the inner loop, branch back to the start of the inner_loop   i < 4711
    
################## end of 1 ms timer   
  
    # End of the 'for' loop, At this point, about 1 millisecond of time has passed.

    addi a0, a0, -1              # ms = ms - 1,  decrementing the main millisecond counter ms (a0) by one 

    bgtz a0, while_loop          # final check, if ms > 0, which means if a0 is still greater than 0 then branch back to the outer while loop to wait for another millisecond
  # blt x0, a0, while loop     # bgtz is the instruction that makes the outer while loop to repeat  


exit_delay:                      # Exit point for the function 
    jr ra                        # returning from a subroutine
  # ret 
