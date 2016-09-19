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

void PrintGate(int index, PGATE gate)
{
	printf("Gate info [%d]:\n", index);
	printf("\tOffset 1:%lx\n", gate->Offset1);
	printf("\tSegment selector:%lx\n", gate->SegmentSelector);
	printf("\tData: %lx\n", gate->Data);
	printf("\tOffset2: %lx\n", gate->Offset2);
	printf("\tOffset3: %lx\n", gate->Offset3);
	printf("\Reserved: %lx\n", gate->Reserved);
	printf("\n");	
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
			printf("CPU info\n");
			printf("\tIDTR addr %llx, limit %d\n", info.Idtr.addr, info.Idtr.limit);
			printf("\tGDTR addr %llx, limit %d\n", info.Gdtr.addr, info.Gdtr.limit);
			printf("\tCR0 %llx, CR2 %llx, CR3 %llx\n", info.cr0, info.cr2, info.cr3);

			for(int index = 0; index < 5; ++index)
			{
				PrintGate(index, &info.Gates[index]);
			}
		}		

		CloseHandle(hDevice);
	}
}

int wmain(int argc, wchar_t* argv[])
{
	TCHAR currentDirectory[MAX_PATH], driverFullPath[MAX_PATH];
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
	}	

	return 0;
}

