

.global led_test

led_test:
    lui t0, 0x04000
    sw a0, 0(t0)
    jr ra
    