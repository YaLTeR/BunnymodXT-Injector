#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>

DWORD GetProcessID(const TCHAR* processName)
{
	HANDLE snapshot;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "CreateToolhelp32Snapshot error: %d\n", GetLastError());
		return 0;
	}

	if (Process32First(snapshot, &entry) == TRUE)
	{
		do
		{
			if (_tcsicmp(entry.szExeFile, processName) == 0)
			{
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		} while (Process32Next(snapshot, &entry) == TRUE);
	}
	else
	{
		fprintf(stderr, "Process32First error: %d\n", GetLastError());
	}

	CloseHandle(snapshot);
	return 0;
}

BOOL GetBunnymodDLLFileName(TCHAR* fileName, DWORD nSize)
{
	TCHAR exePath[MAX_PATH];

	if (GetModuleFileName(NULL, exePath, MAX_PATH) != 0)
	{
		// Assume that the path did fit.
		TCHAR *temp = _tcsrchr(exePath, '\\');
		if (temp != NULL)
		{
			*(temp + 1) = '\0';
			_tcscat_s(exePath, MAX_PATH, TEXT("BunnymodXT.dll"));
			_tcscpy_s(fileName, nSize, exePath);

			return TRUE;
		}
		else
		{
			fputs("Got invalid injector file name!\n", stderr);
		}
	}
	else
	{
		fprintf(stderr, "Failed getting the injector file name: %d\n", GetLastError());
	}

	return FALSE;
}

void DoInjection(HANDLE targetProcess)
{
	TCHAR dllFileName[MAX_PATH];
	LPVOID dllPathAddr;
	HANDLE loadlibThread;
	DWORD exitCode;

	dllPathAddr = VirtualAllocEx(targetProcess, NULL, sizeof(dllFileName), (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if (dllPathAddr == NULL)
	{
		fprintf(stderr, "Failed allocating memory: %d\n", GetLastError());
		return;
	}

	if (!GetBunnymodDLLFileName(dllFileName, MAX_PATH))
	{
		VirtualFreeEx(targetProcess, dllPathAddr, 0, MEM_RELEASE);
		return; // The function outputs error messages itself.
	}

	if (WriteProcessMemory(targetProcess, dllPathAddr, dllFileName, sizeof(dllFileName), NULL) == FALSE)
	{
		fprintf(stderr, "Failed writing the DLL path to the process: %d\n", GetLastError());
		VirtualFreeEx(targetProcess, dllPathAddr, 0, MEM_RELEASE);
		return;
	}

	loadlibThread = CreateRemoteThread(targetProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, dllPathAddr, 0, NULL);
	if (loadlibThread == NULL)
	{
		fprintf(stderr, "Failed creating a LoadLibrary thread: %d\n", GetLastError());
		VirtualFreeEx(targetProcess, dllPathAddr, 0, MEM_RELEASE);
		return;
	}

	WaitForSingleObject(loadlibThread, INFINITE);
	if (GetExitCodeThread(loadlibThread, &exitCode) != FALSE)
	{
		if (exitCode != 0)
		{
			_tprintf(TEXT("Success! DLL module address: %p\n"), exitCode);
		}
		else
		{
			fputs("LoadLibrary failed.\n", stderr);
		}
	}
	else
	{
		fprintf(stderr, "Failed getting the LoadLibrary return value: %d\n", GetLastError());
	}

	CloseHandle(loadlibThread);
	VirtualFreeEx(targetProcess, dllPathAddr, 0, MEM_RELEASE);
}

void CheckArgs(int argc, const TCHAR** argv, BOOL* needToWait, size_t processNameSize, TCHAR* processName)
{
	if (needToWait)
		*needToWait = TRUE;

	if (processName)
	{
		_tcsncpy(processName, TEXT("hl.exe"), processNameSize);
		if (processNameSize <= 6)
			processName[processNameSize] = '\0';
	}

	for (int i = 1; i < argc; ++i)
	{
		if (needToWait && (_tcscmp(argv[i], TEXT("-noconfirm")) == 0))
			*needToWait = FALSE;

		if (processName && (_tcscmp(argv[i], TEXT("-processname")) == 0) && ((i + 1) < argc))
		{
			i++;
			_tcsncpy(processName, argv[i], processNameSize);
			if (processNameSize <= _tcslen(argv[i]))
				processName[processNameSize] = '\0';
		}
	}
}

void waitForKey(BOOL needToWait)
{
	if (needToWait == TRUE)
		getc(stdin);
}

int _tmain(int argc, TCHAR** argv)
{
	HANDLE hlProcess;
	DWORD hlPID;
	BOOL needToWait;
	TCHAR processName[16];
	
	CheckArgs(argc, argv, &needToWait, (sizeof(processName) / sizeof(TCHAR)), processName);
	hlPID = GetProcessID(processName);

	if (hlPID == 0)
	{
		_ftprintf(stderr, TEXT("%s is not running!\n"), processName);
		waitForKey(needToWait);
		return 0;
	}

	hlProcess = OpenProcess((PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ), FALSE, hlPID);
	if (hlProcess == NULL)
	{
		fprintf(stderr, "Failed opening the process: %d\n", GetLastError());
		waitForKey(needToWait);
		return 0;
	}

	DoInjection(hlProcess);

	CloseHandle(hlProcess);
	waitForKey(needToWait);
	return 0;
}
