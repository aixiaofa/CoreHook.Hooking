#pragma once
/////////////////////////////////////////////////////////////////////////////
//
//  Trampoline Thread Barrier Functionality (barrier.h of detours.lib)
//
//

BOOL detour_is_loader_lock();

BOOL detour_acquire_self_protection();
void detour_release_self_protection();

void detour_sleep(_In_ DWORD milliSeconds);

/////////////////////////////////////////// Trampoline thread barrier definitions
//

// Maximum number of detours that can be created
#define MAX_HOOK_COUNT              1024
// Maximum number of entries in the hook ACL
#define MAX_ACE_COUNT               128
// Maximum number of threads supported by a hook ACL
#define MAX_THREAD_COUNT            128

typedef struct _HOOK_ACL_
{
    ULONG                   Count;
    BOOL                    IsExclusive;
    ULONG                   Entries[MAX_ACE_COUNT];
} HOOK_ACL;

typedef struct _RUNTIME_INFO_
{
    // Set to "true" if the current thread is within the related hook handler.
    BOOL            IsExecuting;
    // The hook this information entry belongs to. This allows a per thread and hook storage!
    DWORD           HLSIdent;
    // The return address of the current thread's hook handler.
    PVOID           RetAddress;
    // The address of the return address of the current thread's hook handler.
    PVOID*          AddrOfRetAddr;
} RUNTIME_INFO;

typedef struct _THREAD_RUNTIME_INFO_
{
    RUNTIME_INFO*        Entries;
    RUNTIME_INFO*        Current;
    PVOID                Callback;
    BOOL                 IsProtected;
} THREAD_RUNTIME_INFO, *PTHREAD_RUNTIME_INFO;

typedef struct _RTL_SPIN_LOCK_
{
    CRITICAL_SECTION         Lock;
    BOOL                     IsOwned;
} DETOUR_SPIN_LOCK;

typedef struct _THREAD_LOCAL_STORAGE_
{
    THREAD_RUNTIME_INFO      Entries[MAX_THREAD_COUNT];
    DWORD                    IdList[MAX_THREAD_COUNT];
    DETOUR_SPIN_LOCK         ThreadSafe;
} THREAD_LOCAL_STORAGE;

typedef struct _BARRIER_UNIT_
{
    HOOK_ACL                GlobalACL;
    BOOL                    IsInitialized;
    THREAD_LOCAL_STORAGE    TLS;
} BARRIER_UNIT;

static void detour_initialize_lock(_In_ DETOUR_SPIN_LOCK *pLock);

static void detour_delete_lock(_In_ DETOUR_SPIN_LOCK *pLock);

void detour_acquire_lock(_In_ DETOUR_SPIN_LOCK *pLock);

void detour_release_lock(_In_ DETOUR_SPIN_LOCK *pLock);

BOOL detour_is_thread_intercepted(_In_ HOOK_ACL *pLocalACL,
                                  _In_ DWORD    dwThreadId);

LONG detour_set_acl(_In_ HOOK_ACL *pAcl,
                    _In_ BOOL bIsExclusive,
                    _In_ DWORD *dwThreadIdList,
                    _In_ DWORD dwThreadCount);

HOOK_ACL* detour_barrier_get_acl();

extern BARRIER_UNIT         g_BarrierUnit;
extern DETOUR_SPIN_LOCK     g_HookLock;

//////////////////////////////////////////////// Exception handling code
//

VOID detour_assert(PCSTR pszMsg, LPCWSTR pszFile, ULONG nLine);

#ifndef NDEBUG
#define ASSERT(expr)           ASSERT_ALWAYS(expr)
#else
#define ASSERT(expr)
#endif

#define WIDE2(x) L ##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

#define ASSERT_ALWAYS(expression)   \
    do {                                                                 \
    if (!(expression)) {                                                 \
            detour_assert(#expression, WFILE, __LINE__);                 \
    }                                                                    \
    } while (0)


#define DETOUR_ASSERT(expr)         ASSERT(expr)

//////////////////////////////////////////////// Memory validation code
//

#define IsValidPointer              detour_is_valid_pointer

BOOL detour_is_valid_pointer(_In_opt_ CONST VOID *Pointer,
                             _In_     UINT_PTR    Size);

/////////////////////////////////////////////////////////////
//
//  Thread Local Storage functions re-implemented to avoid
//  possible problems with native TLS functions when
//  detouring processes like explorer.exe.
//

BOOL TlsGetCurrentValue(_In_  THREAD_LOCAL_STORAGE *pTls,
                        _Outptr_ THREAD_RUNTIME_INFO  **OutValue);

BOOL TlsAddCurrentThread(_In_ THREAD_LOCAL_STORAGE *pTls);


////////////////////////////////////////////////// Memory management functions

void  detour_free_memory(void *pMemory);

void* detour_allocate_memory(_In_ BOOL   bZeroMemory,
                             _In_ size_t size);
