//#include <windows.h>
//#include <stdio.h>
//#include "xts.h"
//
//
//
//int main()
//{
//
//	int sector = 0;
//	char *buff[512];
//	ReadSector(sector, buff);
//
//	HANDLE hFile;
//
//	hFile = CreateFile(
//		L"\\\\.\\G:",
//		GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ | FILE_SHARE_WRITE,
//		NULL,
//		OPEN_EXISTING,
//		0,
//		NULL);
//	if (INVALID_HANDLE_VALUE == hFile)
//		return -1;
//
//	STORAGE_READ_CAPACITY sc;
//	if (0 <= NtDeviceIoControlFile(hFile, 0, 0, 0, &iosb, IOCTL_STORAGE_READ_CAPACITY, 0, 0, &sc, sizeof(sc)))
//	{
//		DbgPrint("---- IOCTL_STORAGE_READ_CAPACITY ----\n");
//		DbgPrint("%I64x %I64x %x \n", sc.DiskLength.QuadPart, sc.NumberOfBlocks.QuadPart, sc.BlockLength);
//		sc.NumberOfBlocks.QuadPart *= sc.BlockLength;
//		DbgPrint("%I64x %I64u\n", sc.NumberOfBlocks.QuadPart, sc.NumberOfBlocks.QuadPart);
//	}
//
//
//	getch();
//
//}
//
//
//
//
//
//
//uint_8 SectorsPerCluster = 0;
//uint BytesPerSector = 0;
//uint NumberOfFreeClusters = 0;
//uint TotalNumberOfClusters = 0;
//
//GetDiskFreeSpace(
//	Drive,
//	out SectorsPerCluster,
//	out BytesPerSector,
//	out NumberOfFreeClusters,
//	out TotalNumberOfClusters
//	);