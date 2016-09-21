#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "xts.h"

#define KEY "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
#define SECTOR_SIZE 512
#define READ_OPERATION 'r'
#define WRITE_OPERATION 'w'

typedef char IO_OPERATION_TYPE;

typedef struct{
	int sector_start;
	int sector_end;
    xts_encrypt_ctx ctx;
} TARGS, *PTARGS;

HANDLE getDeviceHandle(wchar_t* partition, IO_OPERATION_TYPE mode)
{
	HANDLE device;
	int retCode = 1;

	if (mode == READ_OPERATION)
	{
		device = CreateFile(
			partition,									// Partition to open
			GENERIC_READ,								// Access mode
			FILE_SHARE_READ | FILE_SHARE_WRITE,			// Share Mode
			NULL,										// Security Descriptor
			OPEN_EXISTING,								// How to create
			0,											// File attributes
			NULL);										// Handle to template
	}
	else if(mode == WRITE_OPERATION)
	{
		device = CreateFile(
			partition,									// Partition to open
			GENERIC_READ | GENERIC_WRITE,				// Access mode
			FILE_SHARE_READ | FILE_SHARE_WRITE,			// Share Mode
			NULL,										// Security Descriptor
			OPEN_EXISTING,								// How to create
			0,											// File attributes
			NULL);										// Handle to template

	}

	if (device == INVALID_HANDLE_VALUE)
		retCode = -1;
	 
		if(retCode == 1)
			return device;
		else
			return NULL;


}

int getNoOfSectors(HANDLE device)
	{
		PARTITION_INFORMATION pi;
		DWORD bytesReturnedSize;

		if (DeviceIoControl(
			device,										// handle to device of interest
			IOCTL_DISK_GET_PARTITION_INFO,				// control code of operation to perform
			NULL,										// pointer to buffer to supply input data
			0,											// size of input buffer
			&pi,										// pointer to buffer to receive output data
			sizeof(PARTITION_INFORMATION),				// size of output buffer
			&bytesReturnedSize,							// pointer to variable to receive output byte count
			NULL										// pointer to overlapped structure for asynchronous operation
			))
		{
			//printf("%i\n", pi.PartitionLength);
			return pi.PartitionLength.QuadPart / 512;
		}
		else
		{
			printf("%i\n",GetLastError());
			return -1;
		}
	}

int ReadSector(HANDLE device, BYTE* bufferForBytesTobeRead, DWORD size, int sectorNo )
{
	char buffForPrint[512] = { 0 };
	int retCode = 0;
	DWORD bytesRead;
	int NoOfSectorsOnPartition=0;


	if (NULL == device)
	{
		printf("INVALID HANDLE\n");
		return 0;
	}
	else
	{
		int ret = getNoOfSectors(device);

		if (-1 != ret)
		{
			NoOfSectorsOnPartition = ret;

			if (sectorNo > NoOfSectorsOnPartition)
			{
				printf("Selected sector out of range");
				retCode = -1;
				return retCode;

			}
			else
			{

				SetFilePointer(device, sectorNo * 512, NULL, FILE_BEGIN);

				if (!ReadFile(device, bufferForBytesTobeRead, 512, &bytesRead, NULL))
				{
					printf("Error in reading\n");
				}
				else
				{
					//printf("Sector Accessed\n");

					/*for (int i = 0; i < size; i++)
					{
						printf("%.02x ",bufferForBytesTobeRead[i]);
					}*/
					retCode = 1;
				}
			}
		}
	}

	return retCode;
}

int WriteSector(HANDLE device ,BYTE* bytesToWrite, DWORD size, int sectorNo )
{
	char buffForPrint[512] = { 0 };
	int Code = 0;
	DWORD status;
	int NoOfSectorsOnPartition = 0;
	DWORD bytesReturnedSize = 0;



	if (NULL == device)
	{
		printf("Exiting from WriteSector\n");
		return 0;
	}
	else
	{
		int ret = getNoOfSectors(device, bytesReturnedSize);

		if (-1 != ret)
		{
			NoOfSectorsOnPartition = ret;
			
			if (sectorNo > NoOfSectorsOnPartition)
			{
				printf("Selected sector out of range");
				Code = -1;
				return Code;

			}else
			{
				if (0 != DeviceIoControl(device, FSCTL_IS_VOLUME_MOUNTED, NULL, 0, NULL, 0, &status, NULL))
				{
					if (!DeviceIoControl(device, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &status, NULL))  // lock volume
					{
						// error handling; not sure if retrying is useful
						printf("Error in Dismounting Volume: %i", GetLastError());
					}
				}
				

					SetFilePointer(device, sectorNo * 512, NULL, FILE_BEGIN);

					if (!WriteFile(device, bytesToWrite, size, &status, NULL))
					{
						printf("Error in writing.Error Code: %i\n", GetLastError());
						Code = -1;
						return Code;
					}
					else
					{
						//printf("Sector Written\n");
						Code = 1;
					}
				
			}
		}
	}
	return Code;
}

int getNoOfMaxThreads( const int totalSectors)
{
	if (totalSectors % 64 == 0)
		return 64;
	if (totalSectors % 32 == 0)
		return 32;
	if (totalSectors % 16 == 0)
		return 16;
	if (totalSectors % 8 == 0)
		return 8;
	if (totalSectors % 4 == 0)
		return 4;
	else
		return 2;
}

DWORD WINAPI EncryptSectorBlock(LPVOID lpParam)
{
	PTARGS args = (PTARGS)lpParam;
	static unsigned char buffer[SECTOR_SIZE];

	for (int i = args->sector_start; i <= args->sector_end; i++)
	{
/*
		if (ReadSector(args->read, buffer, SECTOR_SIZE, i) != 1)
			printf("Error reading sector %i\n", i);

		xts_encrypt_sector(buffer, i, SECTOR_SIZE, &(args->ctx));

		if (WriteSector(args->write, buffer, SECTOR_SIZE, i) != 1)
			printf("Error writing sector %i\n", i);
*/
		//init to zero every time in case a sector is failed to be read or write, it can be recover from fault 
		//This may break in future.. Not proper mechanism to recover from fault. just a hack.
		 memset(buffer, 0, SECTOR_SIZE);
	}
}


INT_RETURN EncryptFullVolume(wchar_t* volume, unsigned char* key)
{					
					int TotalNoOfSectors;
					int MAX_Threads;
					int sectors_per_thread;
					int sector_offset;
					
					xts_encrypt_ctx ctx;     //used for context of the crypt

					INT_RETURN ret = EXIT_SUCCESS;   //hoping to pass through with success

	//maintain handles for READ/WRITE operations
	HANDLE write_hand = getDeviceHandle(volume, WRITE_OPERATION) ;
	HANDLE read_hand = getDeviceHandle(volume, READ_OPERATION);
	
	TotalNoOfSectors = getNoOfSectors(read_hand);			// How many sectors do you have? :)
	MAX_Threads = getNoOfMaxThreads(TotalNoOfSectors);		// get MAX Threads to launch max:64  min:2 
	sectors_per_thread = TotalNoOfSectors / MAX_Threads;    //Calculate No of sectors for a single thread 

	
	if (xts_encrypt_key(key, SECTOR_SIZE, &ctx) != EXIT_SUCCESS)
	{
			ret = EXIT_FAILURE;
	}
	else
	{
		//Generate Necessary Arrays to Handle Multithreading 
		PTARGS* ArgsDataArray = malloc(sizeof(PTARGS)*MAX_Threads);
		DWORD* ThreadIdArray = malloc(sizeof(DWORD)*MAX_Threads);
		HANDLE* hThreadArray = malloc(sizeof(HANDLE)*MAX_Threads);


		//Just before allocating threads to sectors, init sector to start from the boot sector i.e. first i.e zero
		sector_offset = 0;

		for (int i = 1; i < MAX_Threads; i++)
		{
			ArgsDataArray[i] = (PTARGS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TARGS));

			if (ArgsDataArray[i] == NULL)
			{
				// If the array allocation fails, the system is out of memory
				// so there is no point in trying to print an error message.
				// Just terminate execution.
				ret = EXIT_FAILURE;
			}
			else
			{
				// Generate unique data for each thread to work with.
				ArgsDataArray[i]->ctx = ctx;										//same key ctx for every sector 
			//	ArgsDataArray[i]->read = read_hand;									//reading handle to volume
			//	ArgsDataArray[i]->write = write_hand;								//writing handle to volume
				ArgsDataArray[i]->sector_start = sector_offset;						//starting sector for block
				ArgsDataArray[i]->sector_end = (sector_offset + sectors_per_thread) - 1;		//ending sector for block


			   // Create the thread to begin execution on its own.
				hThreadArray[i] = CreateThread(
					NULL,                   // default security attributes
					0,                      // use default stack size  
					EncryptSectorBlock,     // thread function name
					ArgsDataArray[i],          // argument to thread function 
					0,                      // use default creation flags 
					&ThreadIdArray[i]);   // returns the thread identifier 

				if (hThreadArray[i] == NULL)
				{
					ret = EXIT_FAILURE;
				}

				sector_offset += sectors_per_thread;
			}
			
		} // End of main thread creation loop.

			// Wait until all threads have terminated.
			WaitForMultipleObjects(MAX_Threads, hThreadArray, TRUE, INFINITE);

			// Free all the dynamically allocated structures
			free(ArgsDataArray);
			free(ThreadIdArray);
			free(hThreadArray);
		
	}
	CloseHandle(read_hand);
	CloseHandle(write_hand);

	return ret;

}
boolean DecryptFullVolume(wchar_t* volume, unsigned char* key)
{
	static unsigned char buffer[SECTOR_SIZE];

	int TotalNoOfSectors;
	xts_decrypt_ctx ctx;

	xts_decrypt_key(key, SECTOR_SIZE, &ctx);

	HANDLE hand = getDeviceHandle(volume, WRITE_OPERATION);
	TotalNoOfSectors = getNoOfSectors(hand);

	/*for (int i = 0; i < TotalNoOfSectors; i++)
	{*/
		if (ReadSector(hand, buffer, SECTOR_SIZE, 0) != 1)
			printf("Error reading sector %i\n", 0);

		xts_decrypt_sector(buffer, 0, SECTOR_SIZE, &ctx);

		if (WriteSector(hand, buffer, SECTOR_SIZE, 0) != 1)
			printf("Error writing sector %i\n", 0);
	/*}*/


}
//
//int main()
//{
//	clock_t start;
//	LONGLONG diff;
//
//	start = clock();
//
//	//if(EncryptFullVolume(L"\\\\.\\G:", KEY) == EXIT_SUCCESS);
//	{ 
//		DecryptFullVolume(L"\\\\.\\F:", KEY);
//
//		diff = (clock() - start) / ((LONGLONG)CLOCKS_PER_SEC);
//		//printf("Successfully Encrypted Volume in %d seconds", diff);
//	}
//	getch();
//}


//
int main()
{

//	//static char buff[512] = { "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin eget scelerisque libero, id tempor lacus. Aliquam semper eros id libero lacinia lacinia. Proin ultrices aliquam urna. Curabitur imperdiet ullamcorper porttitor. Cras a dui venenatis, ornare magna hendrerit, vestibulum tortor. Phasellus a metus lacus. Quisque tristique elit nisi, sed vehicula velit consectetur non. Cras nisl urna, pretium quis ante eget, aliquet vulputate metus. Nulla tellus dolor, pulvinar et neque non, luctus auctor massa nunc." };
	static BYTE read[512];
	static HANDLE hand;
//	DWORD status;
	xts_encrypt_ctx ectx;
	xts_decrypt_ctx dctx;


	hand = getDeviceHandle(L"\\\\.\\G:", 'w');

	int totsect = getNoOfSectors(hand);

	clock_t start;
	LONGLONG diff;

	start = clock();

	for (int i = 0; i < totsect; i++)
	{
		if (ReadSector(hand, read, SECTOR_SIZE, i) == 1)
		{
			//printf("successfully read sector \n");
		}

		xts_encrypt_key(KEY, 512, &ectx);
		xts_encrypt_sector(read, i, 512, &ectx);


		if (WriteSector(hand, read, SECTOR_SIZE, i) == 1)
		{
			//printf("successfully wrote sector \n");
		}
	}

	diff = (clock() - start) / ((double)CLOCKS_PER_SEC);
	printf("Encrypted whole volume in: %lseconds or %l minutes or %l hours", diff, diff / 60, diff / 3600);

	memset(read, 0, SECTOR_SIZE);

	start = clock();

	for (int i = 0; i < totsect; i++)
	{
		if (ReadSector(hand, read, SECTOR_SIZE, i) == 1)
		{
			//printf("successfully read sector \n");
		}

		xts_decrypt_key(KEY, 512, &dctx);
		xts_decrypt_sector(read, i, 512, &dctx);


		if (WriteSector(hand, read, SECTOR_SIZE, i) == 1)
		{
			//printf("successfully wrote sector \n");
		}
	}

	diff = (clock() - start)/ ((double)CLOCKS_PER_SEC);
	printf("Decrypted whole volume in: %l seconds or %l minutes or %l hours", diff, diff / 60, diff / 3600);
	
	
	CloseHandle(hand);				// Close the handle
//
	getch();
//
//
//
}