
.text 
.global switch_test

switch_test:
     lui t0, 0x04000
     lw t1, 0x10(t0)
     
     andi t1,t1,0xf
     
     mul t1,t1,t1
     
     sw t1,0(t0)
     
     j switch_test
     