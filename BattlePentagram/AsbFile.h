#ifndef ASBFILE_H
#define ASBFILE_H

#include <unordered_map>
#include <queue>
#include "FixedMemoryStream.h"
#include "FileStream.h"


;

#pragma pack(push,1)

struct ASBHeader
{
    uint32_t unk_00; // Observed value of 0
    char name[32]; // 4 - Not sure if the full space would be for the script name
    uint32_t func_defs_start; // 24
    uint32_t num_funcs; // 28
    uint32_t code_start; // 2C
    uint32_t code_size; // 30
    uint32_t strings_start; // 34 - This one is aligned to 4 bytes
    uint32_t strings_size; // 38
    uint32_t file_size; // 3C
    uint32_t unk_40; // Observed value of 0

    ASBHeader()
    {
        memset(this, 0, sizeof(ASBHeader));
    }
};
CHECK_STRUCT_SIZE(ASBHeader, 0x44);

struct ASBFuncDef
{
    uint32_t name; // 0 - Offset relative to ASBHeader.strings_start
    uint32_t num_params; // 4
    uint32_t num_locals; // 8
    uint32_t code_start; // C - Offset relative to ASBHeader.code_start
    uint32_t size; // 10 - Size in bytes
};
CHECK_STRUCT_SIZE(ASBFuncDef, 0x14);

#pragma pack(pop)

struct AsbString
{
    std::string str;
    uint32_t pos;

    AsbString()
    {
        pos = 0;
    }
};

struct AsbFunction
{
    std::string name;
    uint32_t pos;
    uint32_t num_params;
    uint32_t num_locals;
    std::vector<uint8_t> code;

    AsbFunction()
    {
        pos = 0;
    }
};

// Assembler only
struct AsbLine
{
    std::string text;
    uint32_t number;
};

class AsbFile : public BaseFile
{
    std::string name;
    std::vector<AsbString> strings;
    std::vector<AsbFunction> functions;

    // For assembler use only
    std::unordered_map<std::string, AsbString> defined_strings;
    std::unordered_map<std::string, AsbFunction> defined_functions;
    std::unordered_map<uint32_t, std::string> unresolved_calls;
    //

private:

    bool DisassembleInstruction(Stream *in, Stream *out, uint32_t address, const std::unordered_map<uint32_t, std::string> &labels) const;
    bool DisassembleFunction(const AsbFunction &function, Stream *out) const;
    bool BuildLabels(const AsbFunction &function, std::unordered_map<uint32_t, std::string> &labels) const;
    bool DisassembleCommon(Stream *out) const;

    std::string MakeString(const std::string &in, bool with_quotes) const;
    bool GetString(uint32_t position, std::string &str) const;
    bool GetNonFuncString(uint32_t position, std::string &str) const;
    uint32_t FindString(const std::string &str) const;

    const AsbFunction *FindFunc(uint32_t position) const;
    uint32_t GetFunctionIdx(const std::string &name) const;

    bool SkipInstruction(Stream *s) const;
    std::string GetTalker(uint32_t pos) const;

    void RemoveComments(std::string &line) const;
    std::string UsefulString(const std::string &str);
    bool GetAssignment(const std::string &str, std::string &left, std::string &right);
    bool ParseString(const std::string &raw, std::string &value);
    bool IsNumber(const std::string &str);
    bool WriteAtAbsoluteAddress(uint32_t addr, uint32_t val);
    bool AssembleStrings(std::queue<AsbLine> &lines);
    bool AssembleFunction(std::queue<AsbLine> &lines, uint32_t &code_addr);
    bool AssembleCommon(const std::string &in);

protected:
    void Reset();

public:
    AsbFile();
    virtual ~AsbFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool Disassemble(const std::string &path) const;
    uint8_t *Disassemble(size_t *psize) const;

    bool Assemble(const std::string &path);
    bool Assemble(const uint8_t *buf, size_t size);
};

#endif // ASBFILE_H
