#include <algorithm>

#include "UtfFile.h"
#include "debug.h"

UtfFile::UtfFile()
{
	big_endian = true;
	is_empty = true;
}

UtfFile::~UtfFile()
{
	Reset();
}

void UtfFile::Reset()
{
	table_name = "";
	add_null = false;
	unk_00 = 0;
	columns.clear();
	rows.clear();
	is_empty = true;
}

bool UtfFile::Load(const uint8_t *buf, size_t size)
{
	Reset();

	const UTFHeader *phdr = (const UTFHeader *)buf;
	uint8_t *temp = nullptr;

    if (size < sizeof(UTFHeader) || phdr->signature != UTF_SIGNATURE)
	{
        return false;
	}

	uint32_t table_size = val32(phdr->table_size);
	UTFTableHeader *table_hdr = (UTFTableHeader *)GetOffsetPtr(phdr, 8, true);

	if (table_size > (size-sizeof(UTFHeader)) )
	{
		DPRINTF("%s: table_size bigger than buffer.\n", FUNCNAME);
		if (temp)
			delete[] temp;

		return false;
	}

	char *strings = (char *)GetOffsetPtr(table_hdr, table_hdr->strings_offset);
	uint8_t *data = GetOffsetPtr(table_hdr, table_hdr->data_offset);

	uint8_t *col_ptr = GetOffsetPtr(table_hdr, sizeof(UTFTableHeader), true);

	for (uint16_t i = 0; i < val16(table_hdr->num_columns); i++)
	{
		UtfColumn col;

		col.flags = *(col_ptr++);

		if (col.flags == 0)
		{
			DPRINTF("%s: aborting here, this column format must be studied better! (column=%d)\n", FUNCNAME, i);
			if (temp)
				delete[] temp;

			return false;
			col.flags = col_ptr[3];
			col_ptr += 4;
		}

		col.name = (char *)GetOffsetPtr(strings, *(uint32_t *)col_ptr);
        //printf("Col.name = %s  flags: %x\n", col.name.c_str(), col.flags);
		col_ptr += 4;

		if ((col.flags & STORAGE_MASK) == STORAGE_CONSTANT)
		{
			switch (col.flags & TYPE_MASK)
			{
				case TYPE_1BYTE: case TYPE_1BYTE2:
					col.constant_u8 = *col_ptr;
					col_ptr++;
                    //printf("%s: Constant u8 %x\n", col.name.c_str(), col.constant_u8);
				break;

				case TYPE_2BYTE: case TYPE_2BYTE2:
					col.constant_u16 = val16(*(uint16_t *)col_ptr);
					col_ptr += 2;
                    //printf("%s: Constant u16: %x\n", col.name.c_str(), col.constant_u16);
				break;

				case TYPE_4BYTE: case TYPE_4BYTE2:
					col.constant_u32 = val32(*(uint32_t *)col_ptr);
					col_ptr += 4;
                    //printf("%s: Constant u32: %x\n", col.name.c_str(), col.constant_u32);
				break;

				case TYPE_8BYTE: case TYPE_8BYTE2:
					col.constant_u64 = val64(*(uint64_t *)col_ptr);
					col_ptr += 8;
                    //printf("%s: Constant u64: %I64x\n", col.name.c_str(), col.constant_u64);
				break;

				case TYPE_FLOAT:
					copy_float(&col.constant_float, *(float *)col_ptr);
					col_ptr += 4;
                    //DPRINTF("Constant float: %f\n", col.constant_float);
				break;

				case TYPE_STRING:
					col.constant_str = (char *)GetOffsetPtr(strings, *(uint32_t *)col_ptr);
					col_ptr += 4;
                    //DPRINTF("Constant string: %s\n", col.constant_str.c_str());
				break;

				case TYPE_DATA:
				{
					uint8_t *binary_data = GetOffsetPtr(data, *(uint32_t *)col_ptr);
					col.constant_data_size = val32(*(uint32_t *)(col_ptr + 4));

					if (col.constant_data_size != 0)
					{
						col.constant_data = new uint8_t[col.constant_data_size];
						if (!col.constant_data)
						{
							DPRINTF("%s: Memory allocation error (0x%x)\n", FUNCNAME, col.constant_data_size);
							if (temp)
								delete[] temp;

							return false;
						}

						memcpy(col.constant_data, binary_data, col.constant_data_size);
					}

					col_ptr += 8;
                    //uint32_t position = Utils::DifPointer(binary_data, table_hdr);
                    //printf("Binary (position: %x, size: %x)\n", position, col.constant_data_size);

				}
				break;

				default:
					DPRINTF("%s: not implemented for constant, flags = 0x%x\n", FUNCNAME, col.flags);
					return false;
			}
		}

		columns.push_back(col);
	}

	for (uint32_t j = 0; j < val32(table_hdr->num_rows); j++)
	{
		UtfRow current_row;
		uint8_t *row_ptr = GetOffsetPtr(table_hdr, val16(table_hdr->rows_offset) + j*val16(table_hdr->row_length), true);

		//DPRINTF("ptr: %x\n", Utils::DifPointer(row_ptr, table_hdr));

		for (uint16_t i = 0; i < val16(table_hdr->num_columns); i++)
		{
			UtfValue current_value;

			uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);

			if (storage_flag == STORAGE_NONE) // 0x00
			{
				current_row.values.push_back(current_value);
				continue;
			}

			if (storage_flag == STORAGE_ZERO) // 0x10
			{
				current_row.values.push_back(current_value);
				continue;
			}

			if (storage_flag == STORAGE_CONSTANT) // 0x30
			{
				current_row.values.push_back(current_value);
				continue;
			}

			current_value.type = columns[i].flags & TYPE_MASK;
            //printf("Type: %x\n", current_value.type);

			switch (current_value.type)
			{
				case TYPE_1BYTE:
				case TYPE_1BYTE2:
					current_value._u8 = *row_ptr;
					row_ptr++;
                    //printf("U8: %s %x\n", columns[i].name.c_str(), current_value._u8);
				break;

				case TYPE_2BYTE:
				case TYPE_2BYTE2:
					current_value._u16 = val16(*(uint16_t *)row_ptr);
					row_ptr += 2;
                    //printf("U16: %s %x\n", columns[i].name.c_str(), current_value._u16);
				break;

				case TYPE_4BYTE:
				case TYPE_4BYTE2:
					current_value._u32 = val32(*(uint32_t *)row_ptr);
                    row_ptr += 4;
                    //printf("U32: %s %x\n", columns[i].name.c_str(), current_value._u32);
				break;

				case TYPE_8BYTE:
				case TYPE_8BYTE2:
					current_value._u64 = val64(*(uint64_t *)row_ptr);
					row_ptr += 8;
                    //printf("U64: %s %I64x\n", columns[i].name.c_str(), current_value._u64);
				break;

				case TYPE_FLOAT:
					copy_float(&current_value._float, *(float *)row_ptr);
					row_ptr += 4;
                    //printf("float: %s %f\n", columns[i].name.c_str(), current_value._float);
				break;

				case TYPE_STRING:
					current_value.str = (char *)GetOffsetPtr(strings, *(uint32_t *)row_ptr);
					row_ptr += 4;
                    //DPRINTF("%s -> %s\n", columns[i].name.c_str(),  current_value.str.c_str());
				break;

				case TYPE_DATA:
				{
					uint8_t *binary_data = GetOffsetPtr(data, *(uint32_t *)row_ptr);
					current_value.data_size = val32(*(uint32_t *)(row_ptr + 4));

					if (current_value.data_size != 0)
					{
                        current_value.data = new uint8_t[current_value.data_size];
						memcpy(current_value.data, binary_data, current_value.data_size);
					}

					row_ptr += 8;
                    //uint32_t position = Utils::DifPointer(binary_data, table_hdr);
                    //printf("%s: Binary (position: %x, size: %x)\n", columns[i].name.c_str(), position, current_value.data_size);
				}
				break;

				default:
					DPRINTF("%s: Not implemented data type: 0x%x\n", FUNCNAME, current_value.type);
					if (temp)
						delete[] temp;

					return false;
			}

			current_row.values.push_back(current_value);
		}

		rows.push_back(current_row);
	}

    if (strings && strcmp(strings, "<NULL>") == 0)
    {
        add_null = true;
    }

	table_name = (char *)GetOffsetPtr(strings, table_hdr->table_name);
	unk_00 = val16(table_hdr->unk_00);

	if (temp)
		delete[] temp;

	is_empty = false;
	return true;
}

size_t UtfFile::CalculateColumnsSize() const
{
    size_t columns_size = columns.size() * (sizeof(uint32_t) + sizeof(uint8_t));

    for (const UtfColumn &col :columns)
    {
        uint8_t storage_flag = (col.flags & STORAGE_MASK);
        uint8_t ctype = (col.flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT)
        {
            switch (ctype)
            {
                case TYPE_1BYTE: case TYPE_1BYTE2:
                    columns_size++;
                break;

                case TYPE_2BYTE: case TYPE_2BYTE2:
                    columns_size += 2;
                break;

                case TYPE_4BYTE: case TYPE_4BYTE2: case TYPE_FLOAT:
                    columns_size += 4;
                break;

                case TYPE_8BYTE: case TYPE_8BYTE2:
                    columns_size += 8;
                break;

                case TYPE_STRING:
                    columns_size += 4;
                break;

                case TYPE_DATA:
                    columns_size += 8;
                break;
            }
        }
    }

    return columns_size;
}

uint16_t UtfFile::CalculateRowLength() const
{
	uint16_t row_length = 0;

	for (const UtfRow &row : rows)
	{
		uint16_t this_row_length = 0;

        //DPRINTF("row_length = %x\n", row_length);

		for (const UtfValue &data : row.values)
		{
			switch (data.type)
			{
				case TYPE_1BYTE: case TYPE_1BYTE2:
					this_row_length += 1;
				break;

				case TYPE_2BYTE: case TYPE_2BYTE2:
					this_row_length += 2;
				break;

				case TYPE_4BYTE: case TYPE_4BYTE2: case TYPE_FLOAT: case TYPE_STRING:
					this_row_length += 4;
				break;

				case TYPE_8BYTE: case TYPE_8BYTE2: case TYPE_DATA:
					this_row_length += 8;
				break;
			}
		}

		if (this_row_length > row_length)
			row_length = this_row_length;
	}

    return row_length;
}

size_t UtfFile::CalculateStringsSize() const
{
    std::vector<std::string> strings_list;
    size_t strings_size = table_name.length() + 1;

    strings_list.push_back(table_name);

    if (add_null)
    {
        strings_list.push_back("<NULL>");
        strings_size += 7;
    }

    assert(rows.size() > 0);

	for (const UtfColumn &col : columns)
	{
        uint8_t storage_flag = (col.flags & STORAGE_MASK);
        uint8_t ctype = (col.flags & TYPE_MASK);

        strings_size += col.name.length() + 1;        

        if (storage_flag == STORAGE_CONSTANT && ctype == TYPE_STRING)
        {
            if (std::find(strings_list.begin(), strings_list.end(), col.constant_str) == strings_list.end())
            {
                strings_list.push_back(col.constant_str);
                strings_size += col.constant_str.length() + 1;
            }
        }
	}    

	for (const UtfRow &row : rows)
	{
        for (const UtfValue &data: row.values)
		{
			if (data.type == TYPE_STRING)
            {
                if (std::find(strings_list.begin(), strings_list.end(), data.str) == strings_list.end())
                {
                    strings_list.push_back(data.str);
                    strings_size += data.str.length() + 1;
                }
			}
		}
	}

    //DPRINTF("sStrings size = %x\n", strings_size);
	return strings_size;
}

size_t UtfFile::CalculateFileSize() const
{
    size_t file_size = sizeof(UTFHeader) + sizeof(UTFTableHeader);

    file_size += CalculateColumnsSize();

	/*if (file_size & 0x1F)
		file_size += (0x20 - (file_size & 0x1F));*/

    file_size += CalculateRowLength()*rows.size();
    file_size += CalculateStringsSize();

	if (file_size & 0x1F)
		file_size += (0x20 - (file_size & 0x1F));

	// Calculate size of binary data
	for (const UtfRow &row : rows)
	{
		for (const UtfValue &data: row.values)
		{
			if (data.type == TYPE_DATA && data.data != nullptr)
			{
				file_size += data.data_size;
				file_size += (0x20 - (file_size & 0x1F)); // DONT'T ADD check for alignment already OK, official tool doesn't, and instead adds 0x20 bytes in that case
			}
		}
	}

	//DPRINTF("file size :%x\n", file_size);
    return file_size;
}

void UtfFile::Debug() const
{
    DPRINTF("Num columns = %Id, num rows = %Id\n", columns.size(), rows.size());

    for (size_t i = 0; i < columns.size(); i++)
    {
        const UtfColumn &column = columns[i];

        printf("Column (%d): %s   (flags 0x%x)\n", (int)i, column.name.c_str(), column.flags);
    }

    uint16_t w;
    uint32_t dw;
    std::string s;
    uint8_t b;

    for (unsigned int i = 0; i < (unsigned int)rows.size(); i++)
    {
        if (GetWord("EventIndex", &w, i))
        {
            printf("EventIndex (%d): %d\n", i, w);
        }

        if (GetDword("CueId", &dw, i))
        {
            printf("CueId (%d): %d\n", i, dw);
        }

        if (GetWord("ReferenceIndex", &w, i))
        {
            printf("ReferenceIndex (%d): %d\n", i, w);
        }

        if (GetByte("ReferenceType", &b, i))
        {
            printf("ReferenceType (%d): %d\n", i, b);
        }

        /*if (GetDword("Length", &dw, i))
        {
            printf("Length (%d): %d\n", i, dw);
        }*/

        if (GetString("CueName", &s, i))
        {
            printf("CueName (%d): %s\n", i, s.c_str());
        }

        if (GetWord("CueIndex", &w, i))
        {
            printf("CueIndex (%d): %d\n", i, w);
        }

        if (GetWord("MemoryAwbId", &w, i))
        {
            printf("MemoryAwbId (%d): %d\n", i, w);
        }

        if (GetByte("Streaming", &b, i))
        {
            printf("Streaming (%d): %d\n", i, b);
        }

        if (GetWord("StreamAwbPortNo", &w, i))
        {
            printf("StreamAwbPortNo (%d): %d\n", i, w);
        }

        if (GetWord("StreamAwbId", &w, i))
        {
            printf("StreamAwbId (%d): %d\n", i, w);
        }

        /*if (GetWord("CommandIndex", &w, i))
        {
            printf("CommandIndex (%d): %d\n", i, w);
        }*/

        if (GetFixedBlob("ReferenceItems", (uint8_t *)&dw, 4, i))
        {
            printf("ReferenceItems (%d): 0x%x, low: %d\n", i, val32(dw), val32(dw)&0xFFFF);
        }

        /*if (GetWord("ControlWorkArea1", &w, i))
        {
            printf("ControlWorkArea1 (%d): %d\n", i, w);
        }*/

        if (GetFixedBlob("TrackIndex", (uint8_t *)&w, 2, i))
        {
            printf("TrackIndex (%d): %d\n", i, val16(w));
        }
        uint8_t buf[10];

        if (GetFixedBlob("Command", buf, 10, i))
        {
            printf("Command (%d): %d %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", i, *(uint32_t *)&buf[6], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
        }
        else
        {
            unsigned int size;
            uint8_t *ptr = GetBlob("Command", &size, false, i);

            if (ptr)
            {
                 printf("Command ");

                for (unsigned int i = 0; i < size; i++)
                {
                    printf("%02X ", ptr[i]);
                }

                printf("\n");
            }
        }

        unsigned int indexes_size;
        uint8_t *indexes = GetBlob("TrackIndex", &indexes_size, false, i);
        if (indexes && indexes_size != 2)
        {
            printf("TrackIndex (%d) ", i);

            for (unsigned int i = 0; i < indexes_size; i++)
                printf("%02X ", indexes[i]);

            printf("\n");
        }

        unsigned int values_size;
        uint8_t *values = GetBlob("TrackValues", &values_size, false, i);
        if (values)
        {
            printf("TrackValues (%d) ", i);

            for (unsigned int i = 0; i < values_size; i++)
                printf("%02X ", values[i]);

            printf("\n");
        }

        /*if (GetByte("Type", &b, i))
        {
            printf("Type (%d): %d\n", i, b);
        }*/

        if (GetString("Name", &s, i))
        {
            printf("Name (%d): %s\n", i, s.c_str());
        }

        if (GetDword("Id", &dw, i))
        {
            printf("Id (%d): %d\n", i, dw);
        }

        if (GetWord("NumTracks", &w, i))
        {
            printf("NumTracks (%d): %d\n", i, w);
        }
    }
}

void UtfFile::DebugDump() const
{
    DPRINTF("Num columns = %Id, num rows = %Id\n", columns.size(), rows.size());

    for (size_t i = 0; i < columns.size(); i++)
    {
        const UtfColumn &column = columns[i];
        uint8_t ctype = (column.flags & TYPE_MASK);
        uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);

        DPRINTF("\n***Column %d: %s   (flags 0x%x)\n", (int)i, column.name.c_str(), column.flags);

        for (unsigned int j = 0; j < (unsigned int)rows.size(); j++)
        {
            if (storage_flag == STORAGE_CONSTANT)
            {
                DPRINTF("Row (constant): ");
            }
            else
            {
                DPRINTF("Row %d: ", j);
            }

            if (ctype == TYPE_1BYTE || ctype == TYPE_1BYTE2)
            {
                uint8_t byte;

                if (GetByte(column.name, &byte, j))
                {
                    DPRINTF("0x%02X (%d)\n", byte, byte);
                }
            }
            else if (ctype == TYPE_2BYTE || ctype == TYPE_2BYTE2)
            {
                uint16_t word;

                if (GetWord(column.name, &word, j))
                {
                    DPRINTF("0x%04X (%d)\n", word, word);
                }
            }
            else if (ctype == TYPE_4BYTE || ctype == TYPE_4BYTE2)
            {
                uint32_t dword;

                if (GetDword(column.name, &dword, j))
                {
                    DPRINTF("0x%08X (%d)\n", dword, dword);
                }
            }
            else if (ctype == TYPE_8BYTE || ctype == TYPE_8BYTE2)
            {
                uint64_t qword;

                if (GetQword(column.name, &qword, j))
                {
                    DPRINTF("0x%16I64X (%I64d)\n", qword, qword);
                }
            }
            else if (ctype == TYPE_FLOAT)
            {
                float f;

                if (GetFloat(column.name, &f, j))
                {
                    DPRINTF("%f\n", f);
                }
            }
            else if (ctype == TYPE_STRING)
            {
                std::string str;

                if (GetString(column.name, &str, j))
                {
                    DPRINTF("\"%s\"\n", str.c_str());
                }
            }
            else if (ctype == TYPE_DATA)
            {
                unsigned int size;
                uint8_t *data = GetBlob(column.name, &size, false, j);

                if (data)
                {
                    DPRINTF("\n");

                    for (unsigned int i = 0; i < size; i++)
                    {
                        DPRINTF("%02X ", data[i]);

                        if ((i&0xF) == 0xF)
                            DPRINTF("\n");
                    }

                    if (size & 0xF)
                        DPRINTF("\n");
                }
                else
                {
                    DPRINTF("Empty\n");
                }
            }

            if (storage_flag == STORAGE_CONSTANT)
                break;
        }

        DPRINTF("\n");
    }
}

char *UtfFile::FindString(char *strings_list, const char *str, char *end)
{
    char *ptr = strings_list;

    while (ptr < end)
    {
        if (strcmp(ptr, str) == 0)
            return ptr;

        ptr += strlen(ptr)+1;
    }

    return nullptr;
}

uint8_t *UtfFile::Save(size_t *psize)
{
    size_t file_size;
    uint32_t offset, str_offset, data_offset_start;
	uint8_t *buf;
    uint8_t	*ptr, *col_ptr, *scol_ptr, *rows_ptr, *data_ptr;
	char *str_ptr;

    file_size = CalculateFileSize();
    assert(file_size <= 0xFFFFFFFF);

    buf = new uint8_t[file_size];
	memset(buf, 0, file_size);

	UTFHeader *hdr = (UTFHeader *)buf;
	hdr->signature = UTF_SIGNATURE;
    hdr->table_size = val32((uint32_t)file_size - sizeof(UTFHeader));

	UTFTableHeader *table_hdr = (UTFTableHeader *)GetOffsetPtr(hdr, sizeof(UTFHeader), true);
	table_hdr->unk_00 = val16(unk_00);
    table_hdr->num_columns = val16((uint16_t)columns.size());
	table_hdr->row_length = val16(CalculateRowLength());
    table_hdr->num_rows = val32((uint32_t)rows.size());

	offset = sizeof(UTFHeader) + sizeof(UTFTableHeader);
	ptr = buf + offset;
    col_ptr = scol_ptr = ptr;

    offset += (uint32_t)CalculateColumnsSize();
	/*if (offset & 0x1F)
		offset += (0x20 - (offset & 0x1F));*/

    assert(offset < 0x10000);
    table_hdr->rows_offset = val16((uint16_t)offset - sizeof(UTFHeader));
	rows_ptr = buf + offset;

    for (const UtfRow &row : rows)
	{
		for (const UtfValue &data : row.values)
		{
			ptr = buf + offset;

			switch (data.type)
			{
				case TYPE_1BYTE: case TYPE_1BYTE2:
					*ptr = data._u8;
					offset++;
				break;

				case TYPE_2BYTE: case TYPE_2BYTE2:
					*(uint16_t *)ptr = val16(data._u16);
					offset += 2;
				break;

				case TYPE_4BYTE: case TYPE_4BYTE2:
					*(uint32_t *)ptr = val32(data._u32);
					offset += 4;
				break;

				case TYPE_8BYTE: case TYPE_8BYTE2:
					*(uint64_t *)ptr = val64(data._u64);
					offset += 8;
				break;

				case TYPE_FLOAT:
					copy_float(ptr, data._float);
					offset += 4;
				break;

				case TYPE_STRING:
					offset += 4;
				break;

				case TYPE_DATA:
					offset += 8;
				break;
			}
		}
    }

    table_hdr->strings_offset = val32(offset - sizeof(UTFHeader));
	str_ptr = (char *)GetOffsetPtr(buf, offset, true);

    char *str_top = str_ptr;

	if (!add_null)
    {
        strcpy(str_ptr, table_name.c_str());
        table_hdr->table_name = 0;

        str_offset = (uint32_t)table_name.length() + 1;
    }
	else
    {
        strcpy(str_ptr, "<NULL>");
        strcpy(str_ptr+7, table_name.c_str());
        table_hdr->table_name = val32(7);

        str_offset = (uint32_t)table_name.length() + 8;
    }

    // TODO: rearrange how constant strings are written, so that they are first strings in strings section,to match official tools

    for (const UtfColumn &col : columns)
	{
        uint8_t storage_flag = (col.flags & STORAGE_MASK);
        uint8_t ctype = (col.flags & TYPE_MASK);
        uint32_t cstring_offset = 0;

        if (storage_flag == STORAGE_CONSTANT && ctype == TYPE_STRING)
        {
            str_ptr =  (char *)GetOffsetPtr(buf, offset+str_offset, true);
            char *existing_string = FindString(str_top, col.constant_str.c_str(), str_ptr);

            if (existing_string)
            {
                cstring_offset = Utils::DifPointer(existing_string, str_top);
            }
            else
            {
                strcpy(str_ptr, col.constant_str.c_str());
                cstring_offset = str_offset;
                str_offset += (uint32_t)col.constant_str.length() + 1;
            }
        }

        *col_ptr = col.flags;
        col_ptr++;

        // for column names, we don't check existing strings!!
        str_ptr =  (char *)GetOffsetPtr(buf, offset+str_offset, true);
        strcpy(str_ptr, col.name.c_str());
        *(uint32_t *)col_ptr = val32(str_offset);
        str_offset += (uint32_t)col.name.length() + 1;

        col_ptr += sizeof(uint32_t);

        if (storage_flag == STORAGE_CONSTANT)
        {
            switch (ctype)
            {
                case TYPE_1BYTE: case TYPE_1BYTE2:
                    *col_ptr = col.constant_u8;
                    col_ptr++;
                break;

                case TYPE_2BYTE: case TYPE_2BYTE2:
                    *(uint16_t *)col_ptr = val16(col.constant_u16);
                    col_ptr += 2;
                break;

                case TYPE_4BYTE: case TYPE_4BYTE2:
                    *(uint32_t *)col_ptr = val32(col.constant_u32);
                    col_ptr += 4;
                break;

                case TYPE_8BYTE: case TYPE_8BYTE2:
                    *(uint64_t *)col_ptr = val64(col.constant_u64);
                    col_ptr += 8;
                break;

                case TYPE_FLOAT:
                    copy_float(col_ptr, col.constant_float);
                    col_ptr += 4;
                break;

                case TYPE_STRING:
                    *(uint32_t *)col_ptr = val32(cstring_offset);
                    col_ptr += 4;
                break;

                case TYPE_DATA:
                    *(uint32_t *)col_ptr = 0;
                    *(uint32_t *)(col_ptr+4) = val32(col.constant_data_size);
                    col_ptr += 8;
                break;
            }
        }
	}

	ptr = rows_ptr;

	for (const UtfRow &row : rows)
	{
		for (const UtfValue &data: row.values)
		{
			switch (data.type)
			{
				case TYPE_1BYTE: case TYPE_1BYTE2:
					ptr++;
				break;

				case TYPE_2BYTE: case TYPE_2BYTE2:
					ptr += 2;
				break;

				case TYPE_4BYTE: case TYPE_4BYTE2: case TYPE_FLOAT:
					ptr += 4;
				break;

				case TYPE_8BYTE: case TYPE_8BYTE2: case TYPE_DATA:
					ptr += 8;
				break;

				case TYPE_STRING:

                    str_ptr =  (char *)GetOffsetPtr(buf, offset+str_offset, true);
                    char *existing_string = FindString(str_top, data.str.c_str(), str_ptr);

                    if (existing_string)
                    {
                        *(uint32_t *)ptr = val32(Utils::DifPointer(existing_string, str_top));
                    }
                    else
                    {
                        *(uint32_t *)ptr = val32(str_offset);
                        strcpy(str_ptr, data.str.c_str());
                        str_offset += (uint32_t)data.str.length() + 1;
                    }

                    ptr += 4;

				break;
			}
		}
	}

    //DPRINTF("Offset = %x, str_offset = %x\n", offset, str_offset);

	offset += str_offset;

	if (offset & 0x1F)
		offset += (0x20 - (offset & 0x1F));

	table_hdr->data_offset = val32(offset - sizeof(UTFHeader));
	data_offset_start = offset;
	ptr = rows_ptr;

    for (const UtfColumn &col : columns)
    {
        uint8_t storage_flag = (col.flags & STORAGE_MASK);
        uint8_t ctype = (col.flags & TYPE_MASK);

        scol_ptr += sizeof(uint32_t) + sizeof(uint8_t);

        if (storage_flag == STORAGE_CONSTANT)
        {
            switch (ctype)
            {
                case TYPE_1BYTE: case TYPE_1BYTE2:
                    scol_ptr++;
                break;

                case TYPE_2BYTE: case TYPE_2BYTE2:
                    scol_ptr += 2;
                break;

                case TYPE_4BYTE: case TYPE_4BYTE2:
                    scol_ptr += 4;
                break;

                case TYPE_8BYTE: case TYPE_8BYTE2:
                    scol_ptr += 8;
                break;

                case TYPE_FLOAT:
                    scol_ptr += 4;
                break;

                case TYPE_STRING:
                    scol_ptr += 4;
                break;

                case TYPE_DATA:
                    if (col.constant_data_size != 0)
                    {
                        DPRINTF("%s: FIXME, implement constant binary data here and also in CalculateFileSize.\n", FUNCNAME);
                        delete[] buf;
                        return nullptr;
                    }

                    *(uint32_t *)scol_ptr = val32(offset - data_offset_start);
                    scol_ptr += 8;
                break;
            }
        }
    }

	for (const UtfRow &row : rows)
	{
		for (const UtfValue &data: row.values)
		{
			switch (data.type)
			{
				case TYPE_1BYTE: case TYPE_1BYTE2:
					ptr++;
				break;

				case TYPE_2BYTE: case TYPE_2BYTE2:
					ptr += 2;
				break;

				case TYPE_4BYTE: case TYPE_4BYTE2: case TYPE_FLOAT: case TYPE_STRING:
					ptr += 4;
				break;

				case TYPE_8BYTE: case TYPE_8BYTE2:
					ptr += 8;
				break;

				case TYPE_DATA:

					if (data.data && data.data_size != 0)
					{
						*(uint32_t *)ptr = val32(offset - data_offset_start);
						*(uint32_t *)(ptr+4) = val32(data.data_size);

						data_ptr = buf + offset;
						memcpy(data_ptr, data.data, data.data_size);

						offset += data.data_size;
						offset += (0x20 - (offset & 0x1F));
					}
					else
					{
						*(uint32_t *)ptr = 0;
						*(uint32_t *)(ptr+4) = 0;
					}

					ptr += 8;
				break;
			}
		}
	}

    //DPRINTF("offset = %x, file_size = %I64x\n", offset, file_size);
    assert(offset == file_size);

	*psize = file_size;
	return buf;
}

bool UtfFile::ColumnExists(const std::string &name) const
{
	for (auto &col : columns)
	{
		if (col.name == name)
			return true;
	}

    return false;
}

bool UtfFile::IsVariableColumn(unsigned int column) const
{
    if (column >= columns.size())
        return false;

    uint8_t storage_flag = (columns[column].flags & STORAGE_MASK);

    if (storage_flag == STORAGE_CONSTANT || storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
        return false;

    return true;
}

unsigned int UtfFile::ColumnIndex(const std::string &name) const
{
    for (size_t i = 0; i < columns.size(); i++)
    {
        if (columns[i].name == name)
            return (unsigned int)i;
    }

    return (unsigned int)-1;
}

bool UtfFile::GetByte(const std::string &name, uint8_t *byte, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_1BYTE && ctype != TYPE_1BYTE2)
					return false;

				*byte = columns[i].constant_u8;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_1BYTE && data.type != TYPE_1BYTE2)
				return false;

			*byte = data._u8;
			return true;
		}
	}

	return false;
}

bool UtfFile::GetWord(const std::string &name, uint16_t *word, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_2BYTE && ctype != TYPE_2BYTE2)
					return false;

				*word = columns[i].constant_u16;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_2BYTE && data.type != TYPE_2BYTE2)
				return false;

			*word = data._u16;
			return true;
		}
	}

	return false;
}

bool UtfFile::GetDword(const std::string &name, uint32_t *dword, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_4BYTE && ctype != TYPE_4BYTE2)
					return false;

				*dword = columns[i].constant_u32;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_4BYTE && data.type != TYPE_4BYTE2)
				return false;

			*dword = data._u32;
			return true;
		}
	}

	return false;
}

bool UtfFile::GetQword(const std::string &name, uint64_t *qword, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_8BYTE && ctype != TYPE_8BYTE2)
					return false;

				*qword = columns[i].constant_u64;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_8BYTE && data.type != TYPE_8BYTE2)
				return false;

			*qword = data._u64;
			return true;
		}
	}

	return false;
}

bool UtfFile::GetFloat(const std::string &name, float *f, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_FLOAT)
					return false;

				*f = columns[i].constant_float;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_FLOAT)
				return false;

			*f = data._float;
			return true;
		}
	}

	return false;
}

bool UtfFile::GetString(const std::string &name, std::string *str, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_STRING)
					return false;

				*str = columns[i].constant_str;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_STRING)
				return false;

			*str = data.str;            
			return true;
		}
	}

	return false;
}

uint8_t *UtfFile::GetBlob(const std::string &name, unsigned int *size, bool alloc_new, unsigned int row) const
{
	if (row >= rows.size())
		return nullptr;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_DATA || columns[i].constant_data_size == 0)
					return nullptr;

				*size = columns[i].constant_data_size;

				if (alloc_new)
				{
                    uint8_t *ret = new uint8_t[*size];

					memcpy(ret, columns[i].constant_data, *size);
					return ret;
				}

				return columns[i].constant_data;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_DATA || data.data_size == 0)
				return nullptr;

			*size = data.data_size;

			if (alloc_new)
			{
                uint8_t *ret = new uint8_t[*size];

				memcpy(ret, data.data, *size);
				return ret;
			}

			return data.data;
		}
	}

	return nullptr;
}

bool UtfFile::GetFixedBlob(const std::string &name, uint8_t *buf, unsigned int size, unsigned int row) const
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

		if (storage_flag == STORAGE_CONSTANT)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_DATA || columns[i].constant_data_size != size)
					return false;

				memcpy(buf, columns[i].constant_data, size);
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			const UtfValue &data = rows[row].values[i];

            //DPRINTF("Fixed blob:%s  %x %x\n", name.c_str(), size, data.data_size);

			if (data.type != TYPE_DATA || data.data_size != size)
				return false;

			memcpy(buf, data.data, size);
			return true;
		}
	}

	return false;
}

bool UtfFile::SetByte(const std::string &name, uint8_t byte, unsigned int row, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_1BYTE && ctype != TYPE_1BYTE2)
					return false;

				columns[i].constant_u8 = byte;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_1BYTE && data.type != TYPE_1BYTE2)
				return false;

			data._u8 = byte;
			return true;
		}
	}

	return false;
}

bool UtfFile::SetWord(const std::string &name, uint16_t word, unsigned int row, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_2BYTE && ctype != TYPE_2BYTE2)
					return false;

				columns[i].constant_u16 = word;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_2BYTE && data.type != TYPE_2BYTE2)
				return false;

			data._u16 = word;
			return true;
		}
	}

	return false;
}

bool UtfFile::SetDword(const std::string &name, uint32_t dword, unsigned int row, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_4BYTE && ctype != TYPE_4BYTE2)
					return false;

				columns[i].constant_u32 = dword;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_4BYTE && data.type != TYPE_4BYTE2)
				return false;

			data._u32 = dword;
			return true;
		}
	}

	return false;
}

bool UtfFile::SetQword(const std::string &name, uint64_t qword, unsigned int row, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_8BYTE && ctype != TYPE_8BYTE2)
					return false;

				columns[i].constant_u64 = qword;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_8BYTE && data.type != TYPE_8BYTE2)
				return false;

			data._u64 = qword;
			return true;
		}
	}

	return false;
}

bool UtfFile::SetFloat(const std::string &name, float f, unsigned int row, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_FLOAT)
					return false;

				columns[i].constant_float = f;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_FLOAT)
				return false;

			data._float = f;
			return true;
		}
	}

	return false;
}

bool UtfFile::SetString(const std::string &name, const std::string &str, unsigned int row, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_STRING)
					return false;

				columns[i].constant_str = str;
				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_STRING)
				return false;

			data.str = str;
			return true;
		}
	}

	return false;
}

bool UtfFile::SetBlob(const std::string &name, uint8_t *blob, unsigned int blob_size, unsigned int row, bool take_ownership, bool change_constant)
{
	if (row >= rows.size())
		return false;

	for (size_t i = 0; i < columns.size(); i++)
	{
		uint8_t storage_flag = (columns[i].flags & STORAGE_MASK);
		uint8_t ctype = (columns[i].flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT && change_constant)
		{
			if (columns[i].name == name)
			{
				if (ctype != TYPE_DATA)
					return false;

				if (columns[i].constant_data && blob_size <= columns[i].constant_data_size)
				{
                    if (take_ownership)
                    {
                        delete[] columns[i].constant_data;
                        columns[i].constant_data = blob;
                    }
                    else
                    {
                        memmove(columns[i].constant_data, blob, blob_size);
                    }

                    columns[i].constant_data_size = blob_size;
				}
				else
				{
					if (columns[i].constant_data)
						delete[] columns[i].constant_data;

                    if (take_ownership)
                    {
                        columns[i].constant_data = blob;
                    }
                    else
                    {
                        columns[i].constant_data = new uint8_t[blob_size];
                        memcpy(columns[i].constant_data, blob, blob_size);
                    }

					columns[i].constant_data_size = blob_size;
				}

				return true;
			}

			continue;
		}

		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO)
		{
			continue;
		}

		if (columns[i].name == name)
		{
			UtfValue &data = rows[row].values[i];

			if (data.type != TYPE_DATA)
				return false;

			if (data.data && blob_size <= data.data_size)
			{
                if (take_ownership)
                {
                    delete[] data.data;
                    data.data = blob;
                }
                else
                {
                    memmove(data.data, blob, blob_size);
                }

				data.data_size = blob_size;
			}
			else
			{
				if (data.data)
					delete[] data.data;

                if (take_ownership)
                {
                    data.data = blob;
                }
                else
                {
                    data.data = new uint8_t[blob_size];
                    memcpy(data.data, blob, blob_size);
                }

                data.data_size = blob_size;
			}

			return true;
		}
	}

    return false;
}

unsigned int UtfFile::GetHighestRow16(const std::string &name, uint16_t *val) const
{
    uint16_t max = 0;
    uint16_t max_row = 0;

    for (uint32_t i = 0; i < GetNumRows(); i++)
    {
        uint16_t w;

        if (!GetWord(name, &w, i))
            return (uint32_t)-1;

        if (w != 0xFFFF && w > max)
        {
            max = w;
            max_row = i;
        }
    }

    *val = max;
    return max_row;
}

unsigned int UtfFile::GetHighestRow32(const std::string &name, uint32_t *val) const
{
    uint32_t max = 0;
    uint32_t max_row = 0;

    for (uint32_t i = 0; i < GetNumRows(); i++)
    {
        uint32_t dw;

        if (!GetDword(name, &dw, i))
            return (uint32_t)-1;

        if (dw != 0xFFFFFFFF && dw > max)
        {
            max = dw;
            max_row = i;
        }
    }

    *val = max;
    return max_row;
}

UtfRow *UtfFile::CreateRow()
{
    if (columns.size() == 0)
        return nullptr;

    UtfRow row;

    row.values.resize(columns.size());

    for (size_t i = 0; i < columns.size(); i++)
    {
        const UtfColumn &column = columns[i];
        UtfValue &value = row.values[i];

        uint8_t storage_flag = (column.flags & STORAGE_MASK);
        uint8_t ctype = (column.flags & TYPE_MASK);

        if (storage_flag == STORAGE_CONSTANT || storage_flag == STORAGE_ZERO || storage_flag == STORAGE_NONE)
            continue;

        value.type = ctype;
    }

    rows.push_back(row);
    return &rows[rows.size()-1];
}

