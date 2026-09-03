#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <sddl.h>
#include <aclapi.h>
#include <stdint.h>

#pragma comment(lib, "Advapi32.lib")


typedef LONG NTSTATUS;
typedef NTSTATUS (NTAPI *oracleisgoated_NtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS (NTAPI *oracleisgoated_NtQueryObject)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS (NTAPI *oracleisgoated_NtSetIoCompletion)(HANDLE, PVOID, PVOID, NTSTATUS, ULONG_PTR);
typedef NTSTATUS (NTAPI *oracleisgoated_LdrRegisterDllNotification)(ULONG, PVOID, PVOID, PVOID *);
typedef NTSTATUS (NTAPI *oracleisgoated_LdrUnregisterDllNotification)(PVOID);

struct oracleisgoated_unicode_string
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
};

struct oracleisgoated_process_handle_entry
{
    HANDLE HandleValue;
    ULONG_PTR HandleCount;
    ULONG_PTR PointerCount;
    ACCESS_MASK GrantedAccess;
    ULONG ObjectTypeIndex;
    ULONG HandleAttributes;
    ULONG Reserved;
};

struct oracleisgoated_process_handle_snapshot
{
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    oracleisgoated_process_handle_entry Handles[1];
};

struct oracleisgoated_remote_list_entry
{
    uintptr_t Address;
    LIST_ENTRY Links;
};

struct oracleisgoated_remote_list_state
{
    uintptr_t Head;
    LIST_ENTRY HeadLinks;
    oracleisgoated_remote_list_entry Entries[128];
    DWORD Count;
    BOOL Active;
};

struct oracleisgoated_hook_state
{
    uintptr_t Target;
    BYTE Saved[15];
    SIZE_T Size;
    DWORD OldProtect;
    BOOL Active;
};

void *oracleisgoated_memzero(void *buffer, size_t size)
{
    unsigned char *out = (unsigned char *)buffer;
    for (size_t i = 0; i < size; ++i)
        out[i] = 0;
    return buffer;
}

void *oracleisgoated_memcopy(void *dst, const void *src, size_t size)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < size; ++i)
        d[i] = s[i];
    return dst;
}

size_t oracleisgoated_strlen(const char *text)
{
    size_t length = 0;
    if (!text)
        return 0;
    while (text[length])
        ++length;
    return length;
}

char oracleisgoated_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 32;
    return c;
}

wchar_t oracleisgoated_wlower(wchar_t c)
{
    if (c >= L'A' && c <= L'Z')
        return c + 32;
    return c;
}

int oracleisgoated_streqi(const char *a, const char *b)
{
    if (!a || !b)
        return 0;
    while (*a && *b)
    {
        if (oracleisgoated_lower(*a) != oracleisgoated_lower(*b))
            return 0;
        ++a;
        ++b;
    }
    return *a == 0 && *b == 0;
}

int oracleisgoated_wstreqi(const wchar_t *a, const wchar_t *b)
{
    if (!a || !b)
        return 0;
    while (*a && *b)
    {
        if (oracleisgoated_wlower(*a) != oracleisgoated_wlower(*b))
            return 0;
        ++a;
        ++b;
    }
    return *a == 0 && *b == 0;
}

int oracleisgoated_streq(const char *a, const char *b)
{
    if (!a || !b)
        return 0;
    while (*a && *b)
    {
        if (*a != *b)
            return 0;
        ++a;
        ++b;
    }
    return *a == 0 && *b == 0;
}

void oracleisgoated_print(const char *text)
{
    DWORD written = 0;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out && out != INVALID_HANDLE_VALUE)
        WriteFile(out, text, (DWORD)oracleisgoated_strlen(text), &written, 0);
}

void *oracleisgoated_heap_alloc(size_t size)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void oracleisgoated_heap_free(void *buffer)
{
    if (buffer)
        HeapFree(GetProcessHeap(), 0, buffer);
}

HMODULE oracleisgoated_get_module_local(const char *name)
{
    return GetModuleHandleA(name);
}

FARPROC oracleisgoated_get_proc_local(HMODULE module, const char *name)
{
    if (!module)
        return 0;
    return GetProcAddress(module, name);
}

int oracleisgoated_file_exists(const char *path)
{
    DWORD attrs = GetFileAttributesA(path);
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

void oracleisgoated_start_decompiler()
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    oracleisgoated_memzero(&si, sizeof(si));
    oracleisgoated_memzero(&pi, sizeof(pi));
    si.cb = sizeof(si);
    char command[] = "Decompiler.exe serve";
    if (CreateProcessA(0, command, 0, 0, FALSE, CREATE_NO_WINDOW, 0, 0, &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

int oracleisgoated_build_module_path(char *out, DWORD size)
{
    DWORD length = GetModuleFileNameA(0, out, size);
    if (!length || length >= size)
        return 0;
    DWORD cut = length;
    while (cut > 0 && out[cut - 1] != '\\' && out[cut - 1] != '/')
        --cut;
    const char suffix[] = "Module.dll";
    DWORD suffixLength = (DWORD)oracleisgoated_strlen(suffix);
    if (cut + suffixLength + 1 > size)
        return 0;
    for (DWORD i = 0; i < suffixLength; ++i)
        out[cut + i] = suffix[i];
    out[cut + suffixLength] = 0;
    return 1;
}

DWORD oracleisgoated_find_process_id(const wchar_t *name)
{
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;
    PROCESSENTRY32W pe;
    oracleisgoated_memzero(&pe, sizeof(pe));
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe))
    {
        do
        {
            if (oracleisgoated_wstreqi(pe.szExeFile, name))
            {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

uintptr_t oracleisgoated_find_remote_module(HANDLE process, const wchar_t *name)
{
    HMODULE modules[1024];
    DWORD needed = 0;
    if (!EnumProcessModulesEx(process, modules, sizeof(modules), &needed, LIST_MODULES_ALL))
        return 0;
    DWORD count = needed / sizeof(HMODULE);
    for (DWORD i = 0; i < count && i < 1024; ++i)
    {
        wchar_t base[MAX_PATH];
        oracleisgoated_memzero(base, sizeof(base));
        if (GetModuleBaseNameW(process, modules[i], base, MAX_PATH))
        {
            if (oracleisgoated_wstreqi(base, name))
                return (uintptr_t)modules[i];
        }
    }
    return 0;
}

int oracleisgoated_read_remote(HANDLE process, uintptr_t address, void *buffer, SIZE_T size)
{
    SIZE_T read = 0;
    if (!ReadProcessMemory(process, (LPCVOID)address, buffer, size, &read))
        return 0;
    return read == size;
}

int oracleisgoated_write_remote(HANDLE process, uintptr_t address, const void *buffer, SIZE_T size)
{
    SIZE_T written = 0;
    if (!WriteProcessMemory(process, (LPVOID)address, buffer, size, &written))
        return 0;
    return written == size;
}

int oracleisgoated_read_remote_string(HANDLE process, uintptr_t address, char *buffer, SIZE_T size)
{
    if (!buffer || size == 0)
        return 0;
    oracleisgoated_memzero(buffer, size);
    for (SIZE_T i = 0; i + 1 < size; ++i)
    {
        SIZE_T read = 0;
        if (!ReadProcessMemory(process, (LPCVOID)(address + i), &buffer[i], 1, &read) || read != 1)
            return 0;
        if (buffer[i] == 0)
            return 1;
    }
    buffer[size - 1] = 0;
    return 1;
}

uintptr_t oracleisgoated_resolve_remote_export(HANDLE process, uintptr_t moduleBase, const char *exportName)
{
    IMAGE_DOS_HEADER dos;
    IMAGE_NT_HEADERS64 nt;
    oracleisgoated_memzero(&dos, sizeof(dos));
    oracleisgoated_memzero(&nt, sizeof(nt));
    if (!oracleisgoated_read_remote(process, moduleBase, &dos, sizeof(dos)))
        return 0;
    if (dos.e_magic != IMAGE_DOS_SIGNATURE)
        return 0;
    if (!oracleisgoated_read_remote(process, moduleBase + (uintptr_t)dos.e_lfanew, &nt, sizeof(nt)))
        return 0;
    if (nt.Signature != IMAGE_NT_SIGNATURE)
        return 0;
    IMAGE_DATA_DIRECTORY dir = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (!dir.VirtualAddress || !dir.Size)
        return 0;
    IMAGE_EXPORT_DIRECTORY exp;
    oracleisgoated_memzero(&exp, sizeof(exp));
    if (!oracleisgoated_read_remote(process, moduleBase + dir.VirtualAddress, &exp, sizeof(exp)))
        return 0;
    DWORD namesSize = exp.NumberOfNames * sizeof(DWORD);
    DWORD funcsSize = exp.NumberOfFunctions * sizeof(DWORD);
    DWORD ordsSize = exp.NumberOfNames * sizeof(WORD);
    DWORD *names = (DWORD *)oracleisgoated_heap_alloc(namesSize);
    DWORD *funcs = (DWORD *)oracleisgoated_heap_alloc(funcsSize);
    WORD *ords = (WORD *)oracleisgoated_heap_alloc(ordsSize);
    uintptr_t result = 0;
    if (!names || !funcs || !ords)
        goto done;
    if (!oracleisgoated_read_remote(process, moduleBase + exp.AddressOfNames, names, namesSize))
        goto done;
    if (!oracleisgoated_read_remote(process, moduleBase + exp.AddressOfFunctions, funcs, funcsSize))
        goto done;
    if (!oracleisgoated_read_remote(process, moduleBase + exp.AddressOfNameOrdinals, ords, ordsSize))
        goto done;
    for (DWORD i = 0; i < exp.NumberOfNames; ++i)
    {
        char current[256];
        if (!oracleisgoated_read_remote_string(process, moduleBase + names[i], current, sizeof(current)))
            continue;
        if (oracleisgoated_streq(current, exportName))
        {
            WORD ord = ords[i];
            if (ord < exp.NumberOfFunctions)
                result = moduleBase + funcs[ord];
            break;
        }
    }
done:
    oracleisgoated_heap_free(names);
    oracleisgoated_heap_free(funcs);
    oracleisgoated_heap_free(ords);
    return result;
}

int oracleisgoated_apply_dacl(const char *path, const char *sddl)
{
    PSECURITY_DESCRIPTOR sd = 0;
    PACL dacl = 0;
    BOOL present = FALSE;
    BOOL defaulted = FALSE;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(sddl, SDDL_REVISION_1, &sd, 0))
        return 0;
    if (!GetSecurityDescriptorDacl(sd, &present, &dacl, &defaulted))
    {
        LocalFree(sd);
        return 0;
    }
    DWORD status = SetNamedSecurityInfoA((LPSTR)path, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, 0, 0, present ? dacl : 0, 0);
    LocalFree(sd);
    return status == ERROR_SUCCESS;
}

void CALLBACK oracleisgoated_dll_notification(ULONG reason, const void *data, void *context)
{
    (void)reason;
    (void)data;
    (void)context;
}

int oracleisgoated_derive_remote_notification_head(HANDLE process, uintptr_t remoteNtdll, uintptr_t *out)
{
    HMODULE localNtdll = oracleisgoated_get_module_local("ntdll.dll");
    if (!localNtdll || !remoteNtdll || !out)
        return 0;
    oracleisgoated_LdrRegisterDllNotification reg = (oracleisgoated_LdrRegisterDllNotification)oracleisgoated_get_proc_local(localNtdll, "LdrRegisterDllNotification");
    oracleisgoated_LdrUnregisterDllNotification unreg = (oracleisgoated_LdrUnregisterDllNotification)oracleisgoated_get_proc_local(localNtdll, "LdrUnregisterDllNotification");
    if (!reg || !unreg)
        return 0;
    PVOID cookie = 0;
    NTSTATUS st = reg(0, (PVOID)oracleisgoated_dll_notification, 0, &cookie);
    if (st < 0 || !cookie)
        return 0;
    LIST_ENTRY localCookieLinks;
    oracleisgoated_memcopy(&localCookieLinks, cookie, sizeof(localCookieLinks));
    unreg(cookie);
    uintptr_t localHead = (uintptr_t)localCookieLinks.Flink;
    if (!localHead)
        return 0;
    *out = remoteNtdll + (localHead - (uintptr_t)localNtdll);
    return 1;
}

int oracleisgoated_unlink_remote_notifications(HANDLE process, uintptr_t remoteNtdll, oracleisgoated_remote_list_state *state)
{
    if (!state)
        return 0;
    oracleisgoated_memzero(state, sizeof(*state));
    if (!oracleisgoated_derive_remote_notification_head(process, remoteNtdll, &state->Head))
        return 0;
    if (!oracleisgoated_read_remote(process, state->Head, &state->HeadLinks, sizeof(state->HeadLinks)))
        return 0;
    uintptr_t current = (uintptr_t)state->HeadLinks.Flink;
    while (current && current != state->Head && state->Count < 128)
    {
        LIST_ENTRY links;
        if (!oracleisgoated_read_remote(process, current, &links, sizeof(links)))
            return 0;
        state->Entries[state->Count].Address = current;
        state->Entries[state->Count].Links = links;
        ++state->Count;
        current = (uintptr_t)links.Flink;
    }
    LIST_ENTRY self;
    self.Flink = (PLIST_ENTRY)state->Head;
    self.Blink = (PLIST_ENTRY)state->Head;
    if (!oracleisgoated_write_remote(process, state->Head, &self, sizeof(self)))
        return 0;
    state->Active = TRUE;
    return 1;
}

void oracleisgoated_restore_remote_notifications(HANDLE process, oracleisgoated_remote_list_state *state)
{
    if (!state || !state->Active)
        return;
    for (DWORD i = 0; i < state->Count; ++i)
        oracleisgoated_write_remote(process, state->Entries[i].Address, &state->Entries[i].Links, sizeof(LIST_ENTRY));
    oracleisgoated_write_remote(process, state->Head, &state->HeadLinks, sizeof(LIST_ENTRY));
    state->Active = FALSE;
}

uintptr_t oracleisgoated_resolve_hook_target(HANDLE process, uintptr_t exportAddress)
{
    BYTE code[16];
    oracleisgoated_memzero(code, sizeof(code));
    if (!oracleisgoated_read_remote(process, exportAddress, code, sizeof(code)))
        return 0;
    if (code[0] == 0x48 && code[1] == 0xB8 && code[10] == 0xFF && code[11] == 0xE0)
    {
        uintptr_t target = 0;
        oracleisgoated_memcopy(&target, code + 2, sizeof(target));
        return target;
    }
    if (code[0] == 0xFF && code[1] == 0x25)
    {
        int32_t disp = 0;
        oracleisgoated_memcopy(&disp, code + 2, sizeof(disp));
        uintptr_t pointerAddress = exportAddress + 6 + disp;
        uintptr_t target = 0;
        if (oracleisgoated_read_remote(process, pointerAddress, &target, sizeof(target)))
            return target;
        oracleisgoated_memcopy(&target, code + 6, sizeof(target));
        return target;
    }
    if (code[0] == 0xE9)
    {
        int32_t disp = 0;
        oracleisgoated_memcopy(&disp, code + 1, sizeof(disp));
        return exportAddress + 5 + disp;
    }
    return 0;
}

int oracleisgoated_patch_nt_create_section(HANDLE process, uintptr_t remoteNtdll, oracleisgoated_hook_state *state)
{
    if (!state)
        return 0;
    oracleisgoated_memzero(state, sizeof(*state));
    uintptr_t ntCreateSection = oracleisgoated_resolve_remote_export(process, remoteNtdll, "NtCreateSection");
    if (!ntCreateSection)
        return 0;
    uintptr_t hookTarget = oracleisgoated_resolve_hook_target(process, ntCreateSection);
    if (!hookTarget)
        return 0;
    state->Target = hookTarget;
    state->Size = sizeof(state->Saved);
    if (!oracleisgoated_read_remote(process, hookTarget, state->Saved, sizeof(state->Saved)))
        return 0;
    if (!VirtualProtectEx(process, (LPVOID)hookTarget, sizeof(state->Saved), PAGE_EXECUTE_READWRITE, &state->OldProtect))
        return 0;
    BYTE stub[] = { 0x49, 0x89, 0xCA, 0xB8, 0x4A, 0x00, 0x00, 0x00, 0x0F, 0x05, 0xC3 };
    if (!oracleisgoated_write_remote(process, hookTarget, stub, sizeof(stub)))
        return 0;
    FlushInstructionCache(process, (LPCVOID)hookTarget, sizeof(stub));
    state->Active = TRUE;
    return 1;
}

void oracleisgoated_restore_nt_create_section(HANDLE process, oracleisgoated_hook_state *state)
{
    if (!state || !state->Active)
        return;
    DWORD temp = 0;
    oracleisgoated_write_remote(process, state->Target, state->Saved, state->Size);
    VirtualProtectEx(process, (LPVOID)state->Target, state->Size, state->OldProtect, &temp);
    FlushInstructionCache(process, (LPCVOID)state->Target, state->Size);
    state->Active = FALSE;
}

int oracleisgoated_patch_placeholders(BYTE *code, SIZE_T size, uintptr_t base)
{
    if (!code || size < sizeof(uintptr_t))
        return 0;
    for (SIZE_T i = 0; i + sizeof(uintptr_t) <= size; ++i)
    {
        uintptr_t value = 0;
        oracleisgoated_memcopy(&value, code + i, sizeof(value));
        if (value - (uint32_t)value == 0x1000000000ULL)
        {
            uintptr_t patched = base + (uint32_t)value;
            oracleisgoated_memcopy(code + i, &patched, sizeof(patched));
        }
    }
    return 1;
}

void oracleisgoated_make_loader(BYTE *out, SIZE_T *size, uintptr_t context)
{
    BYTE code[] = {
        0x48, 0x83, 0xEC, 0x28,
        0x48, 0xB8, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0xFF, 0x10,
        0x48, 0xB9, 0x10, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x48, 0xB8, 0x08, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
        0x48, 0x8B, 0x09,
        0x48, 0x83, 0xC4, 0x28,
        0xFF, 0x20
    };
    oracleisgoated_memcopy(out, code, sizeof(code));
    oracleisgoated_patch_placeholders(out, sizeof(code), context);
    *size = sizeof(code);
}

void oracleisgoated_make_filter(BYTE *out, SIZE_T *size)
{
    BYTE code[] = {
        0x48, 0x85, 0xC9, 0x74, 0x41, 0x48, 0x8B, 0x01,
        0x48, 0x85, 0xC0, 0x74, 0x39, 0x8B, 0x00,
        0x3D, 0x06, 0x00, 0x01, 0x40, 0x74, 0x30,
        0x3D, 0x0A, 0x00, 0x01, 0x40, 0x74, 0x29,
        0x3D, 0x88, 0x13, 0x6D, 0x40, 0x74, 0x22,
        0x3D, 0x63, 0x73, 0x6D, 0xE0, 0x74, 0x1B,
        0x3D, 0x03, 0x00, 0x00, 0x80, 0x74, 0x14,
        0x3D, 0x1D, 0x00, 0x00, 0xC0, 0x74, 0x0D,
        0x3D, 0x04, 0x00, 0x00, 0x80, 0x74, 0x06,
        0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0x31, 0xC0, 0xC3
    };
    oracleisgoated_memcopy(out, code, sizeof(code));
    *size = sizeof(code);
}

int oracleisgoated_write_code_page(HANDLE process, uintptr_t address, const void *code, SIZE_T size)
{
    DWORD old = 0;
    BYTE zero[4096];
    oracleisgoated_memzero(zero, sizeof(zero));
    if (!VirtualProtectEx(process, (LPVOID)address, sizeof(zero), PAGE_EXECUTE_READWRITE, &old))
        return 0;
    if (!oracleisgoated_write_remote(process, address, zero, sizeof(zero)))
        return 0;
    if (!oracleisgoated_write_remote(process, address, code, size))
        return 0;
    FlushInstructionCache(process, (LPCVOID)address, size);
    return 1;
}

int oracleisgoated_is_writable(DWORD protect)
{
    DWORD p = protect & 0xFF;
    if (protect & (PAGE_GUARD | PAGE_NOACCESS))
        return 0;
    return p == PAGE_READWRITE || p == PAGE_WRITECOPY || p == PAGE_EXECUTE_READWRITE || p == PAGE_EXECUTE_WRITECOPY;
}

uintptr_t oracleisgoated_find_zero_cave(HANDLE process)
{
    BYTE *buffer = (BYTE *)oracleisgoated_heap_alloc(0x10000);
    if (!buffer)
        return 0;
    uintptr_t result = 0;
    uintptr_t address = 0;
    MEMORY_BASIC_INFORMATION mbi;
    while (VirtualQueryEx(process, (LPCVOID)address, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        uintptr_t base = (uintptr_t)mbi.BaseAddress;
        uintptr_t size = (uintptr_t)mbi.RegionSize;
        if (mbi.State == MEM_COMMIT && oracleisgoated_is_writable(mbi.Protect) && size >= 0x48)
        {
            uintptr_t offset = 0;
            while (offset < size)
            {
                SIZE_T chunk = (SIZE_T)((size - offset) > 0x10000 ? 0x10000 : (size - offset));
                SIZE_T read = 0;
                if (ReadProcessMemory(process, (LPCVOID)(base + offset), buffer, chunk, &read) && read >= 0x48)
                {
                    SIZE_T run = 0;
                    for (SIZE_T i = 0; i < read; ++i)
                    {
                        if (buffer[i] == 0)
                        {
                            ++run;
                            if (run == 0x48)
                            {
                                result = base + offset + i + 1 - 0x48;
                                goto done;
                            }
                        }
                        else
                        {
                            run = 0;
                        }
                    }
                }
                offset += chunk;
            }
        }
        if (base + size <= address)
            break;
        address = base + size;
    }
done:
    oracleisgoated_heap_free(buffer);
    return result;
}

int oracleisgoated_queue_tp_direct(HANDLE process, uintptr_t callback)
{
    HMODULE ntdll = oracleisgoated_get_module_local("ntdll.dll");
    oracleisgoated_NtQueryInformationProcess ntqip = (oracleisgoated_NtQueryInformationProcess)oracleisgoated_get_proc_local(ntdll, "NtQueryInformationProcess");
    oracleisgoated_NtQueryObject ntqo = (oracleisgoated_NtQueryObject)oracleisgoated_get_proc_local(ntdll, "NtQueryObject");
    oracleisgoated_NtSetIoCompletion ntsic = (oracleisgoated_NtSetIoCompletion)oracleisgoated_get_proc_local(ntdll, "NtSetIoCompletion");
    if (!ntqip || !ntqo || !ntsic)
        return 0;
    ULONG needed = 0;
    ULONG size = 0x20000;
    oracleisgoated_process_handle_snapshot *snapshot = 0;
    NTSTATUS status = 0;
    for (;;)
    {
        oracleisgoated_heap_free(snapshot);
        snapshot = (oracleisgoated_process_handle_snapshot *)oracleisgoated_heap_alloc(size);
        if (!snapshot)
            return 0;
        status = ntqip(process, 51, snapshot, size, &needed);
        if (status != (NTSTATUS)0xC0000004)
            break;
        size = needed ? needed + 0x1000 : size * 2;
    }
    if (status < 0 || !snapshot->NumberOfHandles)
    {
        oracleisgoated_heap_free(snapshot);
        return 0;
    }
    BYTE *typeBuffer = (BYTE *)oracleisgoated_heap_alloc(0x4000);
    if (!typeBuffer)
    {
        oracleisgoated_heap_free(snapshot);
        return 0;
    }
    int success = 0;
    HANDLE currentProcess = GetCurrentProcess();
    for (ULONG_PTR i = 0; i < snapshot->NumberOfHandles; ++i)
    {
        HANDLE duplicated = 0;
        if (!DuplicateHandle(process, snapshot->Handles[i].HandleValue, currentProcess, &duplicated, 0, FALSE, DUPLICATE_SAME_ACCESS))
            continue;
        oracleisgoated_memzero(typeBuffer, 0x4000);
        if (ntqo(duplicated, 2, typeBuffer, 0x4000, 0) >= 0)
        {
            oracleisgoated_unicode_string *typeName = (oracleisgoated_unicode_string *)typeBuffer;
            if (typeName->Buffer && oracleisgoated_wstreqi(typeName->Buffer, L"IoCompletion"))
            {
                uintptr_t cave = oracleisgoated_find_zero_cave(process);
                if (cave)
                {
                    BYTE fake[0x48];
                    oracleisgoated_memzero(fake, sizeof(fake));
                    oracleisgoated_memcopy(fake + 0x38, &callback, sizeof(callback));
                    if (oracleisgoated_write_remote(process, cave, fake, sizeof(fake)))
                    {
                        if (ntsic(duplicated, (PVOID)cave, 0, 0, 0) >= 0)
                            success = 1;
                    }
                }
            }
        }
        CloseHandle(duplicated);
        if (success)
            break;
    }
    oracleisgoated_heap_free(typeBuffer);
    oracleisgoated_heap_free(snapshot);
    return success;
}

int oracleisgoated_prepare_remote_payload(HANDLE process, const char *modulePath, uintptr_t *loaderAddress)
{
    uintptr_t kernel32Base = oracleisgoated_find_remote_module(process, L"kernel32.dll");
    uintptr_t winstaBase = oracleisgoated_find_remote_module(process, L"winsta.dll");
    if (!kernel32Base || !winstaBase)
        return 0;
    uintptr_t loadLibrary = oracleisgoated_resolve_remote_export(process, kernel32Base, "LoadLibraryA");
    uintptr_t setUnhandled = oracleisgoated_resolve_remote_export(process, kernel32Base, "SetUnhandledExceptionFilter");
    if (!loadLibrary || !setUnhandled)
        return 0;
    uintptr_t context = (uintptr_t)VirtualAllocEx(process, 0, 0x118, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!context)
        return 0;
    BYTE contextData[0x118];
    oracleisgoated_memzero(contextData, sizeof(contextData));
    SIZE_T pathLength = oracleisgoated_strlen(modulePath);
    if (pathLength > 0xFF)
        pathLength = 0xFF;
    oracleisgoated_memcopy(contextData, modulePath, pathLength);
    uintptr_t filterAddress = winstaBase + 0x2000;
    oracleisgoated_memcopy(contextData + 0x100, &loadLibrary, sizeof(loadLibrary));
    oracleisgoated_memcopy(contextData + 0x108, &setUnhandled, sizeof(setUnhandled));
    oracleisgoated_memcopy(contextData + 0x110, &filterAddress, sizeof(filterAddress));
    if (!oracleisgoated_write_remote(process, context, contextData, sizeof(contextData)))
        return 0;
    BYTE loader[128];
    BYTE filter[128];
    SIZE_T loaderSize = 0;
    SIZE_T filterSize = 0;
    oracleisgoated_memzero(loader, sizeof(loader));
    oracleisgoated_memzero(filter, sizeof(filter));
    oracleisgoated_make_loader(loader, &loaderSize, context);
    oracleisgoated_make_filter(filter, &filterSize);
    uintptr_t loaderRemote = winstaBase + 0x1000;
    if (!oracleisgoated_write_code_page(process, loaderRemote, loader, loaderSize))
        return 0;
    if (!oracleisgoated_write_code_page(process, filterAddress, filter, filterSize))
        return 0;
    *loaderAddress = loaderRemote;
    return 1;
}

int oracleisgoated_wait_for_module(HANDLE process, const wchar_t *moduleName, uintptr_t *base)
{
    for (;;)
    {
        uintptr_t found = oracleisgoated_find_remote_module(process, moduleName);
        if (found)
        {
            if (base)
                *base = found;
            return 1;
        }
        Sleep(50);
    }
}

int oracleisgoated_run()
{
    oracleisgoated_start_decompiler();
    char modulePath[MAX_PATH];
    oracleisgoated_memzero(modulePath, sizeof(modulePath));
    if (!oracleisgoated_build_module_path(modulePath, sizeof(modulePath)) || !oracleisgoated_file_exists(modulePath))
    {
        return 1;
    }
    DWORD pid = oracleisgoated_find_process_id(L"RobloxPlayerBeta.exe");
    if (!pid)
    {
        return 1;
    }
    HANDLE process = OpenProcess(0x1FFFFE, FALSE, pid);
    if (!process)
    {
        return 1;
    }
    if (oracleisgoated_find_remote_module(process, L"Module.dll"))
    {
        CloseHandle(process);
        return 0;
    }
    oracleisgoated_apply_dacl(modulePath, "D:(A;;FA;;;WD)");
    oracleisgoated_remote_list_state listState;
    oracleisgoated_hook_state hookState;
    oracleisgoated_memzero(&listState, sizeof(listState));
    oracleisgoated_memzero(&hookState, sizeof(hookState));
    uintptr_t remoteNtdll = oracleisgoated_find_remote_module(process, L"ntdll.dll");
    if (!remoteNtdll || !oracleisgoated_unlink_remote_notifications(process, remoteNtdll, &listState))
    {
        CloseHandle(process);
        return 1;
    }
    if (!oracleisgoated_patch_nt_create_section(process, remoteNtdll, &hookState))
    {
        oracleisgoated_restore_remote_notifications(process, &listState);
        CloseHandle(process);
        return 1;
    }
    uintptr_t loader = 0;
    if (!oracleisgoated_prepare_remote_payload(process, modulePath, &loader))
    {
        oracleisgoated_restore_nt_create_section(process, &hookState);
        oracleisgoated_restore_remote_notifications(process, &listState);
        CloseHandle(process);
        return 1;
    }
    if (!oracleisgoated_queue_tp_direct(process, loader))
    {
        oracleisgoated_restore_nt_create_section(process, &hookState);
        oracleisgoated_restore_remote_notifications(process, &listState);
        CloseHandle(process);
        return 1;
    }
    uintptr_t moduleBase = 0;
    oracleisgoated_wait_for_module(process, L"Module.dll", &moduleBase);
    oracleisgoated_restore_nt_create_section(process, &hookState);
    oracleisgoated_restore_remote_notifications(process, &listState);
    oracleisgoated_apply_dacl(modulePath, "D:(A;;FA;;;WD)");
    oracleisgoated_print("injected\r\n");
    CloseHandle(process);
    return 0;
}

// extern "C" void oracleisgoated_entry() ahhhh cheeky boyy oracle
// {
    // ExitProcess((UINT)oracleisgoated_run());
// }

int main()
{
    return (int)oracleisgoated_run();
}