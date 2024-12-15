#ifndef UTILSXML_H
#define UTILSXML_H

#include "Utils.h"

namespace Utils
{
    TiXmlElement *FindRoot(TiXmlHandle *handle, const std::string &root_name);

    uint32_t GetUnsigned(const std::string &str, uint32_t default_value=0);
    uint64_t GetUnsigned64(const std::string &str, uint64_t default_value=0);
    int32_t GetSigned(const std::string &str, int32_t default_value=0);
    bool GetBoolean(const std::string &str, bool default_value=false);
    float GetFloat(const std::string &str, float default_value=0.0f);

    uint32_t GetShortVersion(uint32_t version);
    uint32_t GetLongVersion(uint32_t version);

    size_t GetElemCount(const TiXmlElement *root, const char *name, const TiXmlElement **first=nullptr);

    void WriteParamString(TiXmlElement *root, const char *name, const std::string &value);
    void WriteParamMultipleStrings(TiXmlElement *root, const char *name, const std::vector<std::string> &values);

    void WriteParamUnsigned(TiXmlElement *root, const char *name, uint64_t value, bool hexadecimal=false);
    void WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint32_t> &values, bool hexadecimal=false);
    void WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint16_t> &values, bool hexadecimal=false);
    void WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint8_t> &values, bool hexadecimal=false);

    void WriteParamSigned(TiXmlElement *root, const char *name, int64_t value);

    void WriteParamFloat(TiXmlElement *root, const char *name, float value);
    void WriteParamMultipleFloats(TiXmlElement *root, const char *name, const std::vector<float> &values);

    void WriteParamGUID(TiXmlElement *root, const char *name, const uint8_t *value);
    void WriteParamBlob(TiXmlElement *root, const char *name, const uint8_t *value, size_t size);

    void WriteParamBoolean(TiXmlElement *root, const char *name, bool value);

    bool ReadAttrString(const TiXmlElement *root, const char *name, std::string &value);
    bool ReadAttrMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, char separator=',', bool omit_empty=true);
    bool ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint32_t *value);
    bool ReadAttrUnsigned(const TiXmlElement *root, const char *name, uint64_t *value);
    bool ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint16_t *value);
    bool ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint8_t *value);
    bool ReadAttrSigned(const TiXmlElement *root,  const char *name, int32_t *value);
    bool ReadAttrSigned(const TiXmlElement *root,  const char *name, int16_t *value);
    bool ReadAttrSigned(const TiXmlElement *root,  const char *name, int8_t *value);
    bool ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values);
    bool ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count);
    bool ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, std::vector<int32_t> &values);
    bool ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, int32_t *values, size_t count);
    bool ReadAttrFloat(const TiXmlElement *root,  const char *name, float *value);
    bool ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values);
    bool ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count);
    bool ReadAttrBoolean(const TiXmlElement *root, const char *name, bool *value);

    bool ReadParamString(const TiXmlElement *root, const char *name, std::string & value, const TiXmlElement **ret=nullptr);
    bool ReadParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, const TiXmlElement **ret=nullptr);

    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value);
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value);
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value);
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value);

    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values);

    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count);

    bool ReadParamSigned(const TiXmlElement *root, const char *name, int32_t *value);

    bool ReadParamFloat(const TiXmlElement *root, const char *name, float *value);
    bool ReadParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values);
    bool ReadParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count);

    bool ReadParamGUID(const TiXmlElement *root, const char *name, uint8_t *value);
    uint8_t *ReadParamBlob(const TiXmlElement *root, const char *name, size_t *psize);

    bool ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool ReadParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);

    bool ReadParamBoolean(const TiXmlElement *root, const char *name, bool *value);

    bool GetParamString(const TiXmlElement *root, const char *name, std::string &value, const TiXmlElement **ret=nullptr);
    bool GetParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, const TiXmlElement **ret=nullptr);

    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value);
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value);
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value);
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value);

    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values);

    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count);

    bool GetParamSigned(const TiXmlElement *root, const char *name, int32_t *value);

    bool GetParamFloat(const TiXmlElement *root, const char *name, float *value);
    bool GetParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values);
    bool GetParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count);

    bool GetParamGUID(const TiXmlElement *root, const char *name, uint8_t *value);
    uint8_t *GetParamBlob(const TiXmlElement *root, const char *name, size_t *psize);

    bool GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool GetParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);

    void WriteComment(TiXmlElement *root, const std::string & comment);
}


#endif // UTILSXML_H
