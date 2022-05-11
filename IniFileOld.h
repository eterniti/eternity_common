#ifndef __INIFILE_H__
#define __INIFILE_H__

#include <stdlib.h>

#include "BaseFile.h"

class IniFile : public BaseFile
{
private:
	char *data; // Allocated
	unsigned int data_size;
	char *ptr_current;
	char *section_current; // Allocated
	char *section_current_top;
	char *begin_of_latest_line;
	
	bool Resize(unsigned int new_size);
	bool Increase(unsigned int add_size, char **old_limit = NULL);
	
	void RemoveEndSpaces(char *line);
	
	char *ReadLine();
	bool WriteLine(const char *line);
	
	void RemoveComments(char *line);
	bool IsSection(const char *line);
	
	char *GetValueName(const char *line);
	char *GetValueData(const char *line);
	
	int NumChars(const char *str, char ch);
	int NumStrings(const char *str, const char *sub);
	
	void Reset();
			
public:

	IniFile();
	virtual ~IniFile();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
	bool FindSection(const char *name);
	bool AddSection(const char *name);
	
	char *GetRawValue(const char *section, const char *name);
	bool SetRawValue(const char *section, const char *name, const char *value, bool must_exist);
	
	bool GetIntegerValue(const char *section, const char *name, int *value);
	bool SetIntegerValue(const char *section, const char *name, int value, bool must_exist, bool hexadecimal = false);
	
	char *GetStringValue(const char *section, const char *name);
	bool SetStringValue(const char *section, const char *name, const char *value, bool must_exist, bool with_quotes = true);
	
	bool GetBooleanValue(const char *section, const char *name, bool default_true);
	bool SetBooleanValue(const char *section, const char *name, bool value, bool must_exist);
	
	bool RemoveValue(const char *section, const char *name, bool remove_by_comments); // TODO: incomplete, do not use
	
	bool WriteRawLine(const char *line);
	bool WriteRawLineAt(const char *section, const char *line);
	bool WriteRawLineAt(const char *section, const char *name, const char *line);
};

#endif
