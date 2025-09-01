# handle

> RAII (Resource Acquisition Is Initialization) wrapper for Windows API handle types.

Idea behind this was to unify Windows API handle types into one interface. `HANDLE` type has two invalid values depending on context - either `NULL` or `INVALID_HANDLE_VALUE`. This has been solved with `TaggedHandle<_Ty>` class which is a smaller wrapper around `HANDLE` to add use case context with tags:
```
namespace HandleType
{
    struct Event            {};
    struct Mutex            {};
    struct Semaphore        {};
    struct Process          {};
    struct Thread           {};
    struct IoCompletionPort {};
    struct Job              {};
    struct WaitableTimer    {};

    struct File        {};
    struct NamedPipe   {};
    struct MailSlot    {};
    struct FileMapping {};
    struct Snapshot    {};
    // ...
}
```

## Tech

Built with Visual Studio 2022 platform toolset (MSVC). I did not cover Clang platform toolset. Might move it to CMake - feel free to contribute


## Examples

```cpp
#include "handle.hpp"

// ...

FileHandle fileHandle = CreateFile(L"test.txt",
								   FILE_GENERIC_WRITE | FILE_GENERIC_READ,
								   FILE_SHARE_READ | FILE_SHARE_WRITE,
								   nullptr,
								   OPEN_ALWAYS,
								   FILE_ATTRIBUTE_NORMAL,
								   nullptr);
if (!fileHandle.Valid())
{
	// Handle error...
}
```