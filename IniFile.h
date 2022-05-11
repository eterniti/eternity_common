#ifndef __INIFILE_H__
#define __INIFILE_H__

#include <stdlib.h>

#include "BaseFile.h"

class IniFile : public BaseFile
{
private:

    struct Line
    {
        std::string content;
        std::string section;
        bool is_comment;
        bool is_section;
    };

    struct IntegerConstant
    {
        std::string name;
        int64_t value;
    };

    std::vector<Line> lines;

    std::vector<IntegerConstant> integer_constants;
	
    std::string UsefulString(const std::string &str);
    bool GetAssignment(const std::string &str, std::string &left, std::string &right);

    bool GetRawValue(const std::string &section, const std::string &name, std::string &value);
    bool SetRawValue(const std::string &section, const std::string &name, const std::string &value, bool must_exist);

    void ParseString(const std::string &raw, std::string &value, const std::string section, const std::string &name);
	
	void Reset();
			
public:

	IniFile();
    virtual ~IniFile() override;
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    bool GetIntegerValue(const std::string &section, const std::string &name, int *value, int error_value=0);
    bool GetIntegerValue(const std::string &section, const std::string &name, int64_t *value, int error_value=0);
    bool SetIntegerValue(const std::string &section, const std::string &name, int64_t value, bool hexadecimal=false, bool must_exist=false);
	

    bool GetStringValue(const std::string &section, const std::string &name, std::string &value, const std::string error_value = "");
    bool SetStringValue(const std::string &section, const std::string &name, const std::string &value,  bool with_quotes=true, bool must_exist=false);
	
    bool GetBooleanValue(const std::string &section, const std::string &name, bool *value, bool error_value=false);
    bool SetBooleanValue(const std::string &section, const std::string &name, bool value, bool must_exist=false);

    bool GetFloatValue(const std::string &section, const std::string &name, float *value, float error_value=0.0f);
    bool SetFloatValue(const std::string &section, const std::string &name, float value, bool must_exist=false);

    // Todo: add rest of multiple
    bool GetMultipleIntegersValues(const std::string &section, const std::string &name, std::vector<int> &values, bool clear_on_error=false);
    bool GetMultipleIntegersValues(const std::string &section, const std::string &name, std::vector<int64_t> &values, bool clear_on_error=false);
    bool SetMultipleIntegersValues(const std::string &section, const std::string &name, const std::vector<int> &values, bool hexadecimal=false, bool must_exist=false);
    bool SetMultipleIntegersValues(const std::string &section, const std::string &name, const std::vector<int64_t> &values, bool hexadecimal=false, bool must_exist=false);

    bool GetMultipleStringsValues(const std::string &section, const std::string &name, std::vector<std::string> &values, bool clear_on_error=false);
    bool SetMultipleStringsValues(const std::string &section, const std::string &name, const std::vector<std::string> &values, bool must_exist=false);

    size_t GetAllStringsValues(const std::string &section, std::vector<std::string> &names, std::vector<std::string> &values);

    void AddIntegerConstant(const std::string &name, int64_t value);

    bool SectionExists(const std::string &name) const;
};

#endif
