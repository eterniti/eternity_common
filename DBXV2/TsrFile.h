#ifndef TSRFILE_H
#define TSRFILE_H

#include "FixedMemoryStream.h"

enum ArgumentType
{
    ARG_TYPE_INTEGER = 1,
    ARG_TYPE_FLOAT,
    ARG_TYPE_STRING,
    ARG_TYPE_TAG,
    ARG_TYPE_VARIABLE,
    ARG_TYPE_EXPRESSION,
    ARG_TYPE_MAX
};

struct TsrTag
{
    std::string name;
    uint16_t index;

    bool Load(FixedMemoryStream &stream, size_t &enc_pos);
    size_t CalculateSize() const;
    bool Save(FixedMemoryStream &stream, size_t &enc_pos) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TsrSentenceArg
{
    std::string string_value;
    float float_value;
    uint32_t uint32_value;

    // 1 -> assume integer
    // 2 -> float
    // 3-6 -> string. 3:Literal 4: TAG, 5: variable name, 6: expression
    uint16_t type;

    bool Load(FixedMemoryStream &stream, size_t &enc_pos);
    size_t CalculateSize() const;
    bool Save(FixedMemoryStream &stream, size_t &enc_pos) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TsrSentence
{
    std::string name;
    uint16_t unk;
    std::vector<TsrSentenceArg> args;

    bool Load(FixedMemoryStream &stream, size_t &enc_pos);
    size_t CalculateSize() const;
    bool Save(FixedMemoryStream &stream, size_t &enc_pos) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

struct TsrFunction
{
    std::string name;
    uint16_t unk;
    std::vector<TsrTag> tags;
    std::vector<TsrSentence> sentences;

    bool Load(FixedMemoryStream &stream);
    size_t CalculateSize() const;
    bool Save(FixedMemoryStream &stream) const;

    TiXmlElement *Decompile(TiXmlNode *root) const;
    bool Compile(const TiXmlElement *root);
};

class TsrFile : public BaseFile
{
private:

    std::vector<TsrFunction> functions;

protected:

    void Reset();
    size_t CalculateSize() const;

public:
    TsrFile();
    virtual ~TsrFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    virtual TiXmlDocument *Decompile() const override;
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;

};

#endif // TSRFILE_H
