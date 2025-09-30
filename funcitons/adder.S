
# adder.s
#
# Reads two 4-bit numbers from the switches and displays their sum on the LEDs.
# Number A = Switches [3:0]
# Number B = Switches [7:4]

.section .text
.global adder

adder:
    # Set the base address for peripherals (0x04000000) into t0.
    lui t0, 0x04000

adder_loop:
    # Load the full 10-bit switch value into t1.
    lw t1, 0x10(t0)

    # --- Isolate Number A ---
    # Mask t1 to get only the lower 4 bits (switches 0-3).
    andi t2, t1, 0xf      # t2 now holds Number A

    # --- Isolate Number B ---
    # Shift t1 right by 4 to move the upper bits down.
    srli t3, t1, 4
    # Mask the result to get only the lower 4 bits (now switches 4-7).
    andi t3, t3, 0xf      # t3 now holds Number B

    # --- Perform Addition ---
    # Add the two numbers together, store result in t4.
    add t4, t2, t3

    # --- Display Result ---
    # Write the sum from t4 to the LEDs.
    sw t4, 0(t0)

    # Loop forever.
    j adder_loop