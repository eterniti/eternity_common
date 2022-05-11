#include "AsbFile.h"

enum OpType
{
    OP_LOCAL,
    OP_U8,
    OP_S32,
    OP_S32_OR_STR,
    OP_ARRAY,  // 8 bits (unsigned, len of array) + n * 32 bits
    OP_FUNC,
    OP_NFUNC,
    OP_LABEL,
};

struct AsbInstDef
{
    uint32_t opcode;
    std::string name;
    std::vector<int> op_types;
};

static const std::vector<AsbInstDef> inst_defs =
{
    {   0,      "nop", { } },
    {   1,      "pushc", { OP_S32_OR_STR } },
    {   2,      "pushv", { OP_LOCAL } },
    {   3,      "op3", { OP_S32 } }, // Maybe a call
    {   4,      "op4", { OP_S32 } }, // Maybe a call
    {   5,      "pushret", { } },
    {   6,      "popd", { } },
    {   7,      "pop", { OP_LOCAL } },
    {   8,      "op8", { OP_S32 } }, // Maybe a call
    {   9,      "op9", { OP_S32 } }, // Maybe a call
    {   0xA,    "asgn", { OP_LOCAL, OP_S32 } },
    {   0xB,    "opB", { OP_S32 } }, // Maybe a call
    {   0xC,    "opC", { OP_S32 } }, // May be a call
    {   0xD,    "inc", { OP_LOCAL } },
    {   0xE,    "dec", { OP_LOCAL } },
    {   0xF,    "opF", { OP_S32 } }, // May be a call
    {   0x10,   "op10", { OP_S32 } },  // May be a call
    {   0x11,   "copy", { } }, // Pops two values from stack, assign one to the other
    {   0x12,   "neg", { } }, // Negates the top of stack (ej a = -a)
    {   0x13,   "add", { } },
    {   0x14,   "sub", { } },
    {   0x15,   "mult", { } },
    {   0x16,   "div", { } }, // "Probably", because arm of vita doesn't have div instruction, I'm not sure
    {   0x17,   "mod", { } }, // "Probably", because arm of vita doesn't have div instruction, I'm not sure
    {   0x18,   "and", { } },
    {   0x19,   "or",  { } },
    {   0x1A,   "not", { } },
    {   0x1B,   "eq",   { } }, // Two values read from stack. If equal, 1 is pushed, 0 otherwise
    {   0x1C,   "neq", { } },
    {   0x1D,   "lt", { } }, // This one may be bg, and bg may be lt
    {   0x1E,   "gt", { } },
    {   0x1F,   "lte", { } }, // This one may be bte, and bte may be lte
    {   0x20,   "gte", { } },
    {   0x21,   "jmp", { OP_LABEL } },
    {   0x22,   "jz", { OP_LABEL } },
    {   0x23,   "jnz", { OP_LABEL } },
    {   0x24,   "jmpa", { OP_ARRAY } }, // If array size is > 0, it reads array size 32 bits. It will jump to the one whose index matches top of stack.
    {   0x25,   "op25", { OP_S32 } },
    {   0x26,   "op26", { OP_S32, OP_S32 } }, // It may be a call
    {   0x27,   "call", { OP_FUNC, OP_U8 } }, // There maybe a memmove with size being the second operand
    {   0x28,   "op28", { OP_S32, OP_S32, OP_U8 } }, // It is close to op26, but there is this additional argument which may be a size
    {   0x29,   "calln", { OP_NFUNC, OP_U8 } },
    {   0x2A,   "retp", { } },
    {   0x2B,   "ret", { } },
    {   0x2C,   "op2C", { } }
};

static const std::vector<std::string> char_names =
{
    "NONE", // 0
    "Madoka", // 1
    "Sayaka", // 2
    "Mami", // 3
    "Homura", // 4
    "Kyoko", // 5
    "Gekijo", // 6
    "Hitomi", // 7
    "Kyosuke", // 8
    "Kazuko Saotome", // 9
    "Kyubey", // 10
    "Homura (glasses)", // 11
    "Homura (ribbon)", // 12
    "Ultimate Madoka", // 13
    "Tatsuya", // 14
    "Female student", // 15
    "Teacher", // 16
    "Student", // 17
    "Familiar", // 18
    "Nurse A", // 19
    "Nurse B", // 20
    "Staff", // 21
    "Both", // 22
    "Them three", // 23
    "Them four", // 24
    "All", // 25
    "Mysterious", // 26
    "Unknown", // 27
    "Narrator", // 28
};

AsbFile::AsbFile()
{
    this->big_endian = false;
}

AsbFile::~AsbFile()
{

}

bool AsbFile::DisassembleInstruction(Stream *in, Stream *out, uint32_t address, const std::unordered_map<uint32_t, std::string> &labels) const
{
    uint8_t opcode;

    if (!in->Read8(&opcode))
    {
        DPRINTF("%s: Failed to read opcode.\n", FUNCNAME);
        return false;
    }

    if (opcode >= inst_defs.size())
    {
        DPRINTF("%s: Unrecognized opcode 0x%02x\n", FUNCNAME, opcode);
        return false;
    }

    const AsbInstDef &def = inst_defs[opcode];

    auto it = labels.find(address);
    if (it != labels.end())
    {
        if (!out->Printf("%s:\n", it->second.c_str()))
            return false;
    }

    if (!out->Printf("\t%s\t", def.name.c_str()))
        return false;

    if (opcode == 7 || opcode == 0x21 || opcode == 0x22 || opcode == 0x23) // pop, jmp, jz, jnz
        if (!out->Write8('\t'))
            return false;

    std::string comment;

    for (size_t i = 0; i < def.op_types.size(); i++)
    {
        std::string str;
        int32_t op_s32;
        uint32_t func_idx, label;
        uint8_t local, op_u8, alen;

        switch (def.op_types[i])
        {
            case OP_LOCAL:
                if (!in->Read8(&local))
                {
                    DPRINTF("%s: Failed to read local.\n", FUNCNAME);
                    return false;
                }

                if (!out->Printf("local_%02x", local))
                    return false;
            break;

            case OP_U8:
                if (!in->Read8(&op_u8))
                {
                    DPRINTF("%s: Failed to read op_u8.\n", FUNCNAME);
                    return false;
                }

                if (op_u8 >= 0xA)
                {
                    if (!out->Printf("0x%02x", op_u8))
                        return false;

                    if (comment.length() != 0)
                        comment.push_back(',');

                    comment += Utils::ToString((int)op_u8);
                }
                else
                {
                    if (!out->Printf("%d", op_u8))
                        return false;
                }

            break;

            case OP_S32:
                if (!in->Read32(&op_s32))
                {
                    DPRINTF("%s: Failed to read op_s32.\n", FUNCNAME);
                    return false;
                }

                if (!out->Printf("0x%08x", op_s32))
                    return false;

                if (op_s32 >= 0xA)
                {
                    if (comment.length() != 0)
                        comment.push_back(',');

                    comment += Utils::ToString(op_s32);
                }
            break;

            case OP_S32_OR_STR:
            {
                if (!in->Read32(&op_s32))
                {
                    DPRINTF("%s: Failed to read op_s32.\n", FUNCNAME);
                    return false;
                }

                bool is_string = false;

                if (GetNonFuncString((uint32_t)op_s32, str))
                {
                    uint8_t data[6];
                    static const uint8_t native0_call[6] = { 0x29, 0x00, 0x00, 0x00, 0x00, 0x01 };

                    in->SavePos();

                    if (in->Read(data, sizeof(data)) && memcmp(data, native0_call, sizeof(data)) == 0)
                        is_string = true;

                    in->RestorePos();
                }

                if (is_string)
                {
                    if (!out->Printf("str_%04x", op_s32))
                        return false;
                }
                else
                {
                    if (!out->Printf("%d", op_s32))
                        return false;
                }
            }
            break;

            case OP_ARRAY:
                if (!in->Read8(&alen))
                {
                    DPRINTF("%s: Failed to read alen.\n", FUNCNAME);
                    return false;
                }

                for (uint8_t i = 0; i < alen; i++)
                {
                    int32_t val;

                    if (!in->Read32(&val))
                    {
                        DPRINTF("%s: Failed to read val.\n", FUNCNAME);
                        return false;
                    }

                    if (i == (alen-1))
                    {
                        if (!out->Printf("0x%x", val))
                            return false;
                    }
                    else
                    {
                        if (!out->Printf("0x%x,", val))
                            return false;
                    }
                }
            break;

            case OP_FUNC:
                if (!in->Read32(&func_idx))
                {
                    DPRINTF("%s: Failed to read func_idx.\n", FUNCNAME);
                    return false;
                }

                if (func_idx >= (uint32_t)functions.size())
                {
                    DPRINTF("%s: func_idx %x out of bounds.\n", FUNCNAME, func_idx);
                    return false;
                }

                if (!out->WriteString(functions[func_idx].name))
                    return false;

            break;

            case OP_NFUNC:
                if (!in->Read32(&func_idx))
                {
                    DPRINTF("%s: Failed to read func_idx.\n", FUNCNAME);
                    return false;
                }

                if (!out->Printf("@native%02x", func_idx))
                    return false;

            break;

            case OP_LABEL:

                if (!in->Read32(&label))
                {
                    DPRINTF("%s: Failed to read label.\n", FUNCNAME);
                    return false;
                }

                auto it = labels.find(label);
                if (it != labels.end())
                {
                    if (!out->WriteString(it->second))
                        return false;
                }
                else
                {
                    if (!out->Printf("0x%08x", label))
                        return false;
                }

            break;
        }

        if (i != (def.op_types.size()-1) && !out->Write8(','))
            return false;
    }

    //comment += " @" + Utils::UnsignedToHexString(address, true);

    if (comment.length() > 0)
    {
        comment = " ; " + comment;
        if (!out->WriteString(comment))
            return false;
    }

    return out->Write8('\n');
}

bool AsbFile::BuildLabels(const AsbFunction &function, std::unordered_map<uint32_t, std::string> &labels) const
{
    FixedMemoryStream code(const_cast<uint8_t *>(function.code.data()), function.code.size());
    labels.clear();

     while (code.Tell() != function.code.size())
     {
         uint8_t opcode;

         if (!code.Read8(&opcode))
         {
             DPRINTF("%s: Failed to read opcode.\n", FUNCNAME);
             return false;
         }

         if (opcode >= inst_defs.size())
         {
             DPRINTF("%s: opcode 0x%02x is out of bounds.\n", FUNCNAME, opcode);
             return false;
         }

         const AsbInstDef &def = inst_defs[opcode];

         for (size_t i = 0; i < def.op_types.size(); i++)
         {
             uint32_t address;
             uint8_t alen;

             switch (def.op_types[i])
             {
                case OP_LABEL:

                    if (!code.Read32(&address))
                    {
                        DPRINTF("%s: Failed to read address.\n", FUNCNAME);
                        return false;
                    }

                    if (address < 0x10000)
                        labels[address] = "label_" + Utils::UnsignedToHexString((uint16_t)address, true, false);
                    else
                        labels[address] = "label_" + Utils::UnsignedToHexString(address, false, false);

                break;

                case OP_LOCAL: case OP_U8:

                    if (!code.Seek(sizeof(uint8_t), SEEK_CUR))
                    {
                        DPRINTF("%s: Premature end of function.\n", FUNCNAME);
                        return false;
                    }

                break;

                case OP_S32: case OP_S32_OR_STR: case OP_FUNC: case OP_NFUNC:

                    if (!code.Seek(sizeof(uint32_t), SEEK_CUR))
                    {
                        DPRINTF("%s: Premature end of function (2).\n", FUNCNAME);
                        return false;
                    }

                break;

                case OP_ARRAY:

                    if (!code.Read8(&alen))
                    {
                        DPRINTF("%s: Cannot read alen.\n", FUNCNAME);
                        return false;
                    }

                    if (alen > 0 && !code.Seek(alen*sizeof(uint32_t), SEEK_CUR))
                    {
                        DPRINTF("%s: Premature end of function (3).\n", FUNCNAME);
                        return false;
                    }

                break;
             }
         }
     }

    return true;
}

bool AsbFile::DisassembleFunction(const AsbFunction &function, Stream *out) const
{
    std::unordered_map<uint32_t, std::string> labels;

    if (!BuildLabels(function, labels))
        return false;

    if (!out->Printf("function %s,%u\n", function.name.c_str(), function.num_params))
        return false;

    FixedMemoryStream code(const_cast<uint8_t *>(function.code.data()), function.code.size());

    while (code.Tell() != function.code.size())
    {
        if (!DisassembleInstruction(&code, out, (uint32_t)(code.Tell() + function.pos), labels))
        {
            DPRINTF("%s: Failed to disassemble function %s. Position is at 0x%Ix\n", FUNCNAME, function.name.c_str(), (size_t)code.Tell());
            return false;
        }
    }

    return out->WriteString("endfunction\n\n");
}

std::string AsbFile::MakeString(const std::string &in, bool with_quotes) const
{
    std::string ret;

    for (char c : in)
    {
        if (c == '\n')
        {
            ret += "\\n";
        }
        else if (c == '"')
        {
            ret += "\\\"";
        }
        else
        {
            ret.push_back(c);
        }
    }

    if (with_quotes)
    {
        ret.insert(ret.begin(), '"');
        ret.push_back('"');
    }

    return ret;
}

bool AsbFile::GetString(uint32_t position, std::string &str) const
{
    for (size_t i = 0; i < strings.size(); i++)
    {
        if (strings[i].pos == position)
        {
            str = strings[i].str;
            return true;
        }
    }

    return false;
}

bool AsbFile::GetNonFuncString(uint32_t position, std::string &str) const
{
    if (!GetString(position, str))
        return false;

    for (const AsbFunction &func : functions)
        if (func.name == str)
            return false;

    return true;
}

uint32_t AsbFile::FindString(const std::string &str) const
{
    for (const AsbString &as : strings)
        if (as.str == str)
            return as.pos;

    return 0xFFFFFFFF;
}

const AsbFunction *AsbFile::FindFunc(uint32_t position) const
{
    for (const AsbFunction &func : functions)
        if (func.pos == position)
            return &func;

    return nullptr;
}

uint32_t AsbFile::GetFunctionIdx(const std::string &name) const
{
    for (size_t i = 0; i < functions.size(); i++)
        if (functions[i].name == name)
            return (uint32_t)i;

    return 0xFFFFFFFF;
}

bool AsbFile::SkipInstruction(Stream *s) const
{
    uint8_t opcode;
    if (!s->Read8(&opcode))
        return false;

    if (opcode >= inst_defs.size())
        return false;

    const AsbInstDef &def = inst_defs[opcode];
    for (size_t i = 0; i < def.op_types.size(); i++)
    {
        switch (def.op_types[i])
        {
            case OP_LOCAL: case OP_U8:
                if (!s->Skip(1))
                    return false;
            break;

            case OP_S32: case OP_S32_OR_STR: case OP_FUNC: case OP_NFUNC: case OP_LABEL:
                if (!s->Skip(4))
                    return false;
            break;

            case OP_ARRAY:
            {
                uint8_t arr_size;
                if (!s->Read8(&arr_size))
                    return false;

                if (!s->Skip(arr_size*4))
                    return false;
            }
        }
    }

    return true;
}

std::string AsbFile::GetTalker(uint32_t pos) const
{
    std::string str;
    if (!GetNonFuncString(pos, str))
        return "";

    for (const AsbFunction &func : functions)
    {
        FixedMemoryStream code(const_cast<uint8_t *>(func.code.data()), func.code.size());
        int char_id = -1;

        uint8_t opcode;
        code.SavePos();

        while (code.Read8(&opcode))
        {
            if (opcode == 1) // Push C
            {
                uint32_t candidate;
                if (!code.Read32(&candidate))
                    break;

                if (candidate == pos)
                {
                    uint8_t data0[6];
                    static const uint8_t native0_call[6] = { 0x29, 0x00, 0x00, 0x00, 0x00, 0x01 };
                    code.SavePos();

                    if (!code.Read(data0, sizeof(data0)))
                        break;

                    if (memcmp(data0, native0_call, sizeof(data0)) == 0)
                    {
                        if (char_id >= 0)
                        {
                            if ((size_t)char_id >= char_names.size())
                                break;

                            return char_names[(size_t)char_id];
                        }
                    }

                    code.RestorePos();
                }

                uint8_t data1[11];

                code.SavePos();

                if (code.Read(data1, sizeof(data1)))
                {
                    // pushc XXXXXXXX + calln native1d/1e,2
                    if (data1[0] == 1 && data1[5] == 0x29 && (data1[6] == 0x1D || data1[6] == 0x1E) && data1[7] == 0 && data1[8] == 0 && data1[9] == 0 && data1[10] == 2)
                    {
                        char_id = (int)candidate;
                        continue;
                    }
                }

                code.RestorePos();
                continue;
            }

            code.RestorePos();
            if (!SkipInstruction(&code))
                break;

            code.SavePos();
        }
    }

    // Just for testing
    /*if (!(str.length() == 2 && (uint8_t)str[0] == 0x81 && str[1] == 0x40))
    {
        DPRINTF("Cannot find talker for string at pos %x\n", pos);
        exit(-1);
    }*/

    return "";
}

void AsbFile::Reset()
{
    name.clear();
    strings.clear();    
    functions.clear();
    defined_strings.clear();
    defined_functions.clear();
    unresolved_calls.clear();
}

bool AsbFile::Load(const uint8_t *buf, size_t size)
{
    FixedMemoryStream mem(const_cast<uint8_t *>(buf), size);
    ASBHeader hdr;

    Reset();

    if (!mem.Read(&hdr, sizeof(hdr)))
    {
        DPRINTF("%s: Failed to read header.\n", FUNCNAME);
        return false;
    }

    name = hdr.name;

    if (!mem.Seek(hdr.strings_start, SEEK_SET))
    {
        DPRINTF("%s: Premature end of file.\n", FUNCNAME);
        return false;
    }

    while (mem.Tell() != hdr.strings_start + hdr.strings_size)
    {
        std::string str;

        strings.push_back(AsbString());

        strings.back().pos = (uint32_t)(mem.Tell() - hdr.strings_start);

        if (!mem.ReadCString(strings.back().str))
        {
            DPRINTF("%s: Failed to read a string.\n", FUNCNAME);
            return false;
        }
    }

    if (!mem.Seek(hdr.func_defs_start, SEEK_SET))
    {
        DPRINTF("%s: Premature end of file (2).\n", FUNCNAME);
        return false;
    }

    std::vector<ASBFuncDef> defs;

    defs.resize(hdr.num_funcs);
    functions.resize(hdr.num_funcs);

    if (!mem.Read(defs.data(), defs.size()*sizeof(ASBFuncDef)))
    {
        DPRINTF("%s: Failed to read functions definitions.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < hdr.num_funcs; i++)
    {
        const ASBFuncDef &def = defs[i];
        AsbFunction &func = functions[i];

        if (!GetString(def.name, func.name))
        {
            DPRINTF("%s: Failed to get function %Id name.\n", FUNCNAME, i);
            return false;
        }

        func.pos = def.code_start;
        func.num_params = def.num_params;
        func.num_locals = def.num_locals;

        if (!mem.Seek(hdr.code_start + def.code_start, SEEK_SET))
        {
            DPRINTF("%s: Failed to seek to code start (function %s).\n", FUNCNAME, func.name.c_str());
            return false;
        }

        func.code.resize(def.size);

        if (!mem.Read(func.code.data(), def.size))
        {
            DPRINTF("%s: Failed to read function %s.\n", FUNCNAME, func.name.c_str());
            return false;
        }
    }

    return true;
}

uint8_t *AsbFile::Save(size_t *psize)
{
    MemoryStream out;
    ASBHeader hdr;

    strcpy_s(hdr.name, sizeof(hdr.name), name.c_str());
    hdr.func_defs_start = sizeof(hdr);
    hdr.num_funcs = (uint32_t)functions.size();
    hdr.code_start = hdr.func_defs_start + hdr.num_funcs*sizeof(ASBFuncDef);

    if (!out.Write(&hdr, sizeof(hdr)))
        return nullptr;

    for (const AsbFunction &func : functions)
    {
        ASBFuncDef def;

        def.name = FindString(func.name);
        if (def.name == 0xFFFFFFFF)
        {
            DPRINTF("%s: Internal error.\n", FUNCNAME);
            return nullptr;
        }

        def.num_params = func.num_params;
        def.num_locals = func.num_locals;
        def.code_start = func.pos;
        def.size = (uint32_t)func.code.size();

        if (!out.Write(&def, sizeof(def)))
            return nullptr;
    }

    for (const AsbFunction &func : functions)
    {
        if (!out.Write(func.code.data(), func.code.size()))
            return nullptr;
    }

    hdr.code_size = (uint32_t)(out.Tell() - hdr.code_start);
    if (!out.Align(4))
        return nullptr;

    hdr.strings_start = (uint32_t)out.Tell();

    for (const AsbString &str: strings)
    {
        if (!out.WriteString(str.str, true))
            return nullptr;
    }

    *psize = (size_t)out.GetSize();

    hdr.strings_size = (uint32_t)(*psize - hdr.strings_start);
    hdr.file_size = (uint32_t)*psize;

    if (!out.Seek(0, SEEK_SET) || !out.Write(&hdr, sizeof(hdr)))
        return nullptr;

    return out.GetMemory(true);
}

bool AsbFile::DisassembleCommon(Stream *out) const
{
    if (!out->WriteString("; This file should be read with Shift-JIS encoding.\n"))
        return false;

    if (!out->Printf("script %s\n\n", name.c_str()))
        return false;

    if (strings.size() > 0)
    {
        if (!out->WriteString("strings\n"))
            return false;

        for (size_t i = 0; i < strings.size(); i++)
        {
            std::string talker = GetTalker(strings[i].pos);

            if (talker.length() == 0)
            {
                if (!out->Printf("\tstr_%04x = %s\n", strings[i].pos, MakeString(strings[i].str, true).c_str()))
                    return false;
            }
            else
            {
                if (!out->Printf("\tstr_%04x = %s; %s\n", strings[i].pos, MakeString(strings[i].str, true).c_str(), talker.c_str()))
                    return false;
            }
        }

        if (!out->WriteString("endstrings\n\n"))
            return false;
    }

    for (const AsbFunction &function : functions)
    {
        if (!DisassembleFunction(function, out))
            return false;
    }

    return true;
}

bool AsbFile::Disassemble(const std::string &path) const
{
    FileStream out("wb");

    if (!out.LoadFromFile(path))
        return false;

    return DisassembleCommon(&out);
}

uint8_t *AsbFile::Disassemble(size_t *psize) const
{
    MemoryStream out;

    if (!DisassembleCommon(&out))
        return nullptr;

    *psize = (size_t)out.GetSize();
    return out.GetMemory(true);
}

void AsbFile::RemoveComments(std::string &line) const
{
    if (line.find(';') == std::string::npos)
        return;

    bool in_quotes = false;
    std::string new_line;

    char prev_ch = 0;

    for (size_t i = 0; i < line.length(); i++)
    {
        char ch = line[i];

        if (!in_quotes)
        {
            if (ch == ';')
                break;

            if (ch == '"')
                in_quotes = true;
        }
        else
        {
            if (ch == '"' && prev_ch != '\\')
                in_quotes = false;
        }

        new_line.push_back(ch);
        prev_ch = ch;
    }

    line = new_line;
}

std::string AsbFile::UsefulString(const std::string &str)
{
    std::string result;
    std::string temp = str;
    Utils::TrimString(temp);

    bool in_single_quote = false;
    bool in_double_quote = false;
    char prev = 0;

    for (char c : temp)
    {
        if (c == '\'' && prev != '\\')
        {
            if (in_single_quote)
            {
                in_single_quote = false;
            }
            else if (!in_double_quote)
            {
                in_single_quote = true;
            }
        }
        else if (c == '"' && prev != '\\')
        {
            if (in_double_quote)
            {
                in_double_quote = false;
            }
            else if (!in_single_quote)
            {
                in_double_quote = true;
            }
        }
        else if (c == '#' || c == ';')
        {
            if (!in_single_quote && !in_double_quote)
            {
                Utils::TrimString(result);
                return result;
            }
        }

        result.push_back(c);
        prev = c;
    }

    Utils::TrimString(result);
    return result;
}

bool AsbFile::GetAssignment(const std::string &str, std::string &left, std::string &right)
{
    char prev = 0;
    bool assignment_found = false;
    bool in_single_quote = false;
    bool in_double_quote = false;

    left.clear();
    right.clear();

    std::string temp = UsefulString(str);

    for (char c : temp)
    {
        if (assignment_found)
        {
            right.push_back(c);
        }
        else
        {
            if (c == '\'' && prev != '\\')
            {
                if (in_single_quote)
                {
                    in_single_quote = false;
                }
                else if (!in_double_quote)
                {
                    in_single_quote = true;
                }
            }
            else if (c == '"' && prev != '\\')
            {
                if (in_double_quote)
                {
                    in_double_quote = false;
                }
                else if (!in_single_quote)
                {
                    in_double_quote = true;
                }
            }
            else if (c == '=')
            {
                if (!in_single_quote && !in_double_quote)
                {
                    assignment_found = true;
                }
            }

            if (!assignment_found)
                left.push_back(c);
        }
    }

    if (!assignment_found)
        return false;

    Utils::TrimString(left);
    Utils::TrimString(right);

    return true;
}

bool AsbFile::ParseString(const std::string &raw, std::string &value)
{
    bool has_quotes = (raw.length() > 0 && (raw[0] == '"' || raw[0] == '\''));
    bool single_quotes = (has_quotes && raw[0] == '\'');
    bool quotes_closed = false;

    if (has_quotes)
    {
        value.clear();
    }
    else
    {
        if (raw.length() > 0)
            value = raw[0];
    }

    for (size_t i = 1; i < raw.length(); i++) // Start in second character
    {
        char c = raw[i];

        if (has_quotes)
        {
            if ((single_quotes && c == '\'') || (!single_quotes && c == '"'))
            {
                if (i != (raw.length()-1))
                {
                    // possible unintended use of quotes without \"\\\" prepended
                }

                quotes_closed = true;
                break;
            }
        }

        if (i != (raw.length()-1))
        {
            if (c == '\\' && raw[i+1] == 'n')
            {
                value.push_back('\n');
                i++;
                continue;
            }
            else if (c == '\\' && raw[i+1] == '\"')
            {
                value.push_back('\"');
                i++;
                continue;
            }
            else if (c == '\\' && raw[i+1] == '\'')
            {
                value.push_back('\'');
                i++;
                continue;
            }
        }

        value.push_back(c);
    }

    if (has_quotes && !quotes_closed)
        return false;

    return true;
}

bool AsbFile::IsNumber(const std::string &str)
{
    std::string strl = Utils::ToLowerCase(str);
    bool hex = Utils::BeginsWith(strl, "0x");
    size_t i = (hex) ? 2 : 0;

    if (hex && str.length() == 2)
        return false;

    for (; i < str.length(); i++)
    {
        char ch = str[i];
        bool allowed = false;

        if (ch >= '0' && ch <= '9')
            allowed = true;
        else if (hex && ch >= 'a' && ch <= 'f')
            allowed = true;
        else if (ch == '-' && i == 0)
            allowed = true;

        if (!allowed)
            return false;
    }

    return true;
}

bool AsbFile::WriteAtAbsoluteAddress(uint32_t addr, uint32_t val)
{
    if (functions.size() == 0)
        return false;

    for (AsbFunction &func: functions)
    {
        if (addr >= func.pos && addr <= (func.pos + (uint32_t)func.code.size()))
        {
            // Boundaries not checked, but anyway this is a internal function
            uint8_t *ptr = func.code.data();
            *(uint32_t *)(ptr + addr - func.pos) = val;
            return true;
        }
    }

    return false;
}

bool AsbFile::AssembleStrings(std::queue<AsbLine> &lines)
{
    std::vector<std::string> comps;
    Utils::GetMultipleStringsSpaces(lines.front().text, comps);

    if (comps[0] != "strings")
    {
        DPRINTF("%s: was expecting \"strings\" at line %d but got \"%s\".\n", FUNCNAME, lines.front().number, comps[0].c_str());
        return false;
    }

    if (comps.size() > 1)
    {
        DPRINTF("%s: Syntax error at line %d.\n", FUNCNAME, lines.front().number);
        return false;
    }

    lines.pop();
    uint32_t position = 0;

    while (!lines.empty())
    {
        AsbLine &line = lines.front();
        Utils::GetMultipleStringsSpaces(line.text, comps);

        if (comps[0] == "endstrings")
        {
            if (comps.size() > 1)
            {
                DPRINTF("%s: Syntax error at line %d.\n", FUNCNAME, line.number);
                return false;
            }

            if (strings.size() == 0)
            {
                DPRINTF("%s: At least one string must be defined!\n", FUNCNAME);
                return false;
            }

            lines.pop();
            return true;
        }
        else
        {
            std::string left, right_raw, right;
            if (!GetAssignment(line.text, left, right_raw) || !ParseString(right_raw, right))
            {
                DPRINTF("%s: Syntax error at line %d.\n", FUNCNAME, line.number);
                return false;
            }

            auto it = defined_strings.find(left);
            if (it != defined_strings.end())
            {
                DPRINTF("%s: Error at line %d, \"%s\" was already defined.\n", FUNCNAME, line.number, left.c_str());
                return false;
            }

            strings.push_back(AsbString());
            AsbString &string = strings.back();

            string.pos = position;
            string.str = right;
            defined_strings[left] = string;
            position += (uint32_t)right.length() + 1;

            /* ***** For testing */
            std::vector<std::string> llines;
            Utils::GetMultipleStrings(right, llines, '\n');
            if (llines.size() > 3)
            {
                DPRINTF("Warning, string with more than 3 lines. At line %d.\n", line.number);
            }
            for (const std::string &lline : llines)
            {
                if (lline.length() > 48)
                {
                    DPRINTF("Warning, string with a line bigger than 48. At line %d.\n", line.number);
                }
            }
            /* *********/

            lines.pop();
        }
    }

    DPRINTF("%s: end of file reached before \"endstrings\" found.\n", FUNCNAME);
    return false;
}

bool AsbFile::AssembleFunction(std::queue<AsbLine> &lines, uint32_t &code_addr)
{
    std::vector<std::string> comps;
    Utils::GetMultipleStringsSpaces(lines.front().text, comps);

    if (comps[0] != "function")
    {
        DPRINTF("%s: was expecting \"function\" at line %d but got \"%s\".\n", FUNCNAME, lines.front().number, comps[0].c_str());
        return false;
    }

    if (comps.size() == 1)
    {
        DPRINTF("%s: function name not defined, at line %d.\n", FUNCNAME, lines.front().number);
        return false;
    }
    else if (comps.size() != 2)
    {
        DPRINTF("%s: syntax error at line %d.\n", FUNCNAME, lines.front().number);
        return false;
    }

    Utils::GetMultipleStrings(std::string(comps[1]), comps, ',', false);
    if (comps[0].length() == 0)
    {
        DPRINTF("%s: empty function name, at line %d.\n", FUNCNAME, lines.front().number);
        return false;
    }

    if (comps.size() == 1)
    {
        DPRINTF("%s: number of params for function \"%s\" not defined, at line %d.\n", FUNCNAME, comps[0].c_str(), lines.front().number);
        return false;
    }
    else if (comps.size() != 2)
    {
        DPRINTF("%s: syntax error at line %d.\n", FUNCNAME, lines.front().number);
        return false;
    }

    auto it = defined_functions.find(comps[0]);
    if (it != defined_functions.end())
    {
        DPRINTF("%s: function \"%s\" was defined before, redefinition at line %d.\n", FUNCNAME, comps[0].c_str(), lines.front().number);
        return false;
    }

    if (FindString(comps[0]) == 0xFFFFFFFF)
    {
        DPRINTF("%s: function \"%s\" must be defined in a string too, line %d.\n", FUNCNAME, comps[0].c_str(), lines.front().number);
        return false;
    }

    if (!IsNumber(comps[1]))
    {
        DPRINTF("%s: cannot parse number of parameters, at line %d.\n", FUNCNAME, lines.front().number);
        return false;
    }

    functions.push_back(AsbFunction());
    AsbFunction &function = functions.back();

    function.name = comps[0];
    function.pos = code_addr;
    function.num_params = Utils::GetUnsigned(comps[1]);
    function.num_locals = 0;

    defined_functions[comps[0]] = function;

    MemoryStream code;
    std::unordered_map<std::string, uint32_t> defined_labels;
    std::unordered_map<uint32_t, std::string> unresolved_labels;
    std::unordered_map<std::string, uint8_t> defined_locals;

    lines.pop();
    while (!lines.empty())
    {
        AsbLine &line = lines.front();
        Utils::GetMultipleStringsSpaces(line.text, comps);

        if (comps[0] == "endfunction")
        {
            if (comps.size() > 1)
            {
                DPRINTF("%s: Syntax error at line %d.\n", FUNCNAME, line.number);
                return false;
            }

            if (code.GetSize() == 0)
            {
                DPRINTF("%s: function \"%s\" is empty, it needs at least one instruction!\n", FUNCNAME, function.name.c_str());
                return false;
            }

            for (const auto &it : unresolved_labels)
            {
                auto it2 = defined_labels.find(it.second);
                if (it2 == defined_labels.end())
                {
                    DPRINTF("%s: cannot resolve label \"%s\" in function \"%s\".\n", FUNCNAME, it.second.c_str(), function.name.c_str());
                    return false;
                }

                uint32_t local_addr = it.first - function.pos;
                if (!code.Seek(local_addr, SEEK_SET))
                {
                    DPRINTF("%s: Some weird error happened.\n", FUNCNAME);
                    return false;
                }

                code.Write32(it2->second);
            }

            function.code.resize((size_t)code.GetSize());
            memcpy(function.code.data(), code.GetMemory(false), function.code.size());
            function.num_locals = (uint32_t)defined_locals.size();
            lines.pop();
            return true;
        }
        else if (line.text.back() == ':')
        {
            if (comps.size() > 1)
            {
                DPRINTF("%s: Syntax error at line %d.\n", FUNCNAME, line.number);
                return false;
            }

            std::string label_name = comps[0].substr(0, comps[0].length()-1);
            auto it = defined_labels.find(label_name);
            if (it != defined_labels.end())
            {
                DPRINTF("%s: label \"%s\" redefined at line %d.\n", FUNCNAME, label_name.c_str(), line.number);
                return false;
            }

            defined_labels[label_name] = code_addr;
            lines.pop();
        }
        else
        {
            Utils::GetMultipleStringsSpaces(line.text, comps);
            const AsbInstDef *inst_def = nullptr;

            for (uint8_t i = 0; i < (uint8_t)inst_defs.size(); i++)
            {
                if (inst_defs[i].name == comps[0])
                {
                    inst_def = &inst_defs[i];
                    break;
                }
            }

            if (inst_def == nullptr)
            {
                DPRINTF("%s: opcode \"%s\" not recognized, at line %d.\n", FUNCNAME, comps[0].c_str(), line.number);
                return false;
            }

            code.Write8((uint8_t)inst_def->opcode);
            code_addr++;

            Utils::GetMultipleStrings(line.text.substr(comps[0].length()), comps, ',');
            if (comps.size() != inst_def->op_types.size() && inst_def->opcode != 0x24) // 24 = jmpa, special case
            {
                DPRINTF("%s: invalid number of parameters for instruction, at line %d.\n", FUNCNAME, line.number);
                return false;
            }

            for (size_t i = 0; i< inst_def->op_types.size(); i++)
            {
                switch (inst_def->op_types[i])
                {
                    case OP_LOCAL:
                    {
                        auto it = defined_locals.find(comps[i]);
                        if (it == defined_locals.end())
                        {
                            if (defined_locals.size() == 256)
                            {
                                DPRINTF("%s: overflow of local vars. Max of 256 per function, error at line %d.\n", FUNCNAME, line.number);
                                return false;
                            }

                            uint8_t lnum = (uint8_t)defined_locals.size();
                            defined_locals[comps[i]] = lnum;
                            code.Write8(lnum);
                        }
                        else
                        {
                            code.Write8(it->second);
                        }

                        code_addr++;
                    }
                    break;

                    case OP_U8:
                    {
                        if (!IsNumber(comps[i]))
                        {
                            DPRINTF("%s: was expecting a number, got \"%s\", at line %d.\n", FUNCNAME, comps[i].c_str(), line.number);
                            return false;
                        }

                        uint32_t var = Utils::GetUnsigned(comps[i], 256);
                        if (var >= 256)
                        {
                            DPRINTF("%s: error, was expecting a 8-bit number, but got \"%s\", at line %d.\n", FUNCNAME, comps[i].c_str(), line.number);
                            return false;
                        }

                        code.Write8((uint8_t)var);
                        code_addr++;
                    }
                    break;

                    case OP_S32:
                    {
                        if (!IsNumber(comps[i]))
                        {
                            DPRINTF("%s: was expecting a number, got \"%s\", at line %d.\n", FUNCNAME, comps[i].c_str(), line.number);
                            return false;
                        }

                        code.Write32(Utils::GetUnsigned(comps[i]));
                        code_addr += sizeof(uint32_t);
                    }
                    break;

                    case OP_S32_OR_STR:
                    {
                        if (IsNumber(comps[i]))
                        {
                            code.Write32(Utils::GetUnsigned(comps[i]));
                        }
                        else
                        {
                            auto it = defined_strings.find(comps[i]);
                            if (it == defined_strings.end())
                            {
                                DPRINTF("%s: reference to undefined string \"%s\", at line %d.\n", FUNCNAME, comps[i].c_str(), line.number);
                                return false;
                            }

                            code.Write32(it->second.pos);
                        }

                        code_addr += sizeof(uint32_t);
                    }
                    break;

                    case OP_ARRAY:
                    {
                        // Untested code, since non of the game asb use this
                        if (comps.size() >= 256)
                        {
                            DPRINTF("%s: array too big, line %d.\n", FUNCNAME, line.number);
                            return false;
                        }

                        code.Write8((uint8_t)comps.size());
                        code_addr++;

                        for (const std::string &str : comps)
                        {
                            if (!IsNumber(str))
                            {
                                DPRINTF("%s: was expecting a number but got \"%s\", at line %d.\n", FUNCNAME, str.c_str(), line.number);
                                return false;
                            }

                            code.Write32(Utils::GetUnsigned(str));
                            code_addr += sizeof(uint32_t);
                        }
                    }
                    break;

                    case OP_FUNC:
                    {
                        uint32_t func_idx = GetFunctionIdx(comps[i]);
                        if (func_idx == 0xFFFFFFFF)
                        {
                            unresolved_calls[code_addr] = comps[i];
                            code.Write32(0xDEADC0DE);
                        }
                        else
                        {
                            code.Write32(func_idx);
                        }

                        code_addr += sizeof(uint32_t);
                    }
                    break;

                    case OP_NFUNC:
                    {
                        if (!Utils::BeginsWith(comps[i], "@native"))
                        {
                            DPRINTF("%s: Unrecognized native function \"%s\" at line %d.\n", FUNCNAME, comps[i].c_str(), line.number);
                            return false;
                        }

                        std::string numstr = "0x" + comps[i].substr(7);
                        if (!IsNumber(numstr))
                        {
                            DPRINTF("%s: Unrecognized native function \"%s\" at line %d.\n", FUNCNAME, comps[i].c_str(), line.number);
                            return false;
                        }

                        code.Write32(Utils::GetUnsigned(numstr));
                        code_addr += sizeof(uint32_t);
                    }
                    break;

                    case OP_LABEL:
                    {
                        auto it = defined_labels.find(comps[i]);
                        if (it == defined_labels.end())
                        {
                            unresolved_labels[code_addr] = comps[i];
                            code.Write32(0xDEADC0DE);
                        }
                        else
                        {
                            code.Write32(it->second);
                        }

                        code_addr += sizeof(uint32_t);
                    }
                    break;
                }
            }

            lines.pop();
        }
    }

    DPRINTF("%s: end of file reached before \"endfunction\" found.\n", FUNCNAME);
    return false;
}

bool AsbFile::AssembleCommon(const std::string &in)
{
    std::vector<std::string> real_lines;
    std::queue<AsbLine> lines;

    Reset();

    Utils::GetMultipleStrings(in, real_lines, '\n', false);

    for (size_t i = 0; i < real_lines.size(); i++)
    {
        RemoveComments(real_lines[i]);
        Utils::TrimString(real_lines[i]);

        if (real_lines[i].length() == 0)
            continue;

        AsbLine line;

        line.text = real_lines[i];
        line.number = (uint32_t)(i+1);
        lines.push(line);
    }

    if (lines.empty())
    {
        DPRINTF("%s: premature end of file.\n", FUNCNAME);
        return false;
    }

    std::vector<std::string> script_def;
    Utils::GetMultipleStringsSpaces(lines.front().text, script_def);

    if (script_def.size() == 0)
    {
        DPRINTF("%s: Internal error, should not be here.\n", FUNCNAME);
        return false;
    }

    if (script_def[0] != "script")
    {
        DPRINTF("%s: was expecting \"script\" but got %s\n", FUNCNAME, script_def[0].c_str());
        return false;
    }

    if (script_def.size() != 2)
    {
        DPRINTF("%s: syntax error at line %d (expecting 2 elements, got %Id)\n", FUNCNAME, lines.front().number, script_def.size());
        return false;
    }

    name = script_def[1];
    lines.pop();

    if (lines.empty())
    {
        DPRINTF("%s: premature end of file.\n", FUNCNAME);
        return false;
    }

    if (!AssembleStrings(lines))
        return false;

    uint32_t code_addr = 0;
    while (!lines.empty())
    {
        if (!AssembleFunction(lines, code_addr))
            return false;
    }

    if (functions.size() == 0)
    {
        DPRINTF("%s: at least one function should have been defined.\n", FUNCNAME);
        return false;
    }

    for (const auto &it : unresolved_calls)
    {
        uint32_t func_idx = GetFunctionIdx(it.second);
        if (func_idx == 0xFFFFFFFF)
        {
            DPRINTF("%s: Unresolved reference to function \"%s\".", FUNCNAME, it.second.c_str());
            return false;
        }

        if (!WriteAtAbsoluteAddress(it.first, func_idx))
        {
            DPRINTF("%s: Weeeeird erroooor, shouldn't have happened, you shoooould not see this.\n", FUNCNAME);
            return false;
        }
    }

    return true;
}

bool AsbFile::Assemble(const std::string &path)
{
    std::string text;
    if (!Utils::ReadTextFile(path, text))
        return false;

    return AssembleCommon(text);
}

bool AsbFile::Assemble(const uint8_t *buf, size_t size)
{
    char *str = new char[size+1];
    memcpy(str, buf, size);
    str[size] = 0;

    std::string text = str;
    delete[] str;

    return AssembleCommon(text);
}
