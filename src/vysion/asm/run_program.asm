public _vysion_asm_RunProgram
public _vysion_asm_ConfigureRunProgram


include 'include/ez80.inc'
include 'include/tiformat.inc'
include 'include/ti84pceg.inc'

;we're making the args be in saferam because sp gets messed up (and has to, unfortunately)
;writing this function in asm for no reason at all
_vysion_asm_ConfigureRunProgram:
	ld iy, 0
	add iy, sp
	;the program name
	ld hl, (iy + 3)
	ld de, ti.appData
	ld bc, 10
	ldir
	;whether it's asm
	ld hl, ti.appData + 10
	ld a, (iy + 6)
	ld (hl), a
	;should it be run with the reloader
	ld hl, ti.appData + 11
	ld a, (iy + 9)
	ld (hl), a
	ret

;intention: Run an arbitrary program, given the entry that should be put into OP1, and whether it's an asm program
;if use_reloader is true, then it should also reload the entry program

;name should also include the type of the program as the first character (TI type)
;so the final entry would maybe look something like TI_PRGM_TYPE, "TEST" or 0x05TEST
;assumes that OP1 is VYSION or whatever program called it
;void vysion_asm_RunProgram(char *name, bool is_asm, bool use_reloader);

_vysion_asm_RunProgram:
copy_to_saferam:
	ld sp, (__exitsp)
	pop iy, af, hl
	ld (hl), a
 	call $04F0
	;fix the return stuff
	ld hl, run_program_addr
	ld de, ti.cursorImage
	ld bc, lengthof code
	ldir
   	ld hl, (ti.OP1)
   	ld (tmp0), hl
   	ld hl, (ti.OP1 + 3)
   	ld (tmp1), hl
   	ld hl, (ti.OP1 + 6)
   	ld (tmp2), hl
	;this is where the args come in
	ld hl, ti.appData
	call ti.Mov9ToOP1
	;jump to the appropriate part, as needed
	;now save some things off the stack so we can use them later
	ld a, (ti.appData + 10)
	ld (is_asm), a
	ld a, (ti.appData + 11)
	ld (use_reloader), a
	;figure out if we need to use the asm or basic bit
	ld a, (is_asm)
	cp a, 1
	jp nz, run_basic_program
	jp run_asm_program
	;jp run_asm_program
	;otherwise

;entry point, requires OP1 be set and the arguments be on the stack
;actually they should be in appdata now
run_program_addr:
	virtual at ti.cursorImage
run_program:
	;here, why not
	extern __exitsp

clear_usermem:
	;clear VYSION out of userMem
	ld de, (ti.asm_prgm_size)
   	ld hl, 0
   	ld (ti.asm_prgm_size), hl
   	ld hl, ti.userMem         ; delete the current program from usermem
   	call ti.DelMem
	ret
	
run_basic_program:
	ld hl, run_program_basic_str
	call ti.HomeUp
	call ti.PutS
	;clear up the graphics
	call ti.ClrScrnFull
	call ti.HomeUp
	call clear_usermem
	ld hl, use_reloader
	bit 0, (hl)
	jr z, reloader_return_basic
	ld hl, reloader_return_basic
	ld (return), hl
	jp set_up_reloader
reloader_return_basic:
	ld iy, ti.flags
	set ti.graphDraw, (iy + ti.graphFlags)
   	set ti.appTextSave, (iy + ti.appFlags)
   	set ti.progExecuting, (iy + ti.newDispF)
   	set ti.appAutoScroll, (iy + ti.appFlags)
   	set ti.cmdExec, (iy + ti.cmdFlags)
   	res ti.onInterrupt, (iy + ti.onFlags)
   	xor a, a
   	ld (ti.kbdGetKy),a
   	call   ti.EnableAPD
   	ei
	ld hl, use_reloader
	bit 0, (hl)
	jr z, return_push_return_address_basic
	ld hl, return_push_return_address_basic
	ld (return), hl
	jp push_return_address
return_push_return_address_basic:
	jq ti.ParseInp

push_return_address:
	call ti.NewLine
	ld hl, pushing_return_address_str
	call ti.PutS
	ld hl, 0
returnaddr := $-3
	push hl
	ld hl, (return)
	jp (hl)

set_up_reloader:
	;attempt to copy the reloader to the stack and set up a return address
	;not sure how to do negative numbers but maybe this would work?
	;should end up with -(reloader_end - reloader_start)
	call ti.NewLine
	ld hl, making_reloader_str
	call ti.PutS
	ld hl, reloader_start - reloader_end
	add hl, sp
	ld sp, hl
	ld (returnaddr), hl
	ex de, hl
	ld hl, reloader_start
	ld bc, reloader_end - reloader_start
	ldir
	ld hl, reloader_error - reloader_start
	add hl, sp
	call ti.PushErrorHandler
	ld hl, (return)
	jp (hl)

reloader_start:
	call ti.PopErrorHandler
reloader_error:
	ld hl, ti.userMem
	ld de, (ti.asm_prgm_size)
	call ti.DelMem
   	di
   	res ti.progExecuting, (iy + ti.newDispF)
   	res ti.cmdExec, (iy + ti.cmdFlags)
   	bit ti.onInterrupt, (iy + ti.onFlags)
   	jp nz, reloader_return_asm.error_on
   	ld hl, 0
tmp0 := $-3 
   	ld (ti.OP1), hl
   	ld hl, 0
tmp1 := $-3
   	ld (ti.OP1 + 3), hl
   	ld hl, 0
tmp2 := $-3
   	ld (ti.OP1 + 6), hl
   	res 0, (iy + ti.asm_Flag2)
	jr run_asm_program_no_reloader

;this is here so that we can have it be the reloader as well, for optimization
run_asm_program:
	ld hl, run_program_asm_str
	call ti.HomeUp
	call ti.PutS
	call clear_usermem
	;do this in any case
	;do this as well
	ld hl, ti.appData
	ld (hl), 0
	ld hl, use_reloader
	bit 0, (hl)
	jr z, reloader_return_asm
	ld hl, reloader_return_asm
	ld (return), hl
	jp set_up_reloader
run_asm_program_no_reloader:
	ld hl, ti.appData
	ld (hl), 1
reloader_return_asm:
	ld iy, ti.flags
.getprgm:
   	call ti.ChkFindSym
   	jr c, .error_not_found
   	call ti.ChkInRam
   	ex de, hl
   	jr z, .in_ram
   	ld bc, 9
   	add hl, bc
   	ld c, (hl)
   	add hl, bc
   	inc hl
.in_ram:
   	call ti.LoadDEInd_s
   	bit 0, (iy + ti.asm_Flag2)
   	jr z, .getsize
   	push de
   	pop bc
   	inc hl
   	inc hl
   	ld  de, ti.userMem
   	ldir
   	call ti.DisableAPD
	;if we don't have to use the reloader we should just run the program
	ld a, (ti.appData)
	cp a, 1
	jr nz, .clear_reloader
	;ld hl, 0
use_reloader := $-1
	;bit 0, hl
	;jr nz, .return_push_return_address_asm
	;if we've returned we should clear the reloader
	;jr .return_push_return_address_asm
	;otherwise we should push the return address and then come back
	ld hl, .return_push_return_address_asm
	ld (return), hl
	jp push_return_address
.clear_reloader:
	di
   	ld hl, reloader_end - reloader_start
   	add hl, sp
   	ld sp, hl
.return_push_return_address_asm:
	;again
	ld hl, ti.appData
	bit 0, (hl)
   	call z, ti.userMem
	;otherwise
	jp ti.userMem
.getsize:
   	push de
   	call ti.EnoughMem
   	pop hl
   	jr c, .error_mem
   	push hl
   	ld de, ti.userMem
   	call ti.InsertMem
   	pop bc
   	ld (ti.asm_prgm_size), bc
   	set 0, (iy + ti.asm_Flag2)
   	jr reloader_return_asm.getprgm
.error_mem:
.error_not_found:
.error_on:
   	di
   	ld hl, reloader_end - reloader_start
   	add hl, sp
   	ld sp, hl
   	ret

;some variables
is_asm:
	db 0
return:
	db 0
	db 0
	db 0
	
run_program_asm_str:
	db "Running asm...", 0
run_program_basic_str:
	db "Running basic...", 0
making_reloader_str:
	db "Making reloader...", 0
pushing_return_address_str:
	db "Pushing return address...", 0

reloader_end:
	load code: $-run_program from run_program
	end virtual
	db code
run_program_end: