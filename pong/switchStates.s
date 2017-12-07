        .arch msp430g2553
        .p2align 1,0
        .text
	
        .data
num:	.word 0 		

        .text
buzz:	.word buttonOne		
        .word buttonTwo 	
        .word buttonThree   
        .word buttonFour      


	.global switchStates
	
	
switchStates:
	
    buttonOne:
        call #p2sw_read
        cmp #1, r12
        jnz buttonTwo
        mov #1, r12
        jmp end
    buttonTwo:
        call #p2sw_read
        cmp #2, r12
        jnz buttonThree
        mov #2, r12
        jmp end
    buttonThree:
        call #p2sw_read
        cmp #3, r12
        jnz buttonFour
        mov #3, r12
        jmp end
    buttonFour:
        call #p2sw_read
        cmp #4, r12
        jnz end
        mov #4, r12
        jmp end
    end:
        pop r0			; return  
