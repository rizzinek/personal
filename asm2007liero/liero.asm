		org 100h
		segment .text

start:		nop

		call	get_tacts
		call	init_keyboard

		mov ah, 0
		mov al, 13h
		int 10h	; 320x200x256

		call	start_timer

mainloop:
		call	clear_buffer
		call	key_handler
		call	count_physics
		call	draw_objects
		call	draw_buffer
		call	_wait
		jmp	mainloop
		ret
; End of main function

exit:		
		mov	dx, [old_key_off]
		mov	ds, [old_key_seg]
		mov	ax, 2509h
		int	21h
		mov	ah, 0
		mov 	al, 3
		int 	10h	; text mode on
		mov	ah, 4Ch
		int	21h	; successful exit
		ret

draw_objects:

		mov di, 30
		mov si, 170
		mov cx, 64
		mov dx, [life1]
		cmp dx, 0
		je .nolf1
.lf1:		mov ax, di
		mov bx, si
		call putpixel_b
		dec dx
		add di, 2
		cmp dx, 0
		jne .lf1

.nolf1:
		mov di, 290
		mov si, 170
		mov cx, 79
		mov dx, [life2]
		cmp dx, 0
		jz .nolf2
.lf2:		mov ax, di
		mov bx, si
		call putpixel_b
		dec dx
		sub di, 2
		cmp dx, 0
		jne .lf2
.nolf2:
  ;worm1
		;;;; Xcoord = x/64
		mov cx, word [x1]
		sar cx, 6
		mov ax, cx

		;;;; Ycoord = y/64
		mov cx, word [y1]
		sar cx, 6
		mov bx, cx
    
		cmp byte [life1], 0
		jnz .alive1
		mov bp, worm1_pic_dead
		jmp .w1
  .alive1:
		cmp byte [last1], 0
		jne .lw1
		mov bp, worm1_pic_r
		jmp .w1
  .lw1:
		mov bp, worm1_pic_l
  .w1:
		call put_sprite
  ;crosshair1
		cmp byte [life1], 0
		je .noxhair1
		mov cx, word [x1]
		cmp byte [last1], 0
		jne .la
		add cx, [crossX1]
		jmp .a
  .la:
		sub cx, [crossX1]
	.a:
		sar cx, 6
		mov ax, cx

		mov cx, word [y1]
		add cx, [crossY1]
		sar cx, 6
		mov bx, cx

		mov	bp, aim_pic
		call	put_sprite
		.noxhair1:
  ;1st worm's bullets
		xor	di, di
.b1:
		xor	ax, ax
		xor	bx, bx
		mov	ax, word [xbullets1 + di]
		mov	bx, word [ybullets1 + di]
		shr	ax, 6
		shr	bx, 6
		mov	cl, 14
		call	putpixel_b
		add	di, 2
		cmp	di, 200
		jne	.b1
		
  ;worm2
		;;;; Xcoord = x/64
		mov cx, word [x2]
		sar cx,6
		mov ax, cx
		;;;; Ycoord = y/64
		mov cx, word [y2]
		sar cx,6
		mov bx, cx

		cmp byte [life2], 0
		jnz .alive2
		mov bp, worm2_pic_dead
		jmp .w2
  .alive2:
		cmp	byte [last2], 0
		jne	.lw2
		mov	bp, worm2_pic_r
		jmp	.w2
.lw2:		mov	bp, worm2_pic_l
.w2:		call	put_sprite
  ;crosshair2
		cmp byte [life2], 0
		je .noxhair2
		mov cx, word [x2]
		cmp byte [last2], 0
		jne .la2
		add cx, [crossX2]
		jmp .a2
  .la2:
		sub cx, [crossX2]
  .a2:
		sar cx, 6
		mov ax, cx

		mov cx, word [y2]
		add cx, [crossY2]
		sar cx, 6
		mov bx, cx

		mov	bp, aim_pic
		call	put_sprite
  .noxhair2:
  ;2nd worm's bullets
		xor	di, di
.b2:
		xor	ax, ax
		xor	bx, bx
		mov	ax, [xbullets2 + di]
		mov	bx, [ybullets2 + di]
		shr	ax, 6
		shr	bx, 6
		mov	cl, 14
		call	putpixel_b
		add di, 2
		cmp	di, 200
		jne	.b2
		mov ax, 0
		mov bx, 0
		mov cx, 0
		call putpixel_b
		ret

get_tacts:
		xor	ax, ax
		mov	es, ax
		mov	ax, [es:046Ch]
		inc	ax
.l1:
		cmp	ax, [es:046Ch]
		jb	.l1

		mov	cx, [es:046Ch]
		rdtsc
		push	edx
		push	eax
		add	cx, 91
.wait:
		cmp	cx, [es:046Ch]
		ja	.wait
		rdtsc
		pop	ebx
		sub	eax, ebx
		pop	ebx
		sbb	edx, ebx
		mov	ebx, 30*5
		div	ebx
		mov     [tacts], eax 
		ret

start_timer:
		rdtsc
		mov	[timer], eax
		mov	[timer + 4], edx
		ret

_wait:
.l1:		rdtsc
		sub	eax, [timer]
		cmp	edx, [timer + 4]
		je	.l2
		add	eax, 0EFFFFFFFh
.l2:		cmp	eax, [tacts]
		jb	.l1
		inc	word [time]
		rdtsc
		mov	[timer], eax
		mov	[timer + 4], edx
		ret

init_keyboard:
		mov	ax, 3509h
		int	21h
		mov	[old_key_off], bx
		mov	[old_key_seg], es
		mov	ax, 2509h
		mov	dx, keyboard_int
		int	21h	
		ret
		

keyboard_int:
		push	ax
		push	bx
		mov	ah, 1
		in	al, 60h
		test	al, 80h
		jz	.x1
		xor	ax, 180h
.x1:		mov	bl, al
                mov	bh, 0
		mov	[key_pressed + bx], ah
		in	al, 61h
		mov	ah, al
		or	al, 80h
		out	61h, al
		mov	al, ah
		out	61h, al
		mov	al, 20h
		out	20h, al
		pop	bx
		pop	ax
		iret


key_handler:
		cmp	byte [key_pressed + 1], 0
		je	.n1
		call	exit
.n1:		cmp	byte [key_pressed + 30], 0
		je	.n2
		call	key_left1
.n2:		cmp	byte [key_pressed + 32], 0
		je	.n3
		call	key_right1
.n3:		cmp	byte [key_pressed + 17], 0
		je	.n4
		call	key_up1
.n4:		cmp	byte [key_pressed + 31], 0
		je	.n5
		call	key_down1
.n5:		cmp	byte [key_pressed + 36], 0
		je	.n6
		call	key_left2
.n6:		cmp	byte [key_pressed + 38], 0
		je	.n7
		call	key_right2
.n7:		cmp	byte [key_pressed + 23], 0
		je	.n8
		call	key_up2
.n8:		cmp	byte [key_pressed + 37], 0
		je	.n9                 
		call	key_down2
.n9:		cmp 	byte [key_pressed + 47], 0
		je 	.n10
		call 	key_jump1
.n10:		cmp	byte [key_pressed + 46], 0
		je	.n11
		call	key_fire1
.n11:		cmp	byte [key_pressed + 50], 0
		je	.n12
		call	key_jump2
.n12:           cmp	byte [key_pressed + 49], 0
		je	.n13
		call	key_fire2
.n13:		
		ret


key_left1:	
		mov ax, 0
		mov bx, -32
		mov cx, 0
		call move_worm
		mov	al, 1
		mov	byte [last1], al
		ret
key_right1:	
		mov ax, 0
		mov bx, 32
		mov cx, 0
		call move_worm
		mov	al, 0
		mov	byte [last1], al
		ret
key_up1:	
    cmp word [crossY1], 1
    jns up1_q2
  up1_q1:
    cmp word [crossX1], 64
    jz up1_left
    sub word [crossX1], 128
  up1_left:
    jmp up1_right
  up1_q2:
    cmp word [crossX1], 1088
    jz up1_right
    add word [crossX1], 128
  up1_right:
    cmp word [crossY1], -896
    jz up1_up
    sub word [crossY1], 128
  up1_up:
		ret
key_jump1:
	add word [vy1], -5
	ret

key_jump2:
	add word [vy2], -5
	ret
key_down1:
    cmp word [crossY1], 1
    jns down1_q2
  down1_q1:
    cmp word [crossX1], 1088
    jz down1_right
    add word [crossX1], 128
  down1_right:
    jmp down1_left
  down1_q2:
    cmp word [crossX1], 64
    jz down1_left
    sub word [crossX1], 128
  down1_left:
    cmp word [crossY1], 1024
    jz down1_down
    add word [crossY1], 128
  down1_down:
		ret

key_left2:
		mov ax, 1
		mov bx, -32
		mov cx, 0
		call move_worm
		mov	al, 1
		mov	byte [last2], al
		ret
key_right2:
		mov ax, 1
		mov bx, 32
		mov cx, 0
		call move_worm
		mov	al, 0
		mov	byte [last2], al
		ret
key_up2:
    cmp word [crossY2], 1
    jns up2_q2
  up2_q1:
    cmp word [crossX2], 64
    jz up2_left
    sub word [crossX2], 128
  up2_left:
    jmp up2_right
  up2_q2:
    cmp word [crossX2], 1088
    jz up2_right
    add word [crossX2], 128
  up2_right:
    cmp word [crossY2], -896
    jz up2_up
    sub word [crossY2], 128
  up2_up:
		ret
key_down2:	
    cmp word [crossY2], 1
    jns down2_q2
  down2_q1:
    cmp word [crossX2], 1088
    jz down2_right
    add word [crossX2], 128
  down2_right:
    jmp down2_left
  down2_q2:
    cmp word [crossX2], 64
    jz down2_left
    sub word [crossX2], 128
  down2_left:
    cmp word [crossY2], 1024
    jz down2_down
    add word [crossY2], 128
  down2_down:
		ret

key_fire1:	
		cmp byte [life1], 0
		je .l1
		mov ax, [last_fire_time1]
		add ax, 2
		mov bx, [time]
		cmp bx, ax
		jb .l1
		mov [last_fire_time1], bx
		mov bx, [bulletn1]
		cmp bx, 200
		jne .lt
		mov bx, 0
.lt:
		mov ax, [x1]
		add ax, 3
		cmp byte [last1], 0
		jne .la2
		add ax, [crossX1]
		jmp .a2
.la2:		sub ax, [crossX1]
.a2:
		mov word [xbullets1 + bx], ax
		mov ax, word [y1]
		add ax, 4
		add ax, word [crossY1]
		mov word [ybullets1 + bx], ax
		
		xor ax, ax
		cmp byte [last1], 0
		jne	.lw1
		add ax, [crossX1]
		sar ax, 3
		add ax, [vx1]
		mov word [vxbullets1 + bx], ax
		jmp	.w1
.lw1:		sub ax, [crossX1]
		add ax, 2*64
		sar ax, 3
		add ax, [vx1]
		mov word [vxbullets1 + bx], ax
.w1:		
		mov ax, [crossY1]
		sar ax, 3
		add ax, [vy1]
		mov word [vybullets1 + bx], ax

		add bx, 2
		mov [bulletn1], bx
.l1:
		ret

key_fire2:	
		cmp byte [life2], 0
		je .l2
		mov ax, [last_fire_time2]
		add ax, 2
		mov bx, [time]
		cmp bx, ax
		jb .l2
		mov [last_fire_time2], bx
		mov bx, [bulletn2]
		cmp bx, 200
		jne .lt
		mov bx, 0
.lt:
		mov ax, [x2]
		add ax, 3
		cmp byte [last2], 0
		jne .la2
		add ax, [crossX2]
		jmp .a2
.la2:		sub ax, [crossX2]
.a2:
		mov word [xbullets2 + bx], ax
		mov ax, word [y2]
		add ax, 4
		add ax, word [crossY2]
		mov word [ybullets2 + bx], ax

		xor ax, ax
		cmp byte [last2], 0
		jne	.lw2
		add ax, [crossX2]
		sar ax, 3
		add ax, [vx2]
		mov word [vxbullets2 + bx], ax
		jmp	.w2
.lw2:		sub ax, [crossX2]
		add ax, 2*64
		sar ax, 3
		add ax, [vx2]
		mov word [vxbullets2 + bx], ax
.w2:		mov ax, [crossY2]
		sar ax, 3
		add ax, [vy2]
		mov word [vybullets2 + bx], ax

		add bx, 2
		mov [bulletn2], bx
.l2:
		ret

check_hits:
  ;1st worm's Bullets
    xor di, di
  hits1:
    mov ax, [x2]
    add ax, 512 ; 8 * 64
    cmp [xbullets1 + di], ax
    jg nohit1
    sub ax, 512
    cmp [xbullets1 + di], ax
    jl nohit1
    mov ax, [y2]
    add ax, 512
    cmp [ybullets1 + di], ax
    jg nohit1
    sub ax, 512
    cmp [ybullets1 + di], ax
    jl nohit1
    mov word [xbullets1 + di], 0
    mov word [ybullets1 + di], 0
    mov word [vxbullets1 + di], 0
    mov word [vybullets1 + di], 0
    cmp byte [life2], 0
    je nohit1
    dec byte [life2]
  nohit1:
    add di, 2
    cmp di, 200
    jnz hits1
  ;2nd worm's bullets
    xor di, di
  hits2:
    xor ax, ax
    add ax, word [x1]
    add ax, 512
    cmp word [xbullets2 + di], ax
    jg nohit2
    sub ax, 512
    cmp word [xbullets2 + di], ax
    jl nohit2
    xor ax, ax
    add ax, word [y1]
    add ax, 512
    cmp word [ybullets2 + di], ax
    jg nohit2
    sub ax, 512
    cmp word [ybullets2 + di], ax
    jl nohit2
    mov word [xbullets2 + di], 0
    mov word [ybullets2 + di], 0
    mov word [vxbullets2 + di], 0
    mov word [vybullets2 + di], 0
    cmp byte [life1], 0
    je nohit2
    dec byte [life1]
  nohit2:
    add di, 2
    cmp di, 200
    jnz hits2

clear_buffer:	;clear buffer
	;use - ax, es, di
		cld
		mov	ax, ds
		mov	es, ax
		mov	di, buffer
		xor	eax, eax
		mov	cx, 03840h
	rep	stosd	; [es:di] <- eax, di+=4;
		ret

draw_buffer:	;show buffer frame
	;use - ax, cx, es, di, si
		cld
		push	ds
		mov	ax, 0A000h
		mov	es, ax
		xor	di, di
		mov	si, buffer
		mov	cx, 03840h
	rep	movsd   ; [es:di] <- [ds:si], di+=1, si+=1;
		pop	ds
		ret

put_sprite:	;set sprite at screen buffer
	;ax <--- x; bx <--- y; bp <--- addres_of_sprite
	;use -- cx, dx, si, di	;save them befor call this function
		mov	di, -1
.w_y:		inc	di	;y loop
		mov	dx, -1
.w_x:		inc	dx	;x loop
		mov	si, di
		shl	si, 3
		add	si, dx
		mov	cl, [bp + si]
		cmp	cl, 0	;transparent pixel
		jz	.no_p
		push	ax
		push	bx
		add	ax, dx
		add	bx, di
		call	putpixel_b
		pop	bx
		pop	ax
.no_p:		cmp	dx, 7
		jnz	.w_x
		cmp	di, 7
		jnz	.w_y
		ret

;Deprecated
putpixel:	;set pixel at screen 
	;ax <--- x; bx <--- y; cl <--- color
		shl	bx, 6
		add	ax, bx
		shl	bx, 2
		add	bx, ax
		mov	ax, 0A000h
		mov 	es, ax
		mov	[es:bx], cl
		ret

putpixel_b:	;set pixel at screen buffer
	;ax <--- x; bx <--- y; cl <--- color
		shl	bx, 6		; bx = 64 * y
		add	ax, bx		; ax = 64 * y + x
		shl	bx, 2		; bx = 256 * y
		add	bx, ax		; bx = 320 * y + x
		mov	[buffer + bx], cl
		ret


move_worm:
	; ax <--- number of worm (0 or 1); bx <--- dx; cx <--- dy
	; returns: ax = 0 <=> unsuccesfull movement
		cmp ax, 0
		jne .secondWorm
		.firstWorm:
		mov ax, word [x1]
		add ax, bx
		cmp ax, word [minx]
		jbe .cantMoveFirst
		cmp ax, word [maxx]
		jae .cantMoveFirst
		mov word [x1], ax	; succesfull x change

		mov ax, word [y1]
		add ax, cx
		cmp ax, word [miny]
		jbe .cantMoveFirst
		cmp ax, word [maxy]
		jae .cantMoveFirst
		mov word [y1], ax	; succesfull y change
		jmp .l1		
		.cantMoveFirst:		
		mov word[vx1], 0
		mov word[vy1], 0
		mov ax, 0
		ret
		
		.secondWorm:
		mov ax, word [x2]
		add ax, bx
		cmp ax, word [minx]
		jbe .cantMoveSecond
		cmp ax, word [maxx]
		jae .cantMoveSecond
		mov word [x2], ax	; succesfull x change
		mov ax, word [y2]
		add ax, cx
		cmp ax, word [miny]
		jbe .cantMoveSecond
		cmp ax, word [maxy]
		jae .cantMoveSecond
		mov word [y2], ax	; succesfull y change
		jmp .l1		
		.cantMoveSecond:		
		mov word[vx2], 0
		mov word[vy2], 0
		mov ax, 0
		.l1:
		ret


count_physics:	; modifying physical variables + coordinates according to laws of physics
		;gravitation part
		mov ax, [vy1]
		add ax, [gacc]
		mov [vy1], ax
		
		mov ax, [vy2]
		add ax, [gacc]
		mov [vy2], ax


			
		mov ax,0
		mov bx,[vx1]
		mov cx,[vy1]
		call move_worm

		mov ax,1
		mov bx,[vx2]
		mov cx,[vy2]
		call move_worm


   ;1st worm's bullets
		xor	di, di
.b1:
		xor	ax, ax
		xor	bx, bx
		mov ax, word [ybullets1 + di]
		mov bx, word [vybullets1 + di]
		add bx, [gacc2] ; gravity on bullets
		mov word [vybullets1 + di], bx
		add ax, bx
		cmp ax, 11520
		jae .bullet1Fail
		cmp ax, 0
		jbe .bullet1Fail

		mov word [ybullets1 + di], ax

		mov ax, word [xbullets1 + di]
		mov bx, word [vxbullets1 + di]
		add ax, bx

		cmp ax, 20480
		jae .bullet1Fail
		cmp ax, 0
		jbe .bullet1Fail

		mov word [xbullets1 + di], ax

		jmp .next
		.bullet1Fail:
		mov word [xbullets1 + di], 0
		mov word [ybullets1 + di], 0
		mov word [vxbullets1 + di], 0
		mov word [vybullets1 + di], 0
		.next:
		add di, 2
		cmp di, 200
		jne	.b1


   ;2st worm's bullets
		xor	di, di
.b2:
		xor	ax, ax
		xor	bx, bx
		mov ax, word [ybullets2 + di]
		mov bx, word [vybullets2 + di]
		add bx, [gacc2] ; gravity on bullets
		add ax, bx
		mov word [vybullets2 + di], bx
		cmp ax, 11520
		jae .bullet2Fail
		cmp ax, 0
		jbe .bullet2Fail		
		mov word [ybullets2 + di], ax

		mov ax, word [xbullets2 + di]
		mov bx, word [vxbullets2 + di]
		add ax, bx

		cmp ax, 20480
		jae .bullet2Fail
		cmp ax, 0
		jbe .bullet2Fail

		mov word [xbullets2 + di], ax

		jmp .next2
		.bullet2Fail:
		mov word [xbullets2 + di], 0
		mov word [ybullets2 + di], 0
		mov word [vxbullets2 + di], 0
		mov word [vybullets2 + di], 0
		.next2:
		add di, 2
		cmp di, 200
		jne	.b2


 		call check_hits
		
		ret
		





		segment .data

; Variables
old_key_off:    dw	0
old_key_seg:	dw      0
tacts:		dd	0 	; Tacts in 1/30 sec
timer:		dd	0, 0	;
x1:		dw      1092	; x 1 worm
y1:		dw	4000	; y 1 worm
x2:		dw      19000	; x 2 worm
y2:		dw	4000	; y 2 worm
last1:		db	0	; 
last2:		db	1	; 
life1:          dw	50
life2:		dw	50
key_pressed:	times 128 db 0	; keys
crossX1:	dw	1088 ; 17 * 64
crossY1:	dw	0
crossX2:	dw	1088 ; 17 * 64
crossY2:	dw	0


time:		dw	0
last_fire_time1:dw	0
last_fire_time2:dw	0

; borders for worm
minx:	dw	1090
maxx:	dw	19150
miny:	dw	870
maxy:	dw	8800

;physical variables
vx1:	dw	0
vy1:	dw	0
vx2:    dw	0
vy2:    dw	0
gacc:   dw      2
gacc2:	dw	2


; Sprite pictures (must be 8*8)
worm1_pic_r:	db 	0, 0, 0, 0, 112, 112, 0, 0,\
			0, 0, 0, 112, 40, 40, 40, 0,\
			0, 0, 0, 40, 112, 112, 4, 0,\
			0, 0, 0, 40, 26, 26, 24, 22,\
			0, 0, 0, 112, 40, 40, 112, 4,\
			0, 4, 40, 4, 40, 40, 0, 0,\
			4, 40, 40, 40, 40, 4, 0, 0,\
	                112, 40, 0, 40, 4, 0, 0, 0

worm1_pic_l:	db 	0, 0, 112, 112, 0, 0, 0, 0,\
			0, 40, 40, 40, 112, 0, 0, 0,\
			0, 4, 112, 112, 40, 0, 0, 0,\
			22, 24, 26, 26, 40, 0, 0, 0,\
			4, 112, 40, 40, 112, 0, 0, 0,\
			0, 0, 40, 40, 4, 40, 4, 0,\
			0, 0, 4, 40, 40, 40, 40, 4,\
	                0, 0, 0, 4, 40, 0, 40, 112

worm1_pic_dead: db	0, 0, 0, 42, 42, 0, 0, 0,\
			0, 0, 0, 42, 42, 0, 0, 0,\
			0, 42, 42, 14, 14, 42, 42, 0,\
			0, 42, 42, 14, 14, 42, 42, 0,\
			0, 0, 0, 60, 60, 0, 0, 0,\
			0, 0, 112, 40, 40, 112, 0, 0,\
			0, 112, 40, 40, 40, 40, 112, 0,\
			112, 40, 40, 40, 40, 40, 40, 112

worm2_pic_r:	db 	0, 0, 0, 0, 1, 1, 0, 0,\
			0, 0, 0, 1, 55, 55, 55, 0,\
			0, 0, 0, 55, 1, 32, 1, 0,\
			0, 0, 0, 55, 26, 26, 24, 22,\
			0, 0, 0, 1, 55, 55, 1, 1,\
			0, 1, 55, 1, 55, 55, 0, 0,\
			1, 55, 55, 55, 55, 1, 0, 0,\
	                1, 55, 0, 55, 1, 0, 0, 0
 
worm2_pic_l:	db 	0, 0, 1, 1, 0, 0, 0, 0,\
			0, 55, 55, 55, 1, 0, 0, 0,\
			0, 1, 32, 1, 55, 0, 0, 0,\
			22, 24, 26, 26, 55, 0, 0, 0,\
			1, 1, 55, 55, 1, 0, 0, 0,\
			0, 0, 55, 55, 1, 55, 1, 0,\
			0, 0, 1, 55, 55, 55, 55, 1,\
	                0, 0, 0, 1, 55, 0, 55, 1

worm2_pic_dead: db	0, 0, 0, 42, 42, 0, 0, 0,\
			0, 0, 0, 42, 42, 0, 0, 0,\
			0, 42, 42, 14, 14, 42, 42, 0,\
			0, 42, 42, 14, 14, 42, 42, 0,\
			0, 0, 0, 75, 75, 0, 0, 0,\
			0, 0, 1, 32, 32, 1, 0, 0,\
			0, 1, 32, 32, 32, 32, 1, 0,\
			1, 32, 32, 32, 32, 32, 32, 1

aim_pic:	db 	0, 0, 0, 0, 0, 0, 0, 0,\
			0, 0, 192, 0, 0, 0, 0, 0,\
			0, 0, 47, 0, 0, 0, 0, 0,\
			192, 47, 0, 47, 192, 0, 0, 0,\
			0, 0, 47, 0, 0, 0, 0, 0,\
			0, 0, 192, 0, 0, 0, 0, 0,\
			0, 0, 0, 0, 0, 0, 0, 0,\
			0, 0, 0, 0, 0, 0, 0, 0

bulletn1:	dw 0
bulletn2:	dw 0
xbullets1:	times 100 dw 0
ybullets1:	times 100 dw 0
vxbullets1:	times 100 dw 0
vybullets1:	times 100 dw 0
xbullets2:	times 100 dw 0
ybullets2:	times 100 dw 0
vxbullets2:	times 100 dw 0
vybullets2:	times 100 dw 0


		segment .bss

; Video buffer
buffer:		resb	57600

		end
