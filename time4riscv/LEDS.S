

.global led_test

led_test:
    lui t0, 0x04000
    sw a0, 0(t0)
    jr ra

# Equivalent C code : 
#     void led_test(unsigned int value) {
#
#     // Create a volatile pointer that points to the specific hardware address.
#
#     volatile unsigned int *led_register = (volatile unsigned int *)0x04000000;
#
#     // Dereference the pointer to write the input 'value' to that memory address.
#
#     *led_register = value;
#     }