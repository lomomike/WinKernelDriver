// DriverStarter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#define UNICODE
#define _UNICODE

#include <windows.h>

#define DEVICE_NAME L"\\\\.\\Testdrv"
#define IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define DRIVER_NAME TEXT("testdrv")
#define DRIVER_FILENAME TEXT("C:\\Users\\Administrator\\Desktop\\Driver2\\TestDriver.sys")


const char message[] = "Greetings from user mode";

void debug(char *text) {
	LPTSTR msgbuf;

	//get error message string for last error
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), LANG_USER_DEFAULT,
		(LPTSTR)&msgbuf, 0, NULL);

	printf("%s: %S\n", text, msgbuf);
}

void CommunicateWithDriver()
{
	HANDLE hDevice;
	ULONG outBuf;
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
			IOCTL_CODE, 
			(LPVOID)message, 
			sizeof(message),
			&outBuf, 
			sizeof(ULONG), 
			&returned, 
			NULL)) 
		{
			debug("ioctl error");
		}
		else
		{
			char output[256];
			sprintf(output, "returner %u bytes, result from driver: %x", returned, outBuf);
			debug(output);
		}		

		CloseHandle(hDevice);
	}
}

int wmain(int argc, wchar_t* argv[])
{
	auto hSCMAnager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
	printf("Load driver...\n");
	if(hSCMAnager)
	{
		printf("Create service...\n");
		wprintf(TEXT("Driver path: %s\n"), DRIVER_FILENAME);

		auto hService = CreateService(
			hSCMAnager,
			DRIVER_NAME,
			DRIVER_NAME,
			SERVICE_START | DELETE | SERVICE_STOP,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_IGNORE,
			DRIVER_FILENAME,
			nullptr, nullptr, nullptr, nullptr, nullptr);
		debug("Create service");

		if (!hService)
		{
			hService = OpenService(hSCMAnager, DRIVER_NAME, SERVICE_START | DELETE);
		}

		if (hService)
		{
			printf("Start service...\n");

			StartService(hService, 0, nullptr);
			debug("Starting service");

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
