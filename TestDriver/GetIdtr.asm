.code
__getIdtr PROC
	sidt [RCX]
	ret
__getIdtr ENDP

__getGdtr PROC
	sgdt [RCX]
	ret
__getGdtr ENDP

__getCr0 PROC
	mov RAX, cr0
	ret
__getCr0 ENDP

__getCr2 PROC
	mov RAX, cr2
	ret
__getCr2 ENDP

__getCr3 PROC
	mov RAX, cr3
	ret
__getCr3 ENDP

END