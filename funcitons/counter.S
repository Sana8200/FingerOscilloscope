
# counter.s

# Makes the LEDs count up in binary (0, 1, 2, 3...).
# A delay loop is used to slow down the counting speed.

.section .text
.global counter

counter:
    # Set the base address for peripherals (0x04000000) into t0.
    lui t0, 0x04000

    # Initialize our counter register t1 to zero.
    li t1, 0

main_loop:
    # Display the current count on the LEDs.
    sw t1, 0(t0)

    # Increment the counter.
    addi t1, t1, 1

    # --- Delay Loop ---
    # Load a large number into t2 to determine delay length.
    # Adjust this value to make the counting faster or slower.
    li t2, 500000

delay_loop:
    # Decrement the delay counter.
    addi t2, t2, -1
    # If t2 is not zero, loop back.
    bnez t2, delay_loop
    # --- End Delay Loop ---

    # Jump back to the main loop to continue counting.
    j main_loop