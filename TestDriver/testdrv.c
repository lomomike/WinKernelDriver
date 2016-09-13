#include <ntddk.h>

#define DEVICE_NAME L"\\Device\\Testdrv"
#define DOS_DEVICE_NAME L"\\DosDevices\\Testdrv"

#define IOCTL_TESTDRV CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

//macros for OACR
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD TestdrvUnload;
__drv_dispatchType(IRP_MJ_CREATE)
__drv_dispatchType(IRP_MJ_CLOSE)
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH TestdrvDispatch;

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, TestdrvUnload)
#pragma alloc_text(PAGE, TestdrvDispatch)

#pragma pack(1)
typedef struct _IDTR {
	UINT16 limit;
	UINT64 addr;
} IDTR, * PIDTR;
#pragma pack()

extern void __fastcall GetIdtr(PIDTR pIdtr);


void DebugInfo(char *str)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "testdrv: %s\n", str);
}

IDTR GetIDT()
{
	IDTR idtr;
	GetIdtr(&idtr);	
	return idtr;
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

	switch (iostack->MajorFunction)
	{
		case IRP_MJ_CREATE:
		case IRP_MJ_CLOSE:
			status = STATUS_SUCCESS;
			break;
		case IRP_MJ_DEVICE_CONTROL:
			if (iostack->Parameters.DeviceIoControl.IoControlCode == IOCTL_TESTDRV)
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

	IDTR idtr = GetIDT();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "testdrv: IDTR addr %x, limit %d\n", idtr.addr, idtr.limit);

	return STATUS_SUCCESS;
}

