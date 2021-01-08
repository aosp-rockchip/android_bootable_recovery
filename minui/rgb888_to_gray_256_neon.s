

.text
.global neon_rgb888_to_gray16ARM_32




neon_rgb888_to_gray16ARM_32:

	@ r0 = dest
	@ r1 = src
	@ r2 = h
	@ r3 = w

		mov			  r12,r13
		push      {r4,r5,r6,r7,r8,r9,r10,r11,lr}
		ldmfd			r12,{r7}       @r7 = vir_width
		mov       r8,r3
		mul     	r2,r2,r7
		mov     	r3,#11
		mov     	r4,#16
		mov     	r5,#5
		mov       r9,#0x00
		mov       r11,r0
		vdup.8  	d4,r3
		vdup.8  	d5,r4
		vdup.8  	d6,r5
		mov       r3,#00
LOOP:
		vld4.8  	{d0-d3},[r1]!
		vld4.8  	{d7-d10},[r1]!
		vld4.8  	{d11-d14},[r1]!
		vld4.8  	{d15-d18},[r1]!

		vmull.u8  q10,d0,d4
		vmlal.u8  q10,d1,d5
		vmlal.u8  q10,d2,d6
		vshr.u16 	q11,q10,#9
		vmov.u16   r5,d22[0]
		and        r6,r5,#0x0000000f
		vmov.u16   r5,d22[1]
		orr        r6,r6,r5,lsl #4
		vmov.u16   r5,d22[2]
		orr      	 r6,r6,r5,lsl #8
		vmov.u16   r5,d22[3]
		orr        r6,r6,r5,lsl #12
		vmov.u16   r5,d23[0]
		orr        r6,r6,r5,lsl #16
		vmov.u16   r5,d23[1]
		orr        r6,r6,r5,lsl #20
		vmov.u16   r5,d23[2]
		orr      	 r6,r6,r5,lsl #24
		vmov.u16   r5,d23[3]
		orr        r6,r6,r5,lsl #28
		str        r6,[r0],#4

		vmull.u8  q10,d7,d4
		vmlal.u8  q10,d8,d5
		vmlal.u8  q10,d9,d6
		vshr.u16 	q11,q10,#9
		vmov.u16   r5,d22[0]
		and        r6,r5,#0x0000000f
		vmov.u16   r5,d22[1]
		orr        r6,r6,r5,lsl #4
		vmov.u16   r5,d22[2]
		orr      	 r6,r6,r5,lsl #8
		vmov.u16   r5,d22[3]
		orr        r6,r6,r5,lsl #12
		vmov.u16   r5,d23[0]
		orr        r6,r6,r5,lsl #16
		vmov.u16   r5,d23[1]
		orr        r6,r6,r5,lsl #20
		vmov.u16   r5,d23[2]
		orr      	 r6,r6,r5,lsl #24
		vmov.u16   r5,d23[3]
		orr        r6,r6,r5,lsl #28
		str        r6,[r0],#4



		vmull.u8  q10,d11,d4
		vmlal.u8  q10,d12,d5
		vmlal.u8  q10,d13,d6
		vshr.u16 	q11,q10,#9
		vmov.u16   r5,d22[0]
		and        r6,r5,#0x0000000f
		vmov.u16   r5,d22[1]
		orr        r6,r6,r5,lsl #4
		vmov.u16   r5,d22[2]
		orr      	 r6,r6,r5,lsl #8
		vmov.u16   r5,d22[3]
		orr        r6,r6,r5,lsl #12
		vmov.u16   r5,d23[0]
		orr        r6,r6,r5,lsl #16
		vmov.u16   r5,d23[1]
		orr        r6,r6,r5,lsl #20
		vmov.u16   r5,d23[2]
		orr      	 r6,r6,r5,lsl #24
		vmov.u16   r5,d23[3]
		orr        r6,r6,r5,lsl #28
		str        r6,[r0],#4

		vmull.u8  q10,d15,d4
		vmlal.u8  q10,d16,d5
		vmlal.u8  q10,d17,d6
		vshr.u16 	q11,q10,#9

		vmov.u16   r5,d22[0]
		and        r6,r5,#0x0000000f
		vmov.u16   r5,d22[1]
		orr        r6,r6,r5,lsl #4
		vmov.u16   r5,d22[2]
		orr      	 r6,r6,r5,lsl #8
		vmov.u16   r5,d22[3]
		orr        r6,r6,r5,lsl #12
		vmov.u16   r5,d23[0]
		orr        r6,r6,r5,lsl #16
		vmov.u16   r5,d23[1]
		orr        r6,r6,r5,lsl #20
		vmov.u16   r5,d23[2]
		orr      	 r6,r6,r5,lsl #24
		vmov.u16   r5,d23[3]
		orr        r6,r6,r5,lsl #28
		str        r6,[r0],#4


		add       r9,r9,#32
		add       r3,r3,#32
		cmp       r3,r7
	  bne       ADD_TO_LOOP
	  mov       r3,#00
	  add       r11,r11,r8,lsr #1
		mov       r0,r11
ADD_TO_LOOP:
		cmp       r9,r2
		blo       LOOP
		pop       {r4,r5,r6,r7,r8,r9,r10,r11,pc}




.text
.global neon_rgb888_to_gray16ARM_16

neon_rgb888_to_gray16ARM_16:

	@ r0 = dest
	@ r1 = src
	@ r2 = h
	@ r3 = w

		mov			  r12,r13
		push      {r4,r5,r6,r7,r8,r9,r10,r11,lr}
		ldmfd			r12,{r7}       @r7 = vir_width
		mov       r8,r3
		mul     	r2,r2,r7
		mov     	r3,#11
		mov     	r4,#16
		mov     	r5,#5
		mov       r9,#0x00
		mov       r11,r0
		vdup.8  	d4,r3
		vdup.8  	d5,r4
		vdup.8  	d6,r5
		mov       r3,#00
LOOP_16:
		vld4.8  	{d0-d3},[r1]!
		vld4.8  	{d7-d10},[r1]!

		vmull.u8  q10,d0,d4
		vmlal.u8  q10,d1,d5
		vmlal.u8  q10,d2,d6
		vshr.u16 	q11,q10,#9
		vmov.u16   r5,d22[0]
		and        r6,r5,#0x0000000f
		vmov.u16   r5,d22[1]
		orr        r6,r6,r5,lsl #4
		vmov.u16   r5,d22[2]
		orr      	 r6,r6,r5,lsl #8
		vmov.u16   r5,d22[3]
		orr        r6,r6,r5,lsl #12
		vmov.u16   r5,d23[0]
		orr        r6,r6,r5,lsl #16
		vmov.u16   r5,d23[1]
		orr        r6,r6,r5,lsl #20
		vmov.u16   r5,d23[2]
		orr      	 r6,r6,r5,lsl #24
		vmov.u16   r5,d23[3]
		orr        r6,r6,r5,lsl #28
		str        r6,[r0],#4

		vmull.u8  q10,d7,d4
		vmlal.u8  q10,d8,d5
		vmlal.u8  q10,d9,d6
		vshr.u16 	q11,q10,#9
		vmov.u16   r5,d22[0]
		and        r6,r5,#0x0000000f
		vmov.u16   r5,d22[1]
		orr        r6,r6,r5,lsl #4
		vmov.u16   r5,d22[2]
		orr      	 r6,r6,r5,lsl #8
		vmov.u16   r5,d22[3]
		orr        r6,r6,r5,lsl #12
		vmov.u16   r5,d23[0]
		orr        r6,r6,r5,lsl #16
		vmov.u16   r5,d23[1]
		orr        r6,r6,r5,lsl #20
		vmov.u16   r5,d23[2]
		orr      	 r6,r6,r5,lsl #24
		vmov.u16   r5,d23[3]
		orr        r6,r6,r5,lsl #28
		str        r6,[r0],#4


		add       r9,r9,#16
		add       r3,r3,#16
		cmp       r3,r7
	  bne       ADD_TO_LOOP_16
	  mov       r3,#00
	  add       r11,r11,r8,lsr #1
		mov       r0,r11
ADD_TO_LOOP_16:
		cmp       r9,r2
		blo       LOOP_16
		pop       {r4,r5,r6,r7,r8,r9,r10,r11,pc}