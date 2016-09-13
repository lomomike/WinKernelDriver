.data
myData dt 0

.code
GetIdtr PROC
	cli
	sidt myData
	sidt [RCX]
	sti
	ret
GetIdtr ENDP
END