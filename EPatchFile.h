#ifndef EPATCHFILE_H
#define EPATCHFILE_H

#include <windef.h>
#include <stdexcept>
#include <unordered_set>

#include "BaseFile.h"
#include "Mutex.h"

struct EInstruction
{
    std::string comment;
    std::vector<uint16_t> search_pattern;

    bool Compile(const TiXmlElement *root);
};

enum EPatchType
{
    EPATCH_TYPE_WRITE,
    EPATCH_TYPE_NOP,
    EPATCH_TYPE_HOOK,
    EPATCH_TYPE_LOG,
    EPATCH_TYPE_NOTIFY,
    EPATCH_TYPE_MAX
};

enum HookType
{
    HOOK_TYPE_NORMAL,
    HOOK_TYPE_CALL,
	HOOK_TYPE_RESOLVE_TARGET,
	HOOK_TYPE_DIRECT_TARGET,
    HOOK_TYPE_MAX
};

enum LogParamType
{
    LOG_PARAM_TYPE_U8,
    LOG_PARAM_TYPE_U16,
    LOG_PARAM_TYPE_U32,
    LOG_PARAM_TYPE_U64,
    LOG_PARAM_TYPE_S8,
    LOG_PARAM_TYPE_S16,
    LOG_PARAM_TYPE_S32,
    LOG_PARAM_TYPE_S64,
    LOG_PARAM_TYPE_FLOAT,
    LOG_PARAM_TYPE_DOUBLE,
    LOG_PARAM_TYPE_CSTR,
    LOG_PARAM_TYPE_WSTR,    
	LOG_PARAM_TYPE_VCSTR,
	LOG_PARAM_TYPE_PTR,
    LOG_PARAM_TYPE_MAX
};

enum LogExtra
{
    LOG_EXTRA_HEXADECIMAL,
    LOG_EXTRA_DECIMAL,
    LOG_EXTRA_HEXADECIMAL_ALIGNED,
};

enum LogCallingConvention
{
    LOG_CC_CDECL,
    LOG_CC_STDCAL,
    LOG_CC_THISCALL,
    LOG_CC_FASTCALL,
    LOG_CC_MAX
};

class EPatch
{
private:

    // Global data
    static std::vector<EPatch> log_patches;
    static Mutex log_mutex;

    // Non serialized data
    std::string patcher_module;

    bool rebuild = true;
    std::vector<uint16_t> search_pattern; // instructions combined in a single item
	std::unordered_set<size_t> block_addresses; // For num_matches > 1

    // Common
    std::string name;
    std::string comment;
    int type;
    std::string module;
    std::string enabled;
	uint32_t num_matches;

    size_t search_start;
    size_t search_down, search_up;

    std::vector<EInstruction> instructions;
    // Write
    std::vector<uint16_t> replace_pattern;

    // Hook
    std::string function; // For hook, a function name in the patcher_module; For log, this is just a friendly string to be displayed
    std::string function_module;
    std::string setup_function;
    int hook_type; // Also used in log

    // Log
    bool log_before;
    std::vector<std::string> log_params;
    std::vector<int> log_types;
    std::vector<bool> log_select;
    std::vector<int> log_extra;
    std::vector<int> log_ptr_sizes;
    std::vector<std::vector<int>> log_deep;
	
	// Log result
	int log_result_type;
	int log_result_extra;
	int log_result_ptr_size;
	std::vector<int> log_result_deep;
	
    int calling_convention;
    void *log_func;
    void *log_original_func;
    bool log_ra;

    // Common for some
    size_t inst_index;
    size_t inst_offset;
    size_t size;

    //
    bool IsPattern(size_t address, const std::vector<uint16_t> &pattern);
    uint8_t *Find();

    //
    void AddLogPatch();
    static EPatch *FindLogPatch(void *addr);

    void LogParam(size_t param, int index);

#define CALLING_CONVENTION			__cdecl
#define SUFFIX_CONVENTION				
#define SUFFIX_CONVENTION_UPPER		

#define NUM_PARAMS 0
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER
   
#ifdef CPU_X86

#define CALLING_CONVENTION			__stdcall
#define SUFFIX_CONVENTION			_Stdcall				
#define SUFFIX_CONVENTION_UPPER		_STDCALL	

#define NUM_PARAMS 0
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER

#define CALLING_CONVENTION			__thiscall
#define SUFFIX_CONVENTION			_Thiscall				
#define SUFFIX_CONVENTION_UPPER		_THISCALL	

#define NUM_PARAMS 0
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER
    

#define CALLING_CONVENTION			__fastcall
#define SUFFIX_CONVENTION			_Fastcall		
#define SUFFIX_CONVENTION_UPPER		_FASTCALL

#define NUM_PARAMS 0
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 1
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 2
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 3
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 4
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 5
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 6
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 7
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 8
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 9
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#define NUM_PARAMS 10
#include "EPatchFile_def.inl"
#undef NUM_PARAMS

#undef CALLING_CONVENTION
#undef SUFFIX_CONVENTION
#undef SUFFIX_CONVENTION_UPPER

#endif // X86

static int ParseHookType(const std::string &type);

public:

    EPatch() : type(EPATCH_TYPE_MAX), num_matches(1) { }
    EPatch(const std::string &patcher) : patcher_module(patcher), type(EPATCH_TYPE_MAX), num_matches(1) { }

    bool Compile(const TiXmlElement *root);
    bool Apply();

    const std::string &GetName() const
    {
        return name;
    }

    int GetEnabled(std::string &setting) const;
    void SetPatcher(const std::string &patcher) { patcher_module = patcher; }
};

class EPatchFile : public BaseFile
{
    std::string patcher_module;

    std::string name;
    std::string enabled;
    std::string comment;

    std::vector<EPatch> patches;

public:

    EPatchFile() { }
    EPatchFile(const std::string &patcher) : patcher_module(patcher) { }
    virtual ~EPatchFile() { }

    virtual bool Compile(TiXmlDocument *doc, bool big_endian) override;

    inline EPatch &operator[](size_t n) { return patches[n]; }
    inline const EPatch &operator[](size_t n) const { return patches[n]; }

    const std::string GetName() const { return name; }
    int GetEnabled(std::string &setting) const;
	
	inline size_t GetNumPatches() { return patches.size(); }

    inline EPatch &operator[](const std::string &patch_name)
    {
        for (EPatch &patch : patches)
        {
            if (Utils::ToLowerCase(patch_name) == Utils::ToLowerCase(patch.GetName()))
                return patch;
        }

        throw std::out_of_range("Patch " + patch_name + " doesn't exist.");
    }

    inline const EPatch &operator[](const std::string &patch_name) const
    {
        for (const EPatch &patch : patches)
        {
            if (Utils::ToLowerCase(patch_name) == Utils::ToLowerCase(patch.GetName()))
                return patch;
        }

        throw std::out_of_range("Patch " + patch_name + " doesn't exist.");
    }

    inline std::vector<EPatch>::iterator begin() { return patches.begin(); }
    inline std::vector<EPatch>::iterator end() { return patches.end(); }

    inline std::vector<EPatch>::const_iterator begin() const { return patches.begin(); }
    inline std::vector<EPatch>::const_iterator end() const { return patches.end(); }
};

#endif // EPATCHFILE_H
