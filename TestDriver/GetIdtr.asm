.code
__getIdtr PROC
	sidt [RCX]
	ret
__getIdtr ENDP

__getGdtr PROC
	sgdt [RCX]
	ret
__getGdtr ENDP
END