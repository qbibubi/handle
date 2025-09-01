#pragma once
#include <type_traits>
#include <cstdint>
#include <concepts>
#include <windows.h>
#include <bit>

/*
 * @brief Creates a HandleTraits<_Ty> specialization
 *
 * @param Handle type
 * @param Invalid handle value for specified handle type
 * @param Closing function for specified handle type
 */
#define CREATE_HANDLE_TRAITS(type, invalidHandleValue, handleCloseFunction)             \
template<>                                                                              \
struct HandleTraits<type>                                                               \
{                                                                                       \
    using Type = type;                                                                  \
                                                                                        \
    inline static const Type InvalidHandleValue = invalidHandleValue;                   \
                                                                                        \
    static void Close(Type handle) noexcept { ::handleCloseFunction(handle);  }         \
    static bool Valid(Type handle) noexcept { return handle != InvalidHandleValue; }    \
};

/*
 * @brief HANDLE tags for TaggedHandle struct
 *
 * HANDLE is reponsible for many types of different resources. Tags provide a way
 * to distinguish them from each other. First part of the namespace has resources
 * that default to NULL. Second part has resources that default to INVALID_HANDLE_VALUE
 */
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
}

template<typename _Tag>
concept HandleValueNull = std::is_same_v<_Tag, HandleType::Event> || 
                          std::is_same_v<_Tag, HandleType::Mutex> ||
                          std::is_same_v<_Tag, HandleType::Process> ||
                          std::is_same_v<_Tag, HandleType::Thread> ||
                          std::is_same_v<_Tag, HandleType::Job> ||
                          std::is_same_v<_Tag, HandleType::WaitableTimer> ||
                          std::is_same_v<_Tag, HandleType::IoCompletionPort>;

template<typename _Tag>
concept HandleValueInvalid = std::is_same_v<_Tag, HandleType::File> ||
                             std::is_same_v<_Tag, HandleType::NamedPipe> ||
                             std::is_same_v<_Tag, HandleType::FileMapping> ||
                             std::is_same_v<_Tag, HandleType::MailSlot> ||
                             std::is_same_v<_Tag, HandleType::Snapshot>;

/*
 * @brief TaggedHandle wraps HANDLE type and adds a HandleType::<Tag> to it for clarity.
 */
template<typename _Tag>
struct TaggedHandle
{
    using Tag = _Tag;
    using Type = HANDLE;

    Type m_Handle;

public:
    constexpr explicit TaggedHandle() noexcept
        : m_Handle(GetHandleInvalidValue())
    {}

    TaggedHandle(Type handle) noexcept
        : m_Handle(handle)
    {}

    [[nodiscard]] static constexpr HANDLE GetHandleInvalidValue() noexcept
    {
        if constexpr (HandleValueNull<_Tag>)
        {
            return nullptr;
        }
        else if constexpr (HandleValueInvalid<_Tag>)
        {
            return INVALID_HANDLE_VALUE;
        }
        else
        {
            static_assert(sizeof(_Tag) == 0, "Unhandled handle type");
        }
    }
};

 /*
  * @brief Base HandleTraits for specialization
  *
  * @tparam Handle type
  */
template<typename _Ty>
struct HandleTraits;

/*
 * @brief HandleTraits specialization for TaggedHandle types
 *
 * @tparam HandleType tag
 */
template<typename _Tag>
struct HandleTraits<TaggedHandle<_Tag>>
{
    using Handle = TaggedHandle<_Tag>;
    using Type   = Handle::Type;
    using Tag    = Handle::Tag;

    inline static const Type InvalidHandleValue = Handle::GetHandleInvalidValue();

    static void Close(Type handle) noexcept 
    { 
        ::CloseHandle(handle); 
    }
    
    [[nodiscard]] static bool Valid(Type handle) noexcept 
    { 
        return handle != InvalidHandleValue; 
    }
};

CREATE_HANDLE_TRAITS(SOCKET,    NULL,    closesocket)
CREATE_HANDLE_TRAITS(HKEY,      nullptr, RegCloseKey)
CREATE_HANDLE_TRAITS(HWND,      nullptr, DestroyWindow)
CREATE_HANDLE_TRAITS(HMENU,     nullptr, DestroyMenu)
CREATE_HANDLE_TRAITS(HICON,     nullptr, DestroyIcon)
CREATE_HANDLE_TRAITS(HDC,       nullptr, DeleteDC)
CREATE_HANDLE_TRAITS(HBITMAP,   nullptr, DeleteObject)
CREATE_HANDLE_TRAITS(HPEN,      nullptr, DeleteObject)
CREATE_HANDLE_TRAITS(HBRUSH,    nullptr, DeleteObject)
CREATE_HANDLE_TRAITS(HPALETTE,  nullptr, DeleteObject)
CREATE_HANDLE_TRAITS(HINSTANCE, nullptr, FreeLibrary)

template<typename _Ty>
struct HandleBaseType
{
    using Type = _Ty;
};

template<typename _Tag>
struct HandleBaseType<TaggedHandle<_Tag>>
{
    using Type = typename TaggedHandle<_Tag>::Type;
};

/*
 * @brief RAII Wrapper around Windows API handles
 *
 * @tparam Handle type
 */
template<typename _Ty>
class Handle
{
private:
    using Traits = HandleTraits<_Ty>;
    using Type   = typename HandleBaseType<_Ty>::Type;

    Type m_Handle;

public:
    constexpr Handle(Type handle = Traits::InvalidHandleValue) noexcept
        : m_Handle(handle)
    {}

    Handle(Handle const&) = delete;
    Handle& operator=(Handle const&) = delete;

    Handle& operator=(Type handle) noexcept
    {
        if (Valid())
        {
            Close();
        }

        m_Handle = handle;
        return *this;
    }

    ~Handle()
    { 
        Close();
    }

public:
    [[nodiscard]] bool Valid() const noexcept
    {
        return Traits::Valid(m_Handle);
    }

    void Close() noexcept
    {
        if (Traits::Valid(m_Handle))
        {
            Traits::Close(m_Handle);
            m_Handle = Traits::InvalidHandleValue;
        }
    }

public:
    // `explicit` grants more type safety but we don't care
    [[nodiscard]] operator Type() const noexcept
    {
        return m_Handle;
    }

    [[nodiscard]] Type Get() const noexcept
    {
        return m_Handle;
    }

    [[nodiscard]] Type* operator&() noexcept
    {
        return &m_Handle;
    }

    [[nodiscard]] Type const* operator&() const noexcept
    {
        return &m_Handle;
    }
};

using EventHandle            = Handle<TaggedHandle<HandleType::Event>>;
using MutexHandle            = Handle<TaggedHandle<HandleType::Mutex>>;
using SemaphoreHandle        = Handle<TaggedHandle<HandleType::Semaphore>>;
using ProcessHandle          = Handle<TaggedHandle<HandleType::Process>>;
using ThreadHandle           = Handle<TaggedHandle<HandleType::Thread>>;
using IoCompletionPortHandle = Handle<TaggedHandle<HandleType::IoCompletionPort>>;
using JobHandle              = Handle<TaggedHandle<HandleType::Job>>;
using WaitableTimerHandle    = Handle<TaggedHandle<HandleType::WaitableTimer>>;

using FileHandle        = Handle<TaggedHandle<HandleType::File>>;
using NamedPipeHandle   = Handle<TaggedHandle<HandleType::NamedPipe>>;
using MailSlotHandle    = Handle<TaggedHandle<HandleType::MailSlot>>;
using FileMappingHandle = Handle<TaggedHandle<HandleType::FileMapping>>;
using SnapshotHandle    = Handle<TaggedHandle<HandleType::Snapshot>>;