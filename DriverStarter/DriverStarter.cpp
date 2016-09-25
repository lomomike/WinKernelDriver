// DriverStarter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <windows.h>
#include "testdrv.h"

#define DEVICE_NAME L"\\\\.\\Testdrv"

#define DRIVER_NAME TEXT("testdrv")
#define DRIVER_FILENAME TEXT("TestDriver.sys")

const char message[] = "Greetings from user mode";

void debug(char *text) {
	LPTSTR msgbuf;

	//get error message string for last error
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), LANG_ENGLISH,
		(LPTSTR)&msgbuf, 0, NULL);

	printf("%s: %S\n", text, msgbuf);
}

ULONG64 inline MakeProcedureAddress(PGATE gate)
{
	return static_cast<ULONG64>(gate->Offset3) << 32 | 
		   static_cast<ULONG64>(gate->Offset2) << 16 | 
		   static_cast<ULONG64>(gate->Offset1);
}

const char * DecodeType(unsigned char type)
{
	
	switch (type)
	{
	case 0: return "Upper 8 bytes of an 14-byte descriptor";
	case 2: return "LDT";
	case 9: return "64-bit TSS (Available)";
	case 11: return "64-bit TSS (Busy)";
	case 12: return "64-bit Call Gate";
	case 14: return "64-bit Interrupt Gate";
	case 15: return "64-bit Trap Gate";
	default: return "Reserved";
	}
}

void PrintGate(int index, PGATE gate)
{
	printf("Gate info [%d]:\n", index);
	printf("\tAddress:%llx\n", MakeProcedureAddress(gate));
	printf("\tSegment selector:%lx\n", gate->SegmentSelector);
	printf("\tIST: %lx, Type: %s (%c), DPL: %lx, P: %lx\n", 
		gate->Flags.IST, 
		DecodeType(gate->Flags.Type), 
		gate->Flags.Type,
		gate->Flags.DPL, 
		gate->Flags.P);
	printf("\n");	
}

void PrintCpuInfo(PCPU_INFO info)
{
	if (info == nullptr)
		return;

	printf("CPU info\n");
	printf("\tIDTR addr %llx, limit %d\n", info->Idtr.addr, info->Idtr.limit);
	printf("\tGDTR addr %llx, limit %d\n", info->Gdtr.addr, info->Gdtr.limit);
	printf("\tCR0 %llx, CR2 %llx, CR3 %llx\n", info->cr0, info->cr2, info->cr3);
}

VOID GetIdtEntries(HANDLE hDevice, PDESCRIPTOR idt)
{
	if (idt == nullptr || idt->addr == 0)
		return;

	unsigned long size = idt->limit + 1;

	LPVOID outputBuffer = static_cast<LPVOID>(malloc(size));
	if (outputBuffer == nullptr)
		return;

	MEMORY_DUMP_IN input;
	input.Address = reinterpret_cast<PVOID>(idt->addr);
	input.Size = size;

	DWORD returned;
	auto ok = DeviceIoControl(hDevice, IOCTL_GET_MEMORY_CONTENT, &input, MEMORY_DUMP_IN_,
		outputBuffer, size, &returned, nullptr);

	if(!ok)
	{
		debug("ioctl call");
		free(outputBuffer);
		return;
	}

	if (returned != size)
	{
		printf("returned %d, size %d\n", returned, size);
		debug("Incorrect size returned");
		free(outputBuffer);
		return;
	}

	PGATE gates = reinterpret_cast<PGATE>(outputBuffer);
	auto count = size / GATE_;

	for(int index = 0; index < count; ++ index)
	{
		GATE gate = gates[index];

		PrintGate(index, &gate);
	}

	free(outputBuffer);
}

void CommunicateWithDriver()
{
	HANDLE hDevice;
	CPU_INFO info;
	DWORD returned;

	hDevice = CreateFile(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
	                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
	                                FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		debug("error opening device");
	}
	else {
		if (0 == DeviceIoControl(
			hDevice, 
			IOCTL_GET_CPU_INFO, 
			nullptr, 
			0,
			&info, 
			CPU_INFO_, 
			&returned, 
			nullptr)) 
		{
			debug("ioctl error");
		}
		else
		{

			PrintCpuInfo(&info);
			
			GetIdtEntries(hDevice, &info.Idtr);
		}		

		CloseHandle(hDevice);
	}
}

int wmain(int argc, wchar_t* argv[])
{

	printf("%d\n", (int) sizeof(FLAGS));
	FLAGS f = { 0 };
	WORD data = 0x8e00;
	memcpy(&f, &data, sizeof(FLAGS));

	

	/*TCHAR currentDirectory[MAX_PATH], driverFullPath[MAX_PATH];
	GetCurrentDirectory(sizeof(TCHAR) * MAX_PATH, currentDirectory);
	swprintf_s(driverFullPath, TEXT("%s\\%s"), currentDirectory, DRIVER_FILENAME);
	
	auto hSCMAnager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
	wprintf(TEXT("Loading driver \"%s\"\n"), driverFullPath);
	if(hSCMAnager)
	{
		auto hService = CreateService(
			hSCMAnager,
			DRIVER_NAME,
			DRIVER_NAME,
			SERVICE_START | DELETE | SERVICE_STOP,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_IGNORE,
			driverFullPath,
			nullptr, nullptr, nullptr, nullptr, nullptr);
		
		if (!hService)
		{
			hService = OpenService(hSCMAnager, DRIVER_NAME, SERVICE_START | DELETE);
		}

		if (hService)
		{
			StartService(hService, 0, nullptr);
			
			CommunicateWithDriver();

			SERVICE_STATUS ss;
			ControlService(hService, SERVICE_CONTROL_STOP, &ss);

			DeleteService(hService);
			CloseServiceHandle(hService);			
		}
		else
		{
			debug("error occured");
		}

		CloseServiceHandle(hSCMAnager);
	}
	else
	{
		debug("Error occured");
	}*/

	return 0;
}

