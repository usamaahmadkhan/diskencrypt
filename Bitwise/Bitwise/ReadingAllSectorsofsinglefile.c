//// Needs DDK installed
//// Compile: cl.exe test.cpp /IC:\WinDDK\3790.1830\inc\wxp /IC:\WinDDK\3790.1830\inc\crt
//#define _WIN32_WINNT 0x0500 
//#include <windows.h>
//#include <winioctl.h>
//#include <stdio.h>
//#include <ntddvol.h>
//
//char *ShowError(void)
//{
//	static char szReturn[128] = { 0 };
//	char *lpMsgBuf;
//	FormatMessage(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//		NULL,
//		GetLastError(),
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPTSTR)&lpMsgBuf,
//		0,
//		NULL
//		);
//	memset(szReturn, 0, sizeof szReturn);
//	strcpy(szReturn, lpMsgBuf);
//	LocalFree(lpMsgBuf);
//	return (char *)szReturn;
//}
//
//int main(void)
//{
//	CHAR szDrive[] = { "G:\\" };
//	CHAR szVolumeHandleName[] = { "\\\\.\\G:" };
//	CHAR szFileName[] = { "G:\\Test Folder\\Testing.txt" };
//	CHAR szVolumeName[128] = { 0 };
//	CHAR szFileSystemName[32] = { 0 };
//	INT iExtentsBufferSize = 1024;
//	DWORD dwBytesReturned;
//	DWORD dwSectorsPerCluster;
//	DWORD dwBytesPerSector;
//	DWORD dwNumberFreeClusters;
//	DWORD dwTotalNumberClusters;
//	DWORD dwVolumeSerialNumber;
//	DWORD dwMaxFileNameLength;
//	DWORD dwFileSystemFlags;
//	DWORD dwClusterSizeInBytes;
//
//	STARTING_VCN_INPUT_BUFFER StartingPointInputBuffer;
//	LARGE_INTEGER FileOffstFromBegDisk;
//	VOLUME_LOGICAL_OFFSET VolumeLogicalOffset;
//	LARGE_INTEGER  PhysicalOffsetReturnValue;
//	VOLUME_PHYSICAL_OFFSETS VolumePhysicalOffsets;
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
//	// Buffer to hold the extents info
//	PRETRIEVAL_POINTERS_BUFFER lpRetrievalPointersBuffer =
//		(PRETRIEVAL_POINTERS_BUFFER)malloc(iExtentsBufferSize);
//
//	//StartingVcn field is the virtual cluster number in file
//	// It is not a file offset
//
//	// FSCTL_GET_RETRIEVAL_POINTERS acquires a list of virtual clusters from the
//	// file system driver that make up the file and the related set of
//	// Logical Clusters that represent the volume storage for these Virtual
//	// Clusters.  On a heavliy fragmented volume, the file may not necessarily
//	// be stored in contiguous storage locations.  Thus, it would be advisable
//	// to follow the mapping of virtual to logical clusters "chain" to find
//	// the complete physical layout of the file.
//
//	// We want to start at the first virtual cluster ZERO   
//	StartingPointInputBuffer.StartingVcn.QuadPart = 0;
//	if (!DeviceIoControl(
//		hFile,
//		FSCTL_GET_RETRIEVAL_POINTERS,
//		&StartingPointInputBuffer,
//		sizeof(STARTING_VCN_INPUT_BUFFER),
//		lpRetrievalPointersBuffer,
//		iExtentsBufferSize,
//		&dwBytesReturned,
//		NULL))
//	{
//		printf("%i\n", GetLastError());
//		return -1;
//	}
//	CloseHandle(hFile);
//
//	if (!GetDiskFreeSpace(
//		szDrive,
//		&dwSectorsPerCluster,
//		&dwBytesPerSector,
//		&dwNumberFreeClusters,
//		&dwTotalNumberClusters))
//	{
//		return -1;
//	}
//	dwClusterSizeInBytes = dwSectorsPerCluster * dwBytesPerSector;
//
//	if (!GetVolumeInformation(
//		szDrive,
//		szVolumeName,
//		128,
//		&dwVolumeSerialNumber,
//		&dwMaxFileNameLength,
//		&dwFileSystemFlags,
//		szFileSystemName,
//		32))
//		return -1;
//
//	HANDLE volumeHandle = CreateFile(
//		szVolumeHandleName,
//		GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ | FILE_SHARE_WRITE,
//		NULL,
//		OPEN_EXISTING,
//		FILE_ATTRIBUTE_NORMAL,
//		NULL);
//
//	if (volumeHandle == INVALID_HANDLE_VALUE)
//		return -1;
//
//	else if (strcmp(szFileSystemName, "NTFS") == 0)
//	{
//		VolumeLogicalOffset.LogicalOffset = lpRetrievalPointersBuffer->Extents[0].Lcn.QuadPart * dwClusterSizeInBytes;
//		if (!DeviceIoControl(
//			volumeHandle,
//			IOCTL_VOLUME_LOGICAL_TO_PHYSICAL,
//			&VolumeLogicalOffset,
//			sizeof(VOLUME_LOGICAL_OFFSET),
//			&VolumePhysicalOffsets,
//			sizeof(VOLUME_PHYSICAL_OFFSETS),
//			&dwBytesReturned,
//			NULL))
//		{
//			CloseHandle(volumeHandle);
//			return -1;
//		}
//		CloseHandle(volumeHandle);
//		PhysicalOffsetReturnValue.QuadPart = 0;
//		PhysicalOffsetReturnValue.QuadPart += VolumePhysicalOffsets.PhysicalOffset[0].Offset;
//		if (PhysicalOffsetReturnValue.QuadPart != -1)
//		{
//			printf("%s starts at 0x%x%x from beginning of the disk\n\n",
//				szFileName,
//				PhysicalOffsetReturnValue.HighPart,
//				PhysicalOffsetReturnValue.LowPart);
//			return 0;
//		}
//	}
//	else
//	{
//		printf("%s File system NOT supported\n", szFileSystemName);
//		return  -1;
//	}
//
//
//	getch();
//	return 0;
//}