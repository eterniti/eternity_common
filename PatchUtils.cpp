#include <windows.h>
#include <MinHook.h>
#include <vector>
#include "PatchUtils.h"
#include "debug.h"

#define MB_1    0x100000
#define GB_2    0x80000000

void *PatchUtils::GetPtr(size_t rel_address, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return nullptr;
	
	return (void *)(mod_top+rel_address);
}

ptrdiff_t PatchUtils::RelAddress(void *ptr, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return -1;
	
	return (uint8_t *)ptr - mod_top;
}

uint8_t PatchUtils::Read8(size_t rel_address, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return 0;
	
	return *(mod_top+rel_address);
}

uint16_t PatchUtils::Read16(size_t rel_address, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return 0;
	
	return *(uint16_t *)(mod_top+rel_address);
}

uint32_t PatchUtils::Read32(size_t rel_address, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return 0;
	
	return *(uint32_t *)(mod_top+rel_address);
}

uint64_t PatchUtils::Read64(size_t rel_address, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return 0;
	
	return *(uint64_t *)(mod_top+rel_address);
}

void PatchUtils::Write8(void *address, uint8_t data)
{
	DWORD lOldProtect;
	
	VirtualProtect(address, sizeof(uint8_t), PAGE_EXECUTE_READWRITE, &lOldProtect);
	*(uint8_t *)address = data;
	VirtualProtect(address, sizeof(uint8_t), lOldProtect, &lOldProtect);
}

bool PatchUtils::Write8(size_t rel_address, uint8_t data, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + rel_address;
	Write8(address, data);
	return true;
}

void PatchUtils::Write16(void *address, uint16_t data)
{
	DWORD lOldProtect;
	
	VirtualProtect(address, sizeof(uint16_t), PAGE_EXECUTE_READWRITE, &lOldProtect);
	*(uint16_t *)address = data;
	VirtualProtect(address, sizeof(uint16_t), lOldProtect, &lOldProtect);
}

bool PatchUtils::Write16(size_t rel_address, uint16_t data, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + rel_address;
	Write16(address, data);
	return true;
}

void PatchUtils::Write32(void *address, uint32_t data)
{
	DWORD lOldProtect;
	
	VirtualProtect(address, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &lOldProtect);
	*(uint32_t *)address = data;
	VirtualProtect(address, sizeof(uint32_t), lOldProtect, &lOldProtect);
}

bool PatchUtils::Write32(size_t rel_address, uint32_t data, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + rel_address;
	Write32(address, data);
	return true;
}

void PatchUtils::Write64(void *address, uint64_t data)
{
	DWORD lOldProtect;
	
	VirtualProtect(address, sizeof(uint64_t), PAGE_EXECUTE_READWRITE, &lOldProtect);
	*(uint64_t *)address = data;
	VirtualProtect(address, sizeof(uint64_t), lOldProtect, &lOldProtect);
}

bool PatchUtils::Write64(size_t rel_address, uint64_t data, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + rel_address;
	Write64(address, data);
	return true;
}

void PatchUtils::Copy(void *dst, void *src, size_t size)
{
	DWORD lOldProtect;
	
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &lOldProtect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, lOldProtect, &lOldProtect);
}

bool PatchUtils::Copy(size_t dst, void *src, size_t size, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + dst;
	Copy(address, src, size);
	return true;
}

bool PatchUtils::Copy(void *dst, size_t src, size_t size, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + src;
	Copy(dst, address, size);
	return true;
}

void PatchUtils::Fill(void *dst, uint8_t value, size_t size)
{
	DWORD lOldProtect;
	
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &lOldProtect);
	memset(dst, value, size);
	VirtualProtect(dst, size, lOldProtect, &lOldProtect);
}

bool PatchUtils::Fill(size_t dst, uint8_t value, size_t size, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + dst;
	Fill(address, value, size);
	return true;
}

bool PatchUtils::Hook(void *address, void **orig, void *new_func)
{	
    static bool mh_inited = false;

    if (!mh_inited)
    {
        if (MH_Initialize() != MH_OK)
        {
            //DPRINTF("Init failed.\n");
            return false;
        }

        mh_inited = true;
    }

    //DPRINTF("Hook %p -> %p\n", address, new_func);

    int err;
    if ((err = MH_CreateHook(address, new_func, orig)) != MH_OK)
    {
        //DPRINTF("Failed createhook. %d\n", err);
        return false;
    }

    return (MH_EnableHook(address) == MH_OK);
}

bool PatchUtils::Hook(size_t rel_address, void **orig, void *new_func, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return false;
	
	uint8_t *address = mod_top + rel_address;
    return Hook(address, orig, new_func);
}

bool PatchUtils::Hook(const char *mod, const char *func, void **orig, void *new_func)
{
   HMODULE hMod = GetModuleHandleA(mod);
   if (!hMod)
       return false;

   void *address = (void *)GetProcAddress(hMod, func);
   if (!address)
       return false;

   return Hook(address, orig, new_func);
}

void *PatchUtils::FindImport(const char *import_mod, const char *import_func, const char *mod, bool is_ordinal)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
    if (!mod_top)
        return nullptr;

    IMAGE_DOS_HEADER *dos_hdr = (IMAGE_DOS_HEADER *)mod_top;
    IMAGE_NT_HEADERS *nt_hdr = (IMAGE_NT_HEADERS *)(mod_top + dos_hdr->e_lfanew);
    IMAGE_IMPORT_DESCRIPTOR	*import_dir = (IMAGE_IMPORT_DESCRIPTOR *)(mod_top + nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (import_dir->Characteristics)
    {
        const char *name = (const char *)(mod_top + import_dir->Name);

        if (strcasecmp(name, import_mod) == 0)
        {
            IMAGE_THUNK_DATA *thunk_data = (IMAGE_THUNK_DATA *)(mod_top + import_dir->OriginalFirstThunk);
            uintptr_t *iat = (uintptr_t *)(mod_top + import_dir->FirstThunk);

            while (thunk_data->u1.Ordinal)
            {
                if (!IMAGE_SNAP_BY_ORDINAL(thunk_data->u1.Ordinal))
                {
                    if (!is_ordinal)
					{
						IMAGE_IMPORT_BY_NAME *import_info = (IMAGE_IMPORT_BY_NAME *)(mod_top + thunk_data->u1.AddressOfData);

						if (strcasecmp((char *)import_info->Name, import_func) == 0)
							return iat;
					}
                }
				else 
				{
					if (is_ordinal && ((uint32_t)thunk_data->u1.Ordinal == (uint32_t)(uintptr_t)import_func))
						return iat;
				}

                thunk_data++;
                iat++;
            }

            return nullptr;
        }

        import_dir++;
    }

    return nullptr;
}

bool PatchUtils::HookImport(const char *import_mod, const char *import_func, void *new_func, const char *mod, bool is_ordinal)
{
    void *addr = FindImport(import_mod, import_func, mod, is_ordinal);
    if (!addr)
        return false;

#ifdef CPU_X86_64
    Write64(addr, (uint64_t)new_func);
#else
    Write32(addr, (uint32_t)new_func);
#endif

    return true;
}

bool PatchUtils::Unhook(void *address)
{
    return (MH_DisableHook(address) == MH_OK);
}

bool PatchUtils::Unhook(size_t rel_address, const char *mod)
{
    uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
    if (!mod_top)
        return false;

    uint8_t *address = mod_top + rel_address;
    return Unhook(address);
}

bool PatchUtils::Unhook(const char *mod, const char *func)
{
    HMODULE hMod = GetModuleHandleA(mod);
    if (!hMod)
        return false;

    void *address = (void *)GetProcAddress(hMod, func);
    if (!address)
        return false;

    return Unhook(address);
}

#ifdef CPU_X86

bool PatchUtils::HookCall(void *call_addr, void **orig, void *new_addr)
{
    uint8_t *ptr = (uint8_t *)call_addr;
    if (*ptr != 0xE8 && *ptr != 0xE9)
        return false;

    int32_t rel = *(int32_t *)(ptr+1);
    uint32_t target = (uint32_t)new_addr - (uint32_t)call_addr - 5;

    if (orig)
		*orig = (void *) ((uint32_t)call_addr + rel + 5);
	
    Write32(ptr+1, target);

    return true;
}

#else

bool PatchUtils::HookCall(void *call_addr, void **orig, void *new_addr, const std::vector<uint8_t> &add_code)
{
    uint8_t *ptr = (uint8_t *)call_addr;
	
	if (*ptr == 0xFF && *(ptr+1) == 0x15)
	{
		if (orig)
		{
			uint32_t disp = 6 + *(uint32_t *)&ptr[2];
			uint64_t *address = (uint64_t *)(ptr + disp);
			*orig = (void *) *address;			
		}
		
		orig = nullptr;
		Write8(ptr, 0xE8);
		Write8(ptr+5, 0x090);
		// now, we can use the other implementation
	}
	
    if (*ptr != 0xE8 && *ptr != 0xE9)
        return false;

    size_t addr = Utils::Align2((size_t)call_addr, 0x100000);
    size_t top;
    size_t down;
	
	if (addr > 0xFFFFFFFF800FFFFF)
		top = addr;
	else
		top = addr + 0x7FF00000;
	
	if (addr < 0x7FF00000)
		down = 0x400000;
	else
		down = addr - 0x7FF00000;
	
    uint8_t *tramp = nullptr;

    for (addr = down; addr < top; addr += 0x100000)
    {
        tramp = (uint8_t *)VirtualAlloc((void *)addr, 0x1000, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (tramp)
            break;
    }

    if (!tramp)
        return false;
	
	if (add_code.size() > 0)
		memcpy(tramp, add_code.data(), add_code.size());

    size_t idx = add_code.size();
	
	tramp[idx] = 0xFF;
    tramp[idx+1] = 0x25;
    *(uint32_t *)&tramp[idx+2] = 0;
    *(uint64_t *)&tramp[idx+6] = (uint64_t)new_addr;

    int32_t rel = *(int32_t *)(ptr+1);
    uint32_t target = (uint32_t) ((uint64_t)tramp - (uint64_t)call_addr - 5);

    if (orig)
		*orig = (void *) ((uint64_t)call_addr + (int64_t)rel + 5);
	
    Write32(ptr+1, target);

    return true;
}

bool PatchUtils::HookCall(void *call_addr, void **orig, void *new_addr)
{
	return HookCall(call_addr, orig, new_addr, { });
}

#endif

bool PatchUtils::HookResolveTarget(void *call_addr, void **orig, void *new_addr, bool expect_call)
{
	uint8_t *ptr = (uint8_t *)call_addr;
	
	if (expect_call)
	{
		if (*ptr != 0xE8 && *ptr != 0xE9)
			return false;
		
		ptr++;
	}
	
	int32_t rel = *(int32_t *)ptr;
	void *func = (void *) ((uintptr_t)ptr + (intptr_t)rel + 4);
	
	return Hook(func, orig, new_addr);
}

#ifdef CPU_X86_64

void *PatchUtils::AllocateIn32BitsArea(void *ref_addr, size_t size, bool executable)
{
    uint8_t *top_addr = (uint8_t *)ref_addr+GB_2;
    uint8_t *bottom_addr = (uint8_t *)ref_addr-GB_2;

    for (uint8_t *test_addr = (uint8_t *)ref_addr + MB_1; test_addr < top_addr; test_addr += MB_1)
    {
        void *buf = VirtualAlloc(test_addr, size, MEM_COMMIT|MEM_RESERVE, (executable) ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
        if (buf)
            return buf;
    }

    for (uint8_t *test_addr = (uint8_t *)ref_addr - MB_1; test_addr > bottom_addr; test_addr -= MB_1)
    {
        void *buf = VirtualAlloc(test_addr, size, MEM_COMMIT|MEM_RESERVE, (executable) ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
        if (buf)
            return buf;
    }

    return nullptr;
}

#endif

struct MemoryBreakpoint
{
    size_t address;
    size_t size;
    size_t page_address;
    size_t page_size;
    DWORD old_prot;
    MemoryBreakpointHandler handler;
};

static std::vector<MemoryBreakpoint> mbps; 

LONG CALLBACK mbp_handler(PEXCEPTION_POINTERS ExceptionInfo)
{
    //DPRINTF("***mbp_handler %p\n", (void *)ExceptionInfo->ExceptionRecord->ExceptionAddress);
    static size_t last_index = 0;

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
    {
        //DPRINTF("***mbp_handler single_step %p \n", (void *)ExceptionInfo->ExceptionRecord->ExceptionAddress);

        DWORD old;
        VirtualProtect((void *)mbps[last_index].page_address, (DWORD)mbps[last_index].page_size, PAGE_NOACCESS, &old);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    else if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
    {
        //DPRINTF("***mbp_handler access violation %p \n", (void *)ExceptionInfo->ExceptionRecord->ExceptionAddress);

        void *pc = (void *)ExceptionInfo->ExceptionRecord->ExceptionAddress;

        if (ExceptionInfo->ExceptionRecord->NumberParameters >= 2)
        {
           size_t addr = (size_t)ExceptionInfo->ExceptionRecord->ExceptionInformation[1];

           for (size_t i = 0; i < mbps.size(); i++)
           {
               const MemoryBreakpoint &mbp = mbps[i];

               if (addr >= mbp.page_address && addr < (mbp.page_address+mbp.page_size))
               {
                    for (size_t j = 0; j < mbps.size(); j++)
                    {
                        const MemoryBreakpoint &mbp2 = mbps[j];

                        if (addr >= mbp2.address && addr < (mbp2.address+mbp2.size))
                        {
                            mbp2.handler(pc, (void *)addr);
                            break;
                        }
                    }

                    DWORD old;
                    VirtualProtect((void *)mbp.page_address, (DWORD)mbp.page_size, mbp.old_prot, &old);

                    ExceptionInfo->ContextRecord->EFlags |= 0x100;
                    last_index = i;
                    return EXCEPTION_CONTINUE_EXECUTION;
               }
           }
        }        
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

bool PatchUtils::SetMemoryBreakpoint(void *addr, size_t len, MemoryBreakpointHandler handler)
{
    static bool handler_created = false;

    if (!handler_created)
    {
        if (!AddVectoredExceptionHandler(1, mbp_handler))
            return false;

        handler_created = true;
    }

    MemoryBreakpoint mbp;
    MEMORY_BASIC_INFORMATION info;

    if (!VirtualQuery(addr, &info, sizeof(info)))
    {
        //DPRINTF("Vq failed %x.\n", GetLastError());
        return false;
    }

    mbp.page_address = (size_t) info.BaseAddress;
    size_t end_addr = Utils::Align2((size_t)addr + len, 4096);
    mbp.page_size = end_addr - mbp.page_address;

    if (!VirtualProtect((void *)mbp.page_address, (DWORD)mbp.page_size, PAGE_NOACCESS, &mbp.old_prot))
    {
        //DPRINTF("Vp failed %x.\n", GetLastError());
        return false;
    }

    mbp.address = (size_t)addr;
    mbp.size = len;
    mbp.handler = handler;

    mbps.push_back(mbp);
    return true;
}

bool PatchUtils::UnsetMemoryBreakpoint(void *addr, size_t len)
{
	for (size_t i = 0; i < mbps.size(); i++)
	{
		if ((size_t)addr == mbps[i].address && len == mbps[i].size)
		{
			DWORD old;
			VirtualProtect((void *)mbps[i].page_address, (DWORD)mbps[i].page_size, mbps[i].old_prot, &old);
			
			mbps.erase(mbps.begin()+i);
			return true;
		}
	}
	
	return false;
}

#ifdef CPU_X86_64 

typedef uintptr_t (* RegisterFunctionType)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);

uintptr_t PatchUtils::InvokeRegisterFunction(uintptr_t address, uintptr_t rcx, uintptr_t rdx, uintptr_t r8, uintptr_t r9, const char *mod)
{
	uint8_t *mod_top = (uint8_t *)GetModuleHandleA(mod);
	if (!mod_top)
		return 0;
	
	RegisterFunctionType func = (RegisterFunctionType)(mod_top+address);
	return func(rcx, rdx, r8, r9);
}

uintptr_t PatchUtils::InvokeVirtualRegisterFunction(void *obj, size_t ofs, uintptr_t rdx, uintptr_t r8, uintptr_t r9)
{
	uintptr_t *vtable = (uintptr_t *) *(uintptr_t *)obj;
	RegisterFunctionType func = (RegisterFunctionType) vtable[ofs/8];

	return func((uintptr_t)obj, rdx, r8, r9);
}

#endif

