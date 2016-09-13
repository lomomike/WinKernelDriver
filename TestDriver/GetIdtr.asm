.code
GetIdtr PROC
	cli
	sidt [RCX]
	sti
	ret
GetIdtr ENDP
END