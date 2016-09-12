// DriverStarter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#define UNICODE
#define _UNICODE

#include <windows.h>

#define DEVICE_NAME L"\\\\.\\Testdrv"
#define IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)


const char message[] = "Greetings from user mode";

void debug(char *text) {
	LPTSTR msgbuf;

	//get error message string for last error
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), LANG_USER_DEFAULT,
		(LPTSTR)&msgbuf, 0, NULL);

	printf("%s: %S\n", text, msgbuf);
}

int wmain(int argc, wchar_t* argv[])
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

	return 0;
}

