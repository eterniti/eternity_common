#include <windows.h>

#include "EpatchFile.h"
#include "PatchUtils.h"
#include "debug.h"

#define WILDCARD    0x777
#define DEFAULT_SEARCH_DOWN 0x200000
#define DEFAULT_SEARCH_UP   0x200000

#define LOG_PARAMS_LIMIT    10

using namespace PatchUtils;

#define CSTR(s) ((s.length() == 0) ? nullptr : s.c_str())
#define REL_ADDR32(a) ((uint32_t)(size_t)a - (uint32_t)(size_t)GetModuleHandleA(CSTR(module)))
#define REL_ADDR32_2(a, p) ((uint32_t)(size_t)a - (uint32_t)(size_t)GetModuleHandleA(CSTR(p->module)))

#define BUF_SIZE_VCSTR	16

struct VCStdString
{	
	union
	{
		char buf[BUF_SIZE_VCSTR]; // For allocation_size < BUF_SIZE_STR
		char *ptr; // For allocation_size >= BUF_SIZE_STR		
	} str; 
	
	size_t length; 
	size_t allocation_size; 
	
	VCStdString()
	{
		str.ptr = nullptr;
		length = 0;
		allocation_size = 0;
	}
	
	inline const char *CStr() const
	{
		if (allocation_size >= BUF_SIZE_VCSTR)
		{
			return str.ptr;
		}
		
		return str.buf;
	}
};
#ifdef CPU_X86_64
CHECK_STRUCT_SIZE(VCStdString, 0x20);
#endif

// TODO: VCStdString size for 32 bits

typedef void (* SETUP_FUNCTION)(void *);

static bool build_pattern(const std::string &c_code, std::vector<uint16_t> &pattern)
{
    std::string code = c_code;

    for (size_t i = 0; i < code.length(); i++)
    {
        if (code[i] == ' ')
        {
            code.erase(code.begin()+i);
            i--;
        }
    }

    size_t len = code.length();
    if (len == 0 || (len&1))
        return false;

    pattern.resize(len / 2);

    for (size_t i = 0; i < pattern.size(); i++)
    {
        char high, low;

        high = code[i*2];
        low = code[(i*2)+1];

        if (high == 'X' || high == 'x')
        {
            if (low != 'X' && low != 'x')
                return false;

            pattern[i] = WILDCARD;
        }
        else
        {
            uint8_t byte;

            if (high >= '0' && high <= '9')
                byte = (high-'0');
            else if (high >= 'A' && high <= 'F')
                byte = high-'A'+0xA;
            else if (high >= 'a' && high <= 'f')
                byte = high-'a'+0xA;
            else
                return false;

            byte <<= 4;

            if (low >= '0' && low <= '9')
                byte |= (low-'0');
            else if (low >= 'A' && low <= 'F')
                byte |= low-'A'+0xA;
            else if (low >= 'a' && low <= 'f')
                byte |= low-'a'+0xA;
            else
                return false;

            pattern[i] = byte;
        }
    }

    return true;
}

bool EInstruction::Compile(const TiXmlElement *root)
{
    std::string code;

    if (!Utils::ReadAttrString(root, "code", code))
        return false;

    if (!build_pattern(code, search_pattern))
        return false;

    if (!Utils::ReadAttrString(root, "comment", comment))
        comment.clear();

    return true;
}

std::vector<EPatch> EPatch::log_patches;
Mutex EPatch::log_mutex;

// Keep lower case
static const std::vector<std::string> epatch_types =
{
    "write",
    "nop",
    "hook",
    "log",
    "notify",
};

static const std::vector<std::string> hook_types =
{
    "normal",
    "call",
	"resolve_target",
	"direct_target",
};

static const std::vector<std::string> log_param_types =
{
    "u8",
    "u16",
    "u32",
    "u64",
    "s8",
    "s16",
    "s32",
    "s64",
    "float",
    "double",
    "cstr",
    "wstr",
	"vcstr",
    "ptr",
};

static const std::vector<std::string> calling_conventions =
{
    "cdecl",
    "stdcall",
    "thiscall",
    "fastcall"
};

bool EPatch::IsPattern(size_t address, const std::vector<uint16_t> &pattern) 
{
    size_t length = pattern.size();
    uint8_t *module_top = (uint8_t *)GetModuleHandleA(CSTR(module));

    for (size_t i = 0; i < length; i++)
    {
        if (pattern[i] >= 0x100)
            continue;

        if (module_top[address+i] != pattern[i])
            return false;
    }
	
	if (num_matches > 1)
	{	
		if (block_addresses.find(address) != block_addresses.end())
			return false;
		
		block_addresses.insert(address);
	}
	
    return true;
}

uint8_t *EPatch::Find()
{
    if (type >= EPATCH_TYPE_MAX)
        return nullptr;

    if (rebuild)
    {
        if (instructions.size() == 0)
            return nullptr;

        size_t total_size = 0;
        for (const EInstruction &ins : instructions)
        {
            total_size += ins.search_pattern.size();
        }

        search_pattern.resize(total_size);
        uint16_t *data = search_pattern.data();

        //DPRINTF("total_size = %I64x\n", total_size);

        for (const EInstruction &ins : instructions)
        {
            memcpy(data, ins.search_pattern.data(), ins.search_pattern.size()*sizeof(uint16_t));
            data += ins.search_pattern.size();
        }

        rebuild = false;
    }

    size_t address_lowest, address_highest;
    size_t address_down, address_up;

    address_lowest = (search_start < search_down) ? 0 : search_start-search_down;
    address_highest = search_start+search_up;
	
	if (address_lowest < 0x1000)
		address_lowest = 0x1000; // Possible fix for possible crash

    address_down=search_start;
    address_up=search_start+1;

    bool search_down_end = (address_down==address_lowest);
    bool search_up_end = (address_up==address_highest);

    while (!search_down_end || !search_up_end)
    {
        if (!search_down_end)
        {
            if (IsPattern(address_down, search_pattern))
            {				
				return (uint8_t *)GetPtr(address_down, CSTR(module));
            }

            address_down--;
            if (address_down == address_lowest)
                search_down_end = true;
        }

        if (!search_up_end)
        {
            if (IsPattern(address_up, search_pattern))
            {
                return (uint8_t *)GetPtr(address_up, (CSTR(module)));
            }

            address_up++;
            if (address_up == address_highest)
                search_up_end = true;
        }
    }

    return nullptr;
}

void EPatch::AddLogPatch()
{
    log_patches.push_back(*this);
}

EPatch *EPatch::FindLogPatch(void *addr)
{
    for (EPatch &patch : log_patches)
    {
        if (patch.log_func == addr)
            return &patch;
    }

    return nullptr;
}

void EPatch::LogParam(size_t param, int index)
{
    if (index >= 0 && (size_t)index >= log_select.size())
        return;	

    if (index >= 0 && !log_select[index])
        return;
	
    if (index < 0 && log_result_type < 0)
        return;

    int extra = (index >= 0) ? log_extra[index] : log_result_extra;
    const char *param_name = (index >= 0) ? log_params[index].c_str() : "result";
    const std::vector<int> &deep = (index >= 0) ? log_deep[index] : log_result_deep;

    for (size_t i = 0; i < deep.size(); i++)
    {
        if (param == 0)
        {
            DPRINTF("%s: <NULL>\n", param_name);
            return;
        }

        param = (size_t) ( ((uint8_t *)param) + deep[i] );
        param = *(size_t *)param;
    }
	
	int type = (index >= 0) ? log_types[index] : log_result_type;

    switch (type)
    {
        case LOG_PARAM_TYPE_U8:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %u\n", param_name, (uint8_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%02x\n", param_name, (uint8_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%x\n", param_name, (uint8_t)param);
            }
        }
        break;

        case LOG_PARAM_TYPE_U16:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %u\n", param_name, (uint16_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%04x\n", param_name, (uint16_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%x\n", param_name, (uint16_t)param);
            }
        }
        break;

        case LOG_PARAM_TYPE_U32:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %u\n", param_name, (uint32_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%08x\n", param_name, (uint32_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%x\n", param_name, (uint32_t)param);
            }
        }
        break;

#ifndef CPU_X86
        case LOG_PARAM_TYPE_U64:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %I64u\n", param_name, param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%016I64x\n", param_name, param);
            }
            else
            {
                DPRINTF("%s: 0x%I64x\n", param_name, param);
            }
        }
        break;
#endif

        case LOG_PARAM_TYPE_S8:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %d\n", param_name, (int8_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%02x\n", param_name, (int8_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%x\n", param_name, (int8_t)param);
            }
        }
        break;

        case LOG_PARAM_TYPE_S16:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %d\n", param_name, (int16_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%04x\n", param_name, (int16_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%x\n", param_name, (int16_t)param);
            }
        }
        break;

        case LOG_PARAM_TYPE_S32:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %d\n", param_name, (int32_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%08x\n", param_name, (int32_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%x\n", param_name, (int32_t)param);
            }
        }
        break;
		
#ifndef CPU_X86
        case LOG_PARAM_TYPE_S64:
        {
            if (extra == LOG_EXTRA_DECIMAL)
            {
                DPRINTF("%s: %I64d\n", param_name, (int64_t)param);
            }
            else if (extra == LOG_EXTRA_HEXADECIMAL_ALIGNED)
            {
                DPRINTF("%s: 0x%016I64x\n", param_name, (int64_t)param);
            }
            else
            {
                DPRINTF("%s: 0x%I64x\n", param_name, (int64_t)param);
            }
        }
        break;		
#endif

        case LOG_PARAM_TYPE_CSTR:
            DPRINTF("%s: \"%s\"\n", param_name, (const char *)param);
        break;

        case LOG_PARAM_TYPE_WSTR:
            DPRINTF("%s: \"%S\"\n", param_name, (const wchar_t *)param);
        break;
		
		case LOG_PARAM_TYPE_VCSTR:			
			DPRINTF("%s: \"%s\"\n", param_name, ((VCStdString *)param)->CStr());			
		break;

        case LOG_PARAM_TYPE_PTR:
        {
            if (param == 0)
            {
                DPRINTF("%s: <NULL>\n", param_name);
                break;
            }

            int size = (index >= 0) ? log_ptr_sizes[index] : log_result_ptr_size;
            int num_lines = size / extra;

            if ((size % extra) != 0)
                num_lines++;

            if (num_lines == 1)
                DPRINTF("%s: ", param_name);
            else
                DPRINTF("%s:\n", param_name);

            uint8_t *data = (uint8_t *)param;
            std::string str;
            for (int i = 0; i < size; i++)
            {
                char temp[4];
                sprintf(temp, "%02X ", data[i]);
                str += temp;
                //DPRINTF("%02X ", data[i]);

                if (i == (size-1) || (i != 0 && ((i+1) % extra) == 0))
                {
                    //DPRINTF("\n");
                    str += '\n';
                }
            }

            DPRINTF("%s", str.c_str());
        }
        break;
    }
}

#define CALLING_CONVENTION			__cdecl
#define SUFFIX_CONVENTION				
#define SUFFIX_CONVENTION_UPPER		

#define NUM_PARAMS 0
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER

#ifdef CPU_X86

#define CALLING_CONVENTION			__stdcall
#define SUFFIX_CONVENTION			_Stdcall			
#define SUFFIX_CONVENTION_UPPER		_STDCALL

#define NUM_PARAMS 0
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER

#define CALLING_CONVENTION			__thiscall
#define SUFFIX_CONVENTION			_Thiscall			
#define SUFFIX_CONVENTION_UPPER		_THISCALL

#define NUM_PARAMS 0
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER

#define CALLING_CONVENTION			__fastcall
#define SUFFIX_CONVENTION			_Fastcall			
#define SUFFIX_CONVENTION_UPPER		_FASTCALL

#define NUM_PARAMS 0
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER


#endif // CPU_X86

int EPatch::ParseHookType(const std::string &type)
{
    int hook_type = HOOK_TYPE_MAX;

    for (size_t i = 0; i < hook_types.size(); i++)
    {
        if (Utils::ToLowerCase(type) == hook_types[i])
        {
            hook_type = (int)i;
            break;
        }
    }

    return hook_type;
}

static bool GetLogType(const std::string &str_type, int &ptype)
{
	ptype  = LOG_PARAM_TYPE_MAX;
	
	for (size_t j = 0 ; j < log_param_types.size(); j++)
	{
		if (str_type == log_param_types[j])
		{
			ptype = (int)j;
			break;
		}
	}

	if (ptype >= LOG_PARAM_TYPE_MAX)
		return false;

#ifdef CPU_X86

	if (ptype == LOG_PARAM_TYPE_S64 || ptype == LOG_PARAM_TYPE_U64)
		return false;
#endif

	if (ptype == LOG_PARAM_TYPE_FLOAT || ptype == LOG_PARAM_TYPE_DOUBLE)
	{
		DPRINTF("%s: float and double params currently not supported.\n", FUNCNAME);
		return false;
	}
	
	return true;
}

static void GetLogExtra(const std::string &modifier, int type, int &extra, int &ptr_size)
{
	if (type < LOG_PARAM_TYPE_PTR)
	{
		if (modifier == "d")
			extra = LOG_EXTRA_DECIMAL;
		else if (modifier == "ha")
			extra = LOG_EXTRA_HEXADECIMAL_ALIGNED;
		else
			extra = LOG_EXTRA_HEXADECIMAL;
	}
	else
	{
		size_t split = modifier.find(':');

		if (split == std::string::npos)
		{
			ptr_size = Utils::GetUnsigned(modifier);
			extra = 16;
		}
		else
		{
			ptr_size =  Utils::GetUnsigned(modifier.substr(0, split));
			extra = Utils::GetUnsigned(modifier.substr(split+1));
		}

		if (extra == 0)
			extra = 16;

		if (ptr_size == 0)
			ptr_size = 16;
	}
}

bool EPatch::Compile(const TiXmlElement *root)
{
    std::string type_str;

    if (!Utils::ReadAttrString(root, "type", type_str))
        return false;

    type = EPATCH_TYPE_MAX;
    for (size_t i = 0 ; i < epatch_types.size(); i++)
    {
        if (Utils::ToLowerCase(type_str) == epatch_types[i])
        {
            type = (int)i;
            break;
        }
    }

    if (type >= EPATCH_TYPE_MAX)
        return false;

    if (!Utils::ReadAttrString(root, "name", name))
        return false;

    if (!Utils::ReadAttrString(root, "comment", comment))
        comment.clear();

    if (!Utils::ReadAttrString(root, "module", module))
        module.clear();

    if (!Utils::ReadAttrString(root, "enabled", enabled) || enabled.length() == 0)
        enabled = "true";

    if (!Utils::ReadAttrUnsigned(root, "search_start", &search_start))
        return false;

    if (!Utils::ReadAttrUnsigned(root, "search_down", &search_down))
        search_down = DEFAULT_SEARCH_DOWN;

    if (!Utils::ReadAttrUnsigned(root, "search_up", &search_up))
        search_up = DEFAULT_SEARCH_UP;
	
    if (!Utils::ReadAttrUnsigned(root, "matches", &num_matches))
        num_matches = 1;

    size_t inst_count = Utils::GetElemCount(root, "Instruction");
    if (inst_count == 0)
        return false;

    instructions.resize(inst_count);

    size_t idx = 0;
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Instruction")
        {
            EInstruction &instruction = instructions[idx++];

            if (!instruction.Compile(elem))
                return false;
        }
    }

    const TiXmlElement *common = nullptr;

    if (type == EPATCH_TYPE_WRITE)
    {
        std::string data;
        const TiXmlElement *write;

        if (!Utils::GetParamString(root, "Write", data, &write))
            return false;

        if (!build_pattern(data, replace_pattern))
            return false;

        common = write;
    }
    else if (type == EPATCH_TYPE_NOP)
    {
        const TiXmlElement *nop;

        if (Utils::GetElemCount(root, "Nop", &nop) == 0)
            return false;

        if (!Utils::ReadAttrUnsigned(nop, "size", &size))
            size = 0;

        common = nop;
    }
    else if (type == EPATCH_TYPE_HOOK)
    {
        const TiXmlElement *hook;
        std::string hook_type_str;

        if (!Utils::GetParamString(root, "Hook", function, &hook) || function.length() == 0)
            return false;

        if (!Utils::ReadAttrString(hook, "module", function_module))
            function_module = patcher_module;

        if (!Utils::ReadAttrString(hook, "setup", setup_function))
            setup_function.clear();

        if (Utils::ReadAttrString(hook, "type", hook_type_str))
        {
            hook_type = ParseHookType(hook_type_str);

            if (hook_type >= HOOK_TYPE_MAX)
                return false;
        }
        else
        {
            hook_type = HOOK_TYPE_NORMAL;
        }

        common = hook;
    }
    else if (type == EPATCH_TYPE_LOG)
    {
        const TiXmlElement *log;
        std::string when;
        std::string cc;
        std::string ra;
        std::string hook_type_str;

        if (!Utils::GetParamMultipleStrings(root, "Log", log_params, &log))
            return false;

        if (log_params.size() > LOG_PARAMS_LIMIT)
        {
            DPRINTF("%s: No more than %d parameters currently supported.\n", FUNCNAME, LOG_PARAMS_LIMIT);
            return false;
        }

        if (Utils::ReadAttrString(log, "hook_type", hook_type_str))
        {
            hook_type = ParseHookType(hook_type_str);

            if (hook_type >= HOOK_TYPE_MAX)
                return false;
        }
        else
        {
            hook_type = HOOK_TYPE_NORMAL;
        }

        if (Utils::ReadAttrString(log, "ra", ra) && Utils::ToLowerCase(ra) == "true")
        {
            log_ra = true;
        }
        else
        {
            log_ra = false;
        }

        if (!Utils::ReadAttrString(log, "function", function))
            return false;

        if (Utils::ReadAttrString(log, "calling_convention", cc) && cc.length() != 0)
        {
            calling_convention = LOG_CC_MAX;
            for (size_t i = 0 ; i < calling_conventions.size(); i++)
            {
                if (Utils::ToLowerCase(cc) == calling_conventions[i])
                {
                    calling_convention = (int)i;
                    break;
                }
            }

            if (calling_convention >= LOG_CC_MAX)
                return false;

#ifndef CPU_X86
            calling_convention = LOG_CC_CDECL;
#endif
        }
        else
        {
            calling_convention = LOG_CC_CDECL;
        }

        if (Utils::ReadAttrString(log, "when", when) && Utils::ToLowerCase(when) == "after")
        {
            log_before = false;
        }
        else
        {
            log_before = true;
        }
		
		// RESULT LOG
		{
			std::string result_type;
		
			if (!Utils::ReadAttrString(log, "result", result_type) || result_type.length() == 0)
			{
				log_result_type = -1; 
			}
			else
			{
				
				std::vector<std::string> deep;
                Utils::GetMultipleStrings(result_type, deep, ':');

                if (deep.size() == 0)
                    return false;

                std::vector<int> &this_deep = log_result_deep;
                this_deep.resize(deep.size()-1);

                std::string this_type = Utils::ToLowerCase(deep[0]);
                Utils::TrimString(this_type);
				
				if (!GetLogType(this_type, log_result_type))
					return false;                

                for (size_t j = 1; j < deep.size(); j++)
                {
                    this_deep[j-1] = Utils::GetUnsigned(deep[j]);
                }
				
				std::string result_extra;
				
				if (!Utils::ReadAttrString(log, "result_extra", result_type))
				{
					if (log_result_type < LOG_PARAM_TYPE_PTR)
                        log_result_extra = 0;
                    else
                        log_result_extra = 16;
					
					log_result_ptr_size = 16;
                }
				else
				{
					std::string modifier = Utils::ToLowerCase(result_extra);
                    Utils::TrimString(modifier);
					
					GetLogExtra(modifier, log_result_type, log_result_extra, log_result_ptr_size);
				}
			}
		} // END RESULT LOG

        if (log_params.size() == 0)
        {
            log_types.clear();
            log_select.clear();
            log_extra.clear();
            log_ptr_sizes.clear();
            log_deep.clear();
        }
        else
        {
            std::vector<std::string> types;
            std::string select;
            std::vector<std::string> extra;

            if (!Utils::ReadAttrMultipleStrings(log, "types", types) || types.size() != log_params.size())
                return false;

            log_types.resize(log_params.size());
            log_deep.resize(log_params.size());

            for (size_t i = 0; i < log_types.size(); i++)
            {
                std::vector<std::string> deep;
                Utils::GetMultipleStrings(types[i], deep, ':');

                if (deep.size() == 0)
                    return false;

                int ptype;

                std::vector<int> &this_deep = log_deep[i];
                this_deep.resize(deep.size()-1);

                std::string this_type = Utils::ToLowerCase(deep[0]);
                Utils::TrimString(this_type);
				
				if (!GetLogType(this_type, ptype))
					return false;

                log_types[i] = ptype;

                for (size_t j = 1; j < deep.size(); j++)
                {
                    this_deep[j-1] = Utils::GetUnsigned(deep[j]);
                }
            }

            if (Utils::ReadAttrString(log, "select", select))
            {
                Utils::TrimString(select);

                if (select.length() != log_params.size())
                    return false;

                log_select.resize(log_params.size());

                for (size_t i = 0; i < log_select.size(); i++)
                {
                    char bool_value = select[i];

                    if (bool_value == '1')
                        log_select[i] = true;
                    else if (bool_value == '0')
                        log_select[i] = false;
                    else
                        return false;
                }
            }
            else
            {
                log_select.clear();
                log_select.resize(log_params.size(), true);
            }

            if (Utils::ReadAttrMultipleStrings(log, "extra", extra))
            {
                if (extra.size() != log_params.size())
                    return false;

                log_extra.resize(log_params.size());
                log_ptr_sizes.resize(log_params.size());

                for (size_t i = 0; i < log_extra.size(); i++)
                {                    
                    std::string modifier = Utils::ToLowerCase(extra[i]);
                    Utils::TrimString(modifier);
					
					GetLogExtra(modifier, log_types[i], log_extra[i], log_ptr_sizes[i]);
				}
            }
            else
            {
                log_extra.resize(log_params.size());

                for (size_t i = 0; i < log_extra.size(); i++)
                {
                    int type = log_types[i];

                    if (type < LOG_PARAM_TYPE_PTR)
                        log_extra[i] = 0;
                    else
                        log_extra[i] = 16;
                }

                log_ptr_sizes.clear();
                log_ptr_sizes.resize(log_params.size(), 16);
            }
        }

        common = log;
    }
    else if (type == EPATCH_TYPE_NOTIFY)
    {
        const TiXmlElement *notify;

        if (!Utils::GetParamString(root, "Notify", function, &notify) || function.length() == 0)
            return false;

        if (!Utils::ReadAttrString(notify, "module", function_module))
            function_module = patcher_module;

        common = notify;
    }

    if (!Utils::ReadAttrUnsigned(common, "inst_index", &inst_index))
        inst_index = 0;

    if (!Utils::ReadAttrUnsigned(common, "inst_offset", &inst_offset))
        inst_offset = 0;

    if (inst_index >= instructions.size())
        return false;

    if (inst_offset >= instructions[inst_index].search_pattern.size())
        return false;

    if (type == EPATCH_TYPE_NOP && size == 0)
    {
        size = instructions[inst_index].search_pattern.size() - inst_offset;

        for (size_t i = inst_offset+1; i < instructions.size(); i++)
        {
            size += instructions[i].search_pattern.size();
        }
    }

    rebuild = true;
    return true;
}

bool EPatch::Apply()
{
    uint32_t current_matches = 0;
	
	while (current_matches < num_matches)
	{
		uint8_t *ptr = Find();
		if (!ptr)
			return false;

		for (size_t i = 0; i < inst_index; i++)
			ptr += instructions[i].search_pattern.size();

		ptr += inst_offset;

		DPRINTF("Patch \"%s\" located at address %p. Relative: 0x%x.\n", name.c_str(), ptr, REL_ADDR32(ptr));

		if (type == EPATCH_TYPE_WRITE)
		{
			for (uint16_t byte : replace_pattern)
			{
				if (byte < 0x100)
					Write8(ptr, (uint8_t)byte);

				ptr++;
			}
		}
		else if (type == EPATCH_TYPE_NOP)
		{
			Nop(ptr, size);
		}
		else if (type == EPATCH_TYPE_HOOK)
		{
			HMODULE mod = GetModuleHandleA(CSTR(function_module));
			if (!mod)
				return false;

			uint8_t *func = (uint8_t *)GetProcAddress(mod, function.c_str());
			if (!func)
				return false;

			void *orig;        

			if (hook_type == HOOK_TYPE_NORMAL)
			{
				if (!Hook(ptr, &orig, func))
					return false;
			}
			else if (hook_type == HOOK_TYPE_RESOLVE_TARGET)
			{
				if (!HookResolveTarget(ptr, &orig, func))
					return false;
			}
			else if (hook_type == HOOK_TYPE_DIRECT_TARGET)
			{
				if (!HookResolveTarget(ptr, &orig, func, false))
					return false;
			}
			else // HOOK_TYPE_CALLL
			{
				if (!HookCall(ptr, &orig, func))
					return false;
			}

			if (setup_function.length() > 0)
			{
				SETUP_FUNCTION setup = (SETUP_FUNCTION)GetProcAddress(mod, setup_function.c_str());
				if (!setup)
				{
					DPRINTF("%s: Warning, setup function \"%s\" not found.\n", FUNCNAME, setup_function.c_str());
				}

				if (setup)
				{
					setup(orig);
				}
			}        
		}
		else if (type == EPATCH_TYPE_LOG)
		{
			size_t num_params = log_params.size();

			if (calling_convention == LOG_CC_CDECL)
			{
				if (num_params == 0)
				{
					log_func = (void *)Log0;
				}
				else if (num_params == 1)
				{
					log_func = (void *)Log1;
				}
				else if (num_params == 2)
				{
					log_func = (void *)Log2;
				}
				else if (num_params == 3)
				{
					log_func = (void *)Log3;
				}
				else if (num_params == 4)
				{
					log_func = (void *)Log4;
				}
				else if (num_params == 5)
				{
					log_func = (void *)Log5;
				}
				else if (num_params == 6)
				{
					log_func = (void *)Log6;
				}
				else if (num_params == 7)
				{
					log_func = (void *)Log7;
				}
				else if (num_params == 8)
				{
					log_func = (void *)Log8;
				}
				else if (num_params == 9)
				{
					log_func = (void *)Log9;
				}
				else if (num_params == 10)
				{
					log_func = (void *)Log10;
				}
			}
	#ifdef CPU_X86
			else if (calling_convention == LOG_CC_STDCAL)
			{
				if (num_params == 0)
				{
					log_func = (void *)Log0_Stdcall;
				}
				else if (num_params == 1)
				{
					log_func = (void *)Log1_Stdcall;
				}
				else if (num_params == 2)
				{
					log_func = (void *)Log2_Stdcall;
				}
				else if (num_params == 3)
				{
					log_func = (void *)Log3_Stdcall;
				}
				else if (num_params == 4)
				{
					log_func = (void *)Log4_Stdcall;
				}
				else if (num_params == 5)
				{
					log_func = (void *)Log5_Stdcall;
				}
				else if (num_params == 6)
				{
					log_func = (void *)Log6_Stdcall;
				}
				else if (num_params == 7)
				{
					log_func = (void *)Log7_Stdcall;
				}
				else if (num_params == 8)
				{
					log_func = (void *)Log8_Stdcall;
				}
				else if (num_params == 9)
				{
					log_func = (void *)Log9_Stdcall;
				}
				else if (num_params == 10)
				{
					log_func = (void *)Log10_Stdcall;
				}
			}
			else if (calling_convention == LOG_CC_THISCALL)
			{
				if (num_params == 0)
				{
					log_func = (void *)Log0_Thiscall;
				}
				else if (num_params == 1)
				{
					log_func = (void *)Log1_Thiscall;
				}
				else if (num_params == 2)
				{
					log_func = (void *)Log2_Thiscall;
				}
				else if (num_params == 3)
				{
					log_func = (void *)Log3_Thiscall;
				}
				else if (num_params == 4)
				{
					log_func = (void *)Log4_Thiscall;
				}
				else if (num_params == 5)
				{
					log_func = (void *)Log5_Thiscall;
				}
				else if (num_params == 6)
				{
					log_func = (void *)Log6_Thiscall;
				}
				else if (num_params == 7)
				{
					log_func = (void *)Log7_Thiscall;
				}
				else if (num_params == 8)
				{
					log_func = (void *)Log8_Thiscall;
				}
				else if (num_params == 9)
				{
					log_func = (void *)Log9_Thiscall;
				}
				else if (num_params == 10)
				{
					log_func = (void *)Log10_Thiscall;
				}
			}
			else if (calling_convention == LOG_CC_FASTCALL)
			{
				if (num_params == 0)
				{
					log_func = (void *)Log0_Fastcall;
				}
				else if (num_params == 1)
				{
					log_func = (void *)Log1_Fastcall;
				}
				else if (num_params == 2)
				{
					log_func = (void *)Log2_Fastcall;
				}
				else if (num_params == 3)
				{
					log_func = (void *)Log3_Fastcall;
				}
				else if (num_params == 4)
				{
					log_func = (void *)Log4_Fastcall;
				}
				else if (num_params == 5)
				{
					log_func = (void *)Log5_Fastcall;
				}
				else if (num_params == 6)
				{
					log_func = (void *)Log6_Fastcall;
				}
				else if (num_params == 7)
				{
					log_func = (void *)Log7_Fastcall;
				}
				else if (num_params == 8)
				{
					log_func = (void *)Log8_Fastcall;
				}
				else if (num_params == 9)
				{
					log_func = (void *)Log9_Fastcall;
				}
				else if (num_params == 10)
				{
					log_func = (void *)Log10_Fastcall;
				}
			}
	#endif // CPU_X86
			else
			{
				// This code should be unreachable unless bugs
				assert(0);
				return false;
			}

			MutexLocker lock(&log_mutex);

			if (hook_type == HOOK_TYPE_NORMAL)
			{
				if (!Hook(ptr, &log_original_func, log_func))
					return false;
			}
			else if (hook_type == HOOK_TYPE_RESOLVE_TARGET)
			{
				 if (!HookResolveTarget(ptr, &log_original_func, log_func))
					return false;
			}
			else if (hook_type == HOOK_TYPE_DIRECT_TARGET)
			{
				if (!HookResolveTarget(ptr, &log_original_func, log_func, false))
					return false;
			}
			else // HOOK_TYPE_CALLL
			{
				if (!HookCall(ptr, &log_original_func, log_func))
					return false;
			}

			AddLogPatch();
		}
		else if (type == EPATCH_TYPE_NOTIFY)
		{
			if (function == "NULL")
				return true;
			
			HMODULE mod = GetModuleHandleA(CSTR(function_module));
			if (!mod)
				return false;

			SETUP_FUNCTION setup = (SETUP_FUNCTION)GetProcAddress(mod, function.c_str());
			if (!setup)
				return false;

			setup(ptr);
		}

		current_matches++;
	}
	

    return true;
}

int EPatch::GetEnabled(std::string &setting) const
{
    if (enabled == "true" || enabled == "1")
        return true;

    if (enabled == "false" || enabled == "0")
        return false;

    setting = enabled;
    return -1;
}

bool EPatchFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    UNUSED(big_endian);

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "EternityPatchFile");

    if (!root)
    {
        DPRINTF("%s: Cannot locate \"EternityPatchFile\" in xml.\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrString(root, "name", name) || name.length() == 0)
    {
        DPRINTF("%s: name field is mandatory (and cannot be 0 length).\n", FUNCNAME);
        return false;
    }

    if (!Utils::ReadAttrString(root, "enabled", enabled) || enabled.length() == 0)
        enabled = "true";

    if (!Utils::ReadAttrString(root, "comment", comment))
        comment.clear();

    size_t patch_count = Utils::GetElemCount(root, "Patch");
    if (patch_count == 0)
    {
        DPRINTF("%s: No patches in file \"%s\"\n", FUNCNAME, name.c_str());
        return false;
    }

    patches.resize(patch_count);

    size_t idx = 0;
    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == "Patch")
        {
            EPatch &patch = patches[idx++];
            patch.SetPatcher(patcher_module);

            if (!patch.Compile(elem))
            {
                DPRINTF("%s: Failed to compile Patch at index 0x%x\n", FUNCNAME, (unsigned int)idx-1);
                return false;
            }
        }
    }

    return true;
}

int EPatchFile::GetEnabled(std::string &setting) const
{
    if (enabled == "true" || enabled == "1")
        return true;

    if (enabled == "false" || enabled == "0")
        return false;

    setting = enabled;
    return -1;
}


