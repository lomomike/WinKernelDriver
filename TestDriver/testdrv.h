#pragma once

#include <ntddk.h>

#pragma pack(1)
typedef struct _DESCRIPTOR {
	UINT16 limit;
	UINT64 addr;
} DESCRIPTOR, *PDESCRIPTOR;

typedef struct _CPU_INFO {
	DESCRIPTOR Idtr;
	DESCRIPTOR Gdtr;
} CPU_INFO, * PCCPU_INFO;
#pragma pack()

#define CPU_INFO_ sizeof(CPU_INFO)


extern void __fastcall __getIdtr(PDESCRIPTOR pIdtr);
extern void __fastcall __getGdtr(PDESCRIPTOR pIdtr);
