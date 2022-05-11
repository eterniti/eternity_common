#include <stdio.h>
#include <string.h>
#include "IniFileOld.h"

#include "debug.h"

#define MAX_LINE	2048

IniFile::IniFile()
{
	this->data = NULL;
	this->section_current = NULL;
	Reset();
}

void IniFile::Reset()
{
	if (this->data)
	{
		delete[] this->data;
		this->data = NULL;
	}
	
	if (this->section_current)
	{
		delete[] this->section_current;
		this->section_current = NULL;
	}
	
	this->data_size = 0;
	this->ptr_current = NULL;
	this->section_current_top = NULL;
	this->begin_of_latest_line = NULL;
}

IniFile::~IniFile()
{
	Reset();
}

bool IniFile::Resize(unsigned int new_size)
{
	char *buf;
		
	if (this->data && new_size <= this->data_size)
	{
		this->data_size = new_size;
		return true;
	}
	
    buf = new char[new_size];
	memset(buf, 0, new_size);
	
	if (this->data)
	{
		int position;
		int position_sc;
	
		position = (int) (this->ptr_current - this->data);	
		this->ptr_current = buf + position;		
		
		if (this->section_current)
		{
			position_sc = (int)(this->section_current_top - this->data);
			this->section_current_top = buf + position_sc;
		}
		
		memcpy(buf, this->data, this->data_size);
		free(this->data);		
	}
	else
	{
		this->ptr_current = buf;
	}
	
	this->data = buf;
	this->data_size = new_size;
	
	return true;
}

bool IniFile::Increase(unsigned int add_size, char **old_limit)
{
	bool ret;
	int old_size = this->data_size;

	ret = this->Resize(old_size + add_size);
	
	if (ret && old_limit)
	{
		*old_limit = this->data + old_size;
	}
	
	return ret;
}

void IniFile::RemoveEndSpaces(char *line)
{
    for (size_t i = strlen(line)-1; i >= 0; i--)
	{
		if (line[i] <= 0x20)
			line[i] = 0;
		else
			break;
	}
}

char *IniFile::ReadLine()
{
	char *line;
	char *limit;
	int i = 0;
	
	limit = this->data + this->data_size;
	
	if (this->ptr_current >= limit)
		return NULL;
		
    line = new char[MAX_LINE];
	
	// Skip spaces
	while (this->ptr_current < limit)
	{
		if (*this->ptr_current > 0x20)
			break;
			
		this->ptr_current++;
	}
	
	this->begin_of_latest_line = this->ptr_current;
	
	if (this->ptr_current >= limit)
	{
		/*delete[] line;
		return NULL;*/
		*line = 0;
		return line;
	}
	
	while (i < (MAX_LINE-1) && this->ptr_current < limit)
	{
		if ((this->ptr_current+1) < limit)
		{
			if (*this->ptr_current == '\r' && *(this->ptr_current + 1) == '\n')
				this->ptr_current++;
		}
		
		if (*this->ptr_current == '\n')
		{
			this->ptr_current++;
			break;
		}
		
		line[i++] = *this->ptr_current;
		this->ptr_current++;
	}	
	
	if (i == 0)
	{
		fprintf(stderr, "Unexpected empty line, this is a bug in parser.\n");
		delete[] line;
		return NULL;
	}
	
	line[i++] = 0;	
	this->RemoveEndSpaces(line);	
	
	return line;
}

bool IniFile::WriteLine(const char *line)
{	
	int size1, size2;
	int len;
	
	if (this->ptr_current > (this->data + this->data_size))
		this->ptr_current = this->data + this->data_size;
		
    size1 = this->data_size - (int)(this->ptr_current - this->data);
    len = (int)strlen(line);
	size2 = len + 2;
	
	if (!this->Increase(size2))
		return false;
		
	if (size1 > 0)
		memcpy(this->ptr_current + size2, this->ptr_current, size1);
		
	if (len > 0)
		memcpy(this->ptr_current, line, len);
		
	this->ptr_current[len] = '\r';
	this->ptr_current[len+1] = '\n';	
	this->ptr_current += size2;
	
	return true;
}

void IniFile::RemoveComments(char *line)
{
	bool in_single_quote = false;
	bool in_double_quote = false;
	bool comments = false;
	
	for (unsigned int i = 0; i < strlen(line); i++)
	{
		if (line[i] == '\'' && (i == 0 || line[i-1] != '\\'))
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
		else if (line[i] == '"' && (i == 0 || line[i-1] != '\\'))
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
		else if (line[i] == ';' || line[i] == '#')
		{
			if (!in_single_quote && !in_double_quote)
			{
				comments = true;
				line[i] = 0;
				break;
			}
		}
	}
	
	if (!comments)
		return;
	
	this->RemoveEndSpaces(line);
}

bool IniFile::IsSection(const char *line)
{
    int len = (int)strlen(line);
	
	if (len <= 2)
		return false;
		
	return (line[0] == '[' && line[len-1] == ']');
}

char *IniFile::GetValueName(const char *line)
{
	char *name;
    const char *eq;
	int copy_size;

	if (strlen(line) <= 2)
		return NULL;
		
	if (*line == '[')
		return NULL;
		
	eq = strchr(line, '=');
	if (!eq || eq == line)
		return NULL;
		
	copy_size = (int)(eq - line);
	name = new char[copy_size+1];
	
	if (!name)
		return NULL;
		
	memcpy(name, line, copy_size);
	name[copy_size] = 0;
	this->RemoveEndSpaces(name);
	
	return name;
}

char *IniFile::GetValueData(const char *line)
{
	char *data;
    const char *eq;
		
	if (strlen(line) <= 2)
		return NULL;
		
	if (*line == '[')
		return NULL;
		
	eq = strchr(line, '=');
	if (!eq || eq == line)
		return NULL;
		
	eq++;
		
	while (*eq <= 0x20 && *eq != 0)
	{
		eq++;
	}
	
	if (strlen(eq) < 1)
		return NULL;
		
	data = new char[strlen(eq)+1];
	if (!data)
		return NULL;
		
	strcpy(data, eq);
	return data;
}

int IniFile::NumChars(const char *str, char ch)
{
	int n = 0;
	
	for (unsigned int i = 0; i < strlen(str); i++)
	{
		if (str[i] == ch)
			n++;
	}
	
	return n;
}

int IniFile::NumStrings(const char *str, const char *sub)
{
	int n = 0;
	const char *p = str;
	
	while (1)
	{
		p = strstr(p, sub);
		if (!p)
			break;
			
		p += strlen(sub);
		n++;
	}
	
	return n;
}

uint8_t *IniFile::Save(size_t *size)
{
    uint8_t *buf = new uint8_t[data_size];

	memcpy(buf, data, data_size);
	*size = data_size;
	
	return buf;
}

bool IniFile::Load(const uint8_t *buf, size_t size)
{	
	Reset();
	
	this->ptr_current = NULL;
	this->data_size = 0;
	
	if (!buf || size == 0)
		return true;

    if (size >= 0xFFFFFFFF)
        return false;
		
    if (!this->Resize((unsigned int)size))
		return false;	
		
	memcpy(this->data, buf, size);
	return true;
}

bool IniFile::FindSection(const char *name)
{
	char *line;
	
	if (!this->data)
		return false;
		
	if (this->section_current && strcasecmp(name, section_current) == 0)
	{
		this->ptr_current = this->section_current_top;
		return true;
	}
	
	if (this->section_current)
	{
		delete[] this->section_current;
		this->section_current = NULL;
		this->section_current_top = NULL;
	}
		
	this->ptr_current = this->data;
	
	while ((line = this->ReadLine()))
	{
		this->RemoveComments(line);
				
		if (this->IsSection(line))
		{
			char *comp_start, *comp_end;
			int compare_size;
							
			comp_start = line+1;
							
			for (unsigned int i = 0; i < strlen(line); i++)
			{
				if (*comp_start <= 0x20)
					comp_start++;
				else
					break;
			}
			
			if (*comp_start == ']')
			{
				delete[] line;
				continue;
			}
			
			comp_end = comp_start + strlen(comp_start) - 1; // points to ']' 
			
            for (int i = (int)strlen(comp_start)-2; i >= 0; i--)
			{
				if (comp_start[i] <= 0x20)
					comp_end--;
				else
					break;
			}
				
            compare_size = (int)(comp_end - comp_start);
			if (strncasecmp(name, comp_start, compare_size) == 0)
			{
				delete[] line;
				
				if (this->section_current)
					delete[] this->section_current;					
				
				this->section_current = new char[strlen(name) + 1];
				if (this->section_current)
				{
					strcpy(this->section_current, name);
					this->section_current_top = this->ptr_current;
				}
				
				return true;
			}
		}
				
		delete[] line;
	}
	
	return false;
}

bool IniFile::AddSection(const char *name)
{
	bool ret;
	char *line;
	
	if (this->FindSection(name))
		return true;
		
	line = new char[strlen(name)+3];	
	if (!line)
		return false;
		
	this->ptr_current = this->data + this->data_size;
		
	sprintf(line, "[%s]", name);	
	ret = this->WriteLine(line);
	delete[] line;
	
	return ret;
}

char *IniFile::GetRawValue(const char *section, const char *name)
{
	char *line;
	
	if (!this->FindSection(section))
		return NULL;
		
	while ((line = this->ReadLine()))
	{
		char *c_name;
		
		this->RemoveComments(line);
		
		if (this->IsSection(line))
		{
			delete[] line;
			break;
		}
		
		c_name = this->GetValueName(line);
		if (c_name)
		{
			if (strcasecmp(c_name, name) == 0)
			{
				char *value = this->GetValueData(line);
				
				if (value)
				{
					delete[] c_name;
					delete[] line;
					
					return value;
				}
			}
			
			delete[] c_name;
		}
		
		delete[] line;
	}
	
	return NULL;
}

bool IniFile::SetRawValue(const char *section, const char *name, const char *value, bool must_exist)
{
	bool ret;
	char *line, *prev_line;
	
	if (!this->FindSection(section))
		return false;
		
	prev_line = this->ptr_current;
			
	while ((line = this->ReadLine()))
	{
		char *c_name;
		
		this->RemoveComments(line);
		
		if (this->IsSection(line))
		{
			delete[] line;
			break;
		}
		
		c_name = this->GetValueName(line);
		if (c_name)
		{
			if (strcasecmp(c_name, name) == 0)
			{
				fprintf(stderr, "Modification of value currently not implemented.\n");
				delete[] c_name;
				delete[] line;
				return false;
			}
			
			delete[] c_name;
		}
		
		prev_line = this->ptr_current;
		delete[] line;
	}
	
	if (must_exist)
		return false;
		
	this->ptr_current = prev_line;	
		
	line = new char[strlen(name) + strlen(value) + 4];
	if (!line)
		return false;
		
	sprintf(line, "%s = %s", name, value);	
	ret = this->WriteLine(line);	
	delete[] line;
	
	return ret;
}

bool IniFile::GetIntegerValue(const char *section, const char *name, int *value)
{
	char *raw = this->GetRawValue(section, name);
	
	if (!raw)
		return false;
		
	*value = strtol(raw, NULL, 0);
	delete[] raw;
	
	return true;
}

bool IniFile::SetIntegerValue(const char *section, const char *name, int value, bool must_exist, bool hexadecimal)
{
	char raw[32];
	
	if (!hexadecimal)
		sprintf(raw, "%d", value);
	else
		sprintf(raw, "0x%x", value);
		
	return this->SetRawValue(section, name, raw, must_exist);
}

char *IniFile::GetStringValue(const char *section, const char *name)
{
	char *raw, *ret;
	unsigned int raw_i, ret_i;
	bool single_quote;
	bool has_quotes = true;
	int n_special_chars;
	bool quotes_closed = false;
	
	raw = this->GetRawValue(section, name);
	
	if (!raw)
		return NULL;
		
	n_special_chars = this->NumStrings(raw, "\\n");
	n_special_chars += this->NumStrings(raw, "\\\"");
	n_special_chars += this->NumStrings(raw, "\\'");
		
	if (*raw == '\'')
	{
		single_quote = true;
	}
	else if (*raw == '"')
	{
		single_quote = false;
	}
	else
	{
		if (n_special_chars == 0)
			return raw;
		
		has_quotes = false;
	}
	
	ret = new char[(strlen(raw)-n_special_chars+1)];
	if (!ret)
	{
		delete[] raw;
		return NULL;
	}
	
	for (raw_i = 1, ret_i = 0; raw_i < strlen(raw); raw_i++)
	{
		if (has_quotes)
		{
			if ((single_quote && raw[raw_i] == '\'') || (!single_quote && raw[raw_i] == '"'))
			{
				if (raw_i != (strlen(raw)-1) && raw[raw_i+1] > 0x20)
				{
					fprintf(stderr, "Warning: possible unintended bad use of quotes without \"\\\" prepended in %s:%s.\n", section, name);
				}				
				
				quotes_closed = true;
				break;
			}
		}
		
		if (raw_i != (strlen(raw)-1))
		{
			if (raw[raw_i] == '\\' && raw[raw_i+1] == 'n')
			{
				ret[ret_i++] = '\n';
				raw_i++;
				continue;
			}
			else if (raw[raw_i] == '\\' && raw[raw_i+1] == '\"')
			{
				ret[ret_i++] = '\"';
				raw_i++;
				continue;
			}
			else if (raw[raw_i] == '\\' && raw[raw_i+1] == '\'')
			{
				ret[ret_i++] = '\'';
				raw_i++;
				continue;
			}
		}
			
		ret[ret_i++] = raw[raw_i];		
	}	
	
	if (has_quotes && !quotes_closed)
	{
		fprintf(stderr, "Warning: unclosed quotes at %s:%s.\n", section, name);
	}
	
	ret[ret_i] = 0;	
	delete[] raw;
	
	return ret;
}

bool IniFile::SetStringValue(const char *section, const char *name, const char *value, bool must_exist, bool with_quotes)
{
	bool ret;
	char *raw;
	int n_special_chars;
	int len;
	unsigned int val_i, raw_i;
	
	n_special_chars = this->NumChars(value, '\n');
	n_special_chars += this->NumChars(value, '"');
	
    len = n_special_chars + (int)strlen(value) + ((with_quotes) ? 2 : 0);
	
	raw = new char[len+1];
	if (!raw)
		return false;
		
	if (strlen(value) == 0)
	{
		if (with_quotes)
		{
			raw[0] = '"';
			raw[1] = '"';
		}
	}
	else
	{		
		for (val_i = 0, raw_i = 0; val_i < strlen(value); val_i++)
		{
			if (with_quotes && val_i == 0)
			{
				raw[raw_i++] = '"';
			}
		
			if (value[val_i] == '\n')
			{
				raw[raw_i++] = '\\';
				raw[raw_i++] = 'n';
			}
			else if (value[val_i] == '"')
			{
				raw[raw_i++] = '\\';
				raw[raw_i++] = '"';
			}
			else
			{
				raw[raw_i++] = value[val_i];
			}
		
			if (with_quotes && val_i == (strlen(value)-1))
			{
				raw[raw_i++] = '"';
			}
		}
	}
	
	raw[len] = 0;
		
	ret = this->SetRawValue(section, name, raw, must_exist);
	delete[] raw;
	
	return ret;
}

bool IniFile::GetBooleanValue(const char *section, const char *name, bool default_true)
{
	char *str = this->GetStringValue(section, name);
	bool ret = default_true;
	
	if (str)
	{
		if (strcasecmp(str, "true") == 0 || str[0] == 1)
			ret = true;
		else if (strcasecmp(str, "false") == 0 || str[0] == 0)
			ret = false;
			
		delete[] str;
	}
	
	return ret;
}

bool IniFile::SetBooleanValue(const char *section, const char *name, bool value, bool must_exist)
{
	char str[6];
	
	strcpy(str, (value) ? "true" : "false");
	return this->SetRawValue(section, name, str, must_exist);
}

bool IniFile::RemoveValue(const char *section, const char *name, bool remove_by_comments)
{
	char *line;
	int rewind;
	
	if (!this->FindSection(section))
		return false;
		
	rewind = (int) (this->ptr_current - this->data);
		
	while ((line = this->ReadLine()))
	{
		char *c_name;
        int line_len = (int)strlen(line);
		
		this->RemoveComments(line);
		
		if (this->IsSection(line))
		{
			delete[] line;
			break;
		}
		
		c_name = this->GetValueName(line);
		if (c_name)
		{
			if (strcasecmp(c_name, name) == 0)
			{
				delete[] c_name;
				delete[] line;
				
				if (remove_by_comments)
				{
					if (!this->Increase(1))
						return false;
										
					this->ptr_current = this->data + rewind;
					memmove(this->ptr_current + 1, this->ptr_current, this->data_size-1-(int)(this->ptr_current-this->data));
					*this->ptr_current = ';';
				}
				else
				{
					this->ptr_current = this->data + rewind;
					memmove(this->ptr_current, this->ptr_current + line_len, this->data_size - line_len - (int)(this->ptr_current-this->data));
					
					if (!this->Resize(this->data_size - line_len))
						return false;
				}
				
				return true;
			}
			
			delete[] c_name;
		}
		
		delete[] line;
		rewind = (int) (this->ptr_current - this->data);
	}
	
	return false;
}

bool IniFile::WriteRawLine(const char *line)
{
	return this->WriteLine(line);
}

bool IniFile::WriteRawLineAt(const char *section, const char *line)
{
	if (!this->FindSection(section))
		return false;
		
	return this->WriteLine(line);
}

bool IniFile::WriteRawLineAt(const char *section, const char *name, const char *line)
{
	char *this_line;
		
	if (!this->FindSection(section))
		return false;
		
	while ((this_line = this->ReadLine()))
	{
		char *c_name;
		
		this->RemoveComments(this_line);
		
		if (this->IsSection(this_line))
		{
			delete[] this_line;
			break;
		}
		
		c_name = this->GetValueName(this_line);
		if (c_name)
		{
			if (strcasecmp(c_name, name) == 0)
			{
				this->ptr_current = this->begin_of_latest_line;
				return this->WriteLine(line);
			}
			
			delete[] c_name;
		}
		
		delete[] this_line;
	}
	
	return false;
}

