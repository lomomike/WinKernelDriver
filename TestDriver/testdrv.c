#include <ntddk.h>
#include "testdrv.h"

#define DEVICE_NAME L"\\Device\\Testdrv"
#define DOS_DEVICE_NAME L"\\DosDevices\\Testdrv"

//macros for OACR
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD TestdrvUnload;
__drv_dispatchType(IRP_MJ_CREATE)
__drv_dispatchType(IRP_MJ_CLOSE)
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH TestdrvDispatch;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, TestdrvUnload)
#pragma alloc_text(PAGE, TestdrvDispatch)
#endif // ALOC_PRAGMA

extern void __fastcall __getIdtr(PDESCRIPTOR pIdtr);
extern void __fastcall __getGdtr(PDESCRIPTOR pIdtr);
extern X64_REGISTER __fastcall __getCr0();
extern X64_REGISTER __fastcall __getCr2();
extern X64_REGISTER __fastcall __getCr3();


void DebugInfo(char *str)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "testdrv: %s\n", str);
}

NTSTATUS GetCpuInfo(PVOID outBuffer, ULONG outBufferLength)
{
	if (outBufferLength < CPU_INFO_)
	{
		RtlZeroMemory((void *)outBufferLength, outBufferLength);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	CPU_INFO info;

	__getIdtr(&info.Idtr);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "testdrv: IDTR addr %llx, limit %d\n", info.Idtr.addr, info.Idtr.limit);

	__getGdtr(&info.Gdtr);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "testdrv: GDTR addr %llx, limit %d\n", info.Gdtr.addr, info.Gdtr.limit);

	info.cr0 = __getCr0();
	info.cr2 = __getCr2();
	info.cr3 = __getCr3();

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "testdrv: CR0 %llx, CR2 %llx, CR3 %llx\n", info.cr0, info.cr2, info.cr3);

	RtlCopyMemory(outBuffer, &info, outBufferLength);
	return STATUS_SUCCESS;
}

NTSTATUS TestdrvDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION iostack;
	NTSTATUS status = STATUS_NOT_SUPPORTED;
	PCHAR buf;
	ULONG len;

	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

	iostack = IoGetCurrentIrpStackLocation(Irp);
	Irp->IoStatus.Information = 0;
	ULONG ioctlCode = iostack->Parameters.DeviceIoControl.IoControlCode;

	switch (iostack->MajorFunction)
	{
		case IRP_MJ_CREATE:
		case IRP_MJ_CLOSE:
			status = STATUS_SUCCESS;
			break;
		case IRP_MJ_DEVICE_CONTROL:
			if (ioctlCode == IOCTL_TESTDRV)
			{
				len = iostack->Parameters.DeviceIoControl.InputBufferLength;
				buf = (PCHAR)Irp->AssociatedIrp.SystemBuffer;
				
				//verify null-terminated and print string to debugger
				if (buf[len - 1] == '\0') {
					DebugInfo(buf);
				}

				ULONG outputLength = iostack->Parameters.DeviceIoControl.OutputBufferLength;
				PULONG outputBufer = (PULONG)Irp->AssociatedIrp.SystemBuffer;
				if (outputLength == sizeof(ULONG))
				{
					RtlZeroMemory(outputBufer, sizeof(ULONG));
					*outputBufer = (ULONG)42;
					Irp->IoStatus.Information = sizeof(ULONG);					
					status = STATUS_SUCCESS;
				}
				else
				{
					status = STATUS_INVALID_PARAMETER;
				}
				
			}
			else if (ioctlCode == IOCTL_GET_CPU_INFO)
			{
				ULONG outputLength = iostack->Parameters.DeviceIoControl.OutputBufferLength;
				PVOID outputBufer = Irp->AssociatedIrp.SystemBuffer;
				status = GetCpuInfo(outputBufer, outputLength);
				if (NT_SUCCESS(status))
				{
					Irp->IoStatus.Information = CPU_INFO_;
				}
			}
			else
			{
				status = STATUS_INVALID_PARAMETER;
			}
			break;			
		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
	}

	Irp->IoStatus.Status = status;
	
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

VOID TestdrvUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING dosdev;

	PAGED_CODE();

	DebugInfo("driver unloading");

	RtlInitUnicodeString(&dosdev, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&dosdev);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING devname, dosname;
	PDEVICE_OBJECT devobj;
	NTSTATUS status;

	UNREFERENCED_PARAMETER(RegistryPath);

	DebugInfo("driver initializing");

	DriverObject->MajorFunction[IRP_MJ_CREATE] = TestdrvDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = TestdrvDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TestdrvDispatch;
	DriverObject->DriverUnload = TestdrvUnload;

	RtlInitUnicodeString(&devname, DEVICE_NAME);
	RtlInitUnicodeString(&dosname, DOS_DEVICE_NAME);

	status = IoCreateDevice(DriverObject, 0, &devname, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &devobj);

	if(!NT_SUCCESS(status))
	{
		DebugInfo("error creating device");
		return status;
	}

	status = IoCreateSymbolicLink(&dosname, &devname);

	if (!NT_SUCCESS(status))
	{
		DebugInfo("error creating symbolic link");
		return status;
	}

	return STATUS_SUCCESS;
}

