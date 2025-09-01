#include "handle.hpp"

#include <iostream>

int main()
{
	using namespace std;

	ProcessHandle paint;
	
	paint = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 6656);
	if (!paint.Valid())
	{
		cout << "Failed to create process handle\n";
		return EXIT_FAILURE;
	}

	FileHandle fileHandle = CreateFile(L"test.txt",
									   FILE_GENERIC_WRITE | FILE_GENERIC_READ,
									   FILE_SHARE_READ | FILE_SHARE_WRITE,
									   nullptr,
									   OPEN_ALWAYS,
									   FILE_ATTRIBUTE_NORMAL,
									   nullptr);
	if (!fileHandle.Valid())
	{
		cout << "Failed to create file handle\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}