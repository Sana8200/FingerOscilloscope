

# led_mirror.s

# Reads the value from the toggle switches and writes it directly to the LEDs.
# An infinite loop keeps the LEDs updated in real-time.

.section .text
.global led_mirror

led_mirror:
    # Set the base address for peripherals (0x04000000) into t0.
    lui t0, 0x04000

loop:
    # Load word from switch address (base + 0x10) into t1.
    lw t1, 0x10(t0)

    # Store word from t1 into LED address (base + 0x0).
    sw t1, 0(t0)

    # Jump back to the beginning of the loop to do it all again.
    j loop