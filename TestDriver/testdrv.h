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
typedef struct _FLAGS
{
	unsigned char IST : 3;
	unsigned char : 5;
	unsigned char Type : 4;
	unsigned char : 1;
	unsigned char DPL : 2;
	unsigned char P : 1;	
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
} CPU_INFO, * PCPU_INFO;
#pragma pack()

#define CPU_INFO_ sizeof(CPU_INFO)

#define GATE_ sizeof(GATE)

typedef struct _MEMORY_DUMP_IN
{
	PVOID Address;
	UINT64 Size;
} MEMORY_DUMP_IN, * PMEMORY_DUMP_IN;

#define MEMORY_DUMP_IN_ sizeof(MEMORY_DUMP_IN)

#define IOCTL_GET_CPU_INFO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MEMORY_CONTENT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

