#pragma once

typedef ULONG64 X64_REGISTER, *PX64_REGISTER;

#define GATES_COUNT 5

#pragma pack(1)
typedef struct _DESCRIPTOR {
	UINT16 limit;
	UINT64 addr;
} DESCRIPTOR, *PDESCRIPTOR;

#pragma warning( push )
#pragma warning( disable: 4214)
typedef struct FLAGS_
{
	char IST : 3;
	char Reserved1 : 5;
	char Type : 4;
	char Reserved2 : 1;
	char DPL : 2;
	char P : 1;	
} FLAGS;
#pragma warning(pop)

typedef struct _GATE {
	UINT16 Offset1;
	UINT16 SegmentSelector;
	FLAGS Flags;
	UINT16 Offset2;
	UINT32 Offset3;
	UINT32 Reserved;
} GATE, * PGATE;

typedef struct _CPU_INFO {
	DESCRIPTOR Idtr;
	DESCRIPTOR Gdtr;
	X64_REGISTER cr0;
	X64_REGISTER cr2;
	X64_REGISTER cr3;
	GATE Gates[GATES_COUNT];
} CPU_INFO, * PCCPU_INFO;
#pragma pack()

#define CPU_INFO_ sizeof(CPU_INFO)

#define GATE_ sizeof(GATE)

#define IOCTL_TESTDRV CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_CPU_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

