include 'include/ez80.inc'
include 'include/tiformat.inc'
include 'include/ti84pceg.inc'

;include 'include/hook_equates.inc'
;include 'include/capnhook.inc'

;thanks Cesium
;https://github.com/mateoconlechuga/cesium/blob/bfbddaab369c8bc6b0f39dc82ac1afa8bddc22e4/src/hooks.asm#L29

hook_token_stop := $d9 - $ce
hook_parser:
	db	$83			; hook signifier
	push	af
	cp	a,2
	jq	z,.maybe_stop
.chain:
	ld	a,(ti.appErr2)
	cp	a,$7f
	jq	nz,.no_chain
	pop	af
	ld	ix,(ti.appErr2 + 1)
	jp	(ix)
.no_chain:
	pop	af
	xor	a,a
	ret
.maybe_stop:
	ld	a,hook_token_stop	; check if stop token
	cp	a,b
	jq	nz,.chain
.stop:
	pop	af
	ld	a,ti.E_AppErr1
	jq	ti.JError
hook_parser_size := $-hook_parser

public _vysion_asm_InstallFixStopHook
_vysion_asm_InstallFixStopHook:
;indentations are now fixed
;why is it jp and not call?
;NOTE: call/ret is the same thing as jp (to remember for later)
	ld	hl,hook_parser
	ld	de,ti.appData
	ld	bc,hook_parser_size
	ldir
	ld	hl,ti.appData
	jq	ti.SetParserHook


public _vysion_asm_ClearFixStopHook
_vysion_asm_ClearFixStopHook:
	bit	ti.parserHookActive,(iy + ti.hookflags4)
	jq	ti.ClrParserHook