/*
 * Code here heavily based on CriPakTools C# code
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CpkFileOld.h"
#include "common.h"

#include "debug.h"


void *Row::GetValue()
{
	switch (type)
	{
		case 0: case 1:
			return (void *)u1._u8;
		break;
		
		case 2: case 3:
			return (void *)u1._u16;
		break;
		
		case 4: case 5:
			return (void *)u1._u32;
		break;
		
		case 6: case 7:
			return (void *)u1._u64;
		break;
		
		case 8:
			return (void *)u1._float;
		break;
		
		case 0xA:
			return (void *)u1.str;
		break;
		
		case 0xB:
			return (void *)u1.data;
		break;
	}
	
	return NULL;
}

CpkUTF::CpkUTF()
{
	big_endian = true;
	is_encrypted = false;
	encrypt_status = false;
}

CpkUTF::~CpkUTF()
{
	
}

void CpkUTF::ToggleEncryption()
{
	if (!is_encrypted)
		return;
	
	big_endian = false;
	uint64_t size = val64(*(uint64_t *)(top - 8));
	big_endian = true;
	
	int32_t m, t;
	uint8_t d;

	m = 0x0000655f;
	t = 0x00004115;

	for (uint64_t i = 0; i < size; i++)
	{
		d = top[i];
		d = (uint8_t)(d ^ (uint8_t)(m & 0xff));
		top[i] = d;
		m *= t;
	}
	
	encrypt_status = !encrypt_status;
}

void CpkUTF::RevertEncryption()
{
	if (!is_encrypted)
		return;
	
	if (encrypt_status)
		return;
	
	ToggleEncryption();
}

bool CpkUTF::Load(uint8_t *buf)
{
	UTFHeader *phdr = (UTFHeader *)buf;
	
	top = buf;	
	if (phdr->signature != UTF_SIGNATURE)
	{	
		is_encrypted = true;
		encrypt_status = true;
		ToggleEncryption();
		
		if (phdr->signature != UTF_SIGNATURE)
		{
			DPRINTF("Decryption failed.\n");
			return false;
		}
	}	
	
	table_size = val32(phdr->table_size);
	table_hdr = (UTFTableHeader *)GetOffsetPtr(phdr, 8, true);
	
	strings = (char *)GetOffsetPtr(table_hdr, table_hdr->strings_offset);
	data = GetOffsetPtr(table_hdr, table_hdr->data_offset);
	
	uint8_t *col_ptr = GetOffsetPtr(table_hdr, sizeof(UTFTableHeader), true);
		
	for (uint16_t i = 0; i < val16(table_hdr->num_columns); i++)
	{
		Column col;
		
		col.flags = *(col_ptr++);
		
		if (col.flags == 0)
		{
			col.flags = col_ptr[3];
			col_ptr += 4;
		}
		
		col.name = (char *)GetOffsetPtr(strings, *(uint32_t *)col_ptr);
		//printf("Col.name = %s\n", col.name);
		col_ptr += 4;
		columns.push_back(col);		
	}
	
	Rows current_entry;
	Row current_row;
	uint32_t storage_flag;
	
	for (uint32_t j = 0; j < val32(table_hdr->num_rows); j++)
	{
		uint8_t *row_ptr = GetOffsetPtr(table_hdr, val32(table_hdr->rows_offset) + j*val16(table_hdr->row_length), true);

		//printf("%x\n", DifPointer(row_ptr, table_hdr));
		
		for (uint16_t i = 0; i < val16(table_hdr->num_columns); i++)
		{			
			storage_flag = (columns[i].flags & STORAGE_MASK);	
			
			if (storage_flag == STORAGE_NONE) // 0x00
			{
				current_entry.rows.push_back(current_row);
				continue;
			}

			if (storage_flag == STORAGE_ZERO) // 0x10
			{
				current_entry.rows.push_back(current_row);	
				continue;
			}

			if (storage_flag == STORAGE_CONSTANT) // 0x30
			{
				current_entry.rows.push_back(current_row);				
				continue;
			}
			
			current_row.type = columns[i].flags & TYPE_MASK;
			current_row.position = Utils::DifPointer(row_ptr, table_hdr);

			switch (current_row.type)
			{
				case 0:
				case 1:
					current_row.u1._u8 = row_ptr;
					row_ptr++;
				break;

				case 2:
				case 3:
					current_row.u1._u16 = (uint16_t *)row_ptr;
					row_ptr += 2;
				break;

				case 4:
				case 5:
					current_row.u1._u32 = (uint32_t *)row_ptr;	
					row_ptr += 4;
					//printf("U32: %x %s\n", val32(*current_row.u1._u64), columns[i].name);
				break;

				case 6:
				case 7:
					current_row.u1._u64 = (uint64_t *)row_ptr;		
					row_ptr += 8;
					//printf("U64: %I64x %s\n", val64(*current_row.u1._u64), columns[i].name);
				break;

				case 8:
					current_row.u1._float = (float *)row_ptr;
					row_ptr += 4;
				break;

				case 0xA:
					current_row.u1.str = (char *)GetOffsetPtr(strings, *(uint32_t *)row_ptr);	
					row_ptr += 4;
					//printf("%s -> %s\n", current_row.u1.str, columns[i].name);
				break;

				case 0xB:
					current_row.u1.data = GetOffsetPtr(data, *(uint32_t *)row_ptr);		
					current_row.data_size = *(uint32_t *)(row_ptr + 4);
					row_ptr += 8;
					current_row.position = Utils::DifPointer(current_row.u1.data, table_hdr);
				break;

				default: 
					DPRINTF("Not implemented!\n");
			}
			
			current_entry.rows.push_back(current_row);			
		}	
		
		rows.push_back(current_entry);
		current_entry.rows.clear();
	}
	
	return true;
}

CpkFile::CpkFile()
{
	big_endian = true;
}

CpkFile::~CpkFile()
{
	
}

void *CpkFile::GetColumnData(CpkUTF & utf, int row, const char *name)
{
	for (size_t i = 0; i < utf.columns.size(); i++)
	{
		uint32_t storage_flag = (utf.columns[i].flags & STORAGE_MASK);	
		
		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO || storage_flag == STORAGE_CONSTANT)
		{
			continue;
		}
		
		if (strcmp(utf.columns[i].name, name) == 0)
		{			
			
			return utf.rows[row].rows[i].GetValue();
		}
	}
	
	return NULL;
}

uint64_t CpkFile::GetColumnDataAsInteger(CpkUTF & utf, int row, const char *name)
{
	for (size_t i = 0; i < utf.columns.size(); i++)
	{
		uint32_t storage_flag = (utf.columns[i].flags & STORAGE_MASK);	
			
		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_CONSTANT)
		{
			continue;
		}
		
		if (strcmp(utf.columns[i].name, name) == 0)
		{
			if (storage_flag == STORAGE_ZERO)
			{
				return 0;
			}
			
			//void *obj = utf.rows[row].rows[i].GetValue();
			int type = utf.rows[row].rows[i].type;
			
			switch (type)
			{
				case 0: case 1:
					return (uint64_t) *utf.rows[row].rows[i].u1._u8;
				break;
				
				case 2: case 3:
					return (uint64_t) val16(*utf.rows[row].rows[i].u1._u16);
				break;
				
				case 4: case 5:
					return (uint64_t) val32(*utf.rows[row].rows[i].u1._u32);
				break;
				
				case 6: case 7:
					return (uint64_t) val64(*utf.rows[row].rows[i].u1._u64);
				break;
				
				default:
					DPRINTF("%s is not integer.\n", name);
					return 0xFFFFFFFFFFFFFFFF;
			}
		}
	}
	
	return 0xFFFFFFFFFFFFFFFF;
}

uint32_t CpkFile::GetColumnPosition(CpkUTF & utf, int row, const char *name)
{
	for (size_t i = 0; i < utf.columns.size(); i++)
	{
		uint32_t storage_flag = (utf.columns[i].flags & STORAGE_MASK);	
			
		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_ZERO || storage_flag == STORAGE_CONSTANT)
		{
			continue;
		}
		
		if (strcmp(utf.columns[i].name, name) == 0)
		{
			return utf.rows[row].rows[i].position;			
		}
	}
	
	return 0xFFFFFFFF;
}

bool CpkFile::SetColumnDataInteger(CpkUTF & utf, int row, const char *name, uint64_t value)
{
	for (size_t i = 0; i < utf.columns.size(); i++)
	{
		uint32_t storage_flag = (utf.columns[i].flags & STORAGE_MASK);	
			
		if (storage_flag == STORAGE_NONE || storage_flag == STORAGE_CONSTANT || storage_flag == STORAGE_ZERO)
		{
			continue;
		}
		
		if (strcmp(utf.columns[i].name, name) == 0)
		{
			int type = utf.rows[row].rows[i].type;
            uint16_t v16;
            uint32_t v32;
			
			switch (type)
			{
				case 0: case 1:
					
					*utf.rows[row].rows[i].u1._u8 = (value&0xFF);
					return true;
					
				break;
				
				case 2: case 3:
					
					v16 = value&0xFFFF;
					*utf.rows[row].rows[i].u1._u16 = val16(v16);
					return true;
					
				break;
				
				case 4: case 5:
					
					v32 = value&0xFFFFFFFF;
					*utf.rows[row].rows[i].u1._u32 = val32(v32);
					return true;
					
				break;
				
				case 6: case 7:
					
					*utf.rows[row].rows[i].u1._u64 = value;
					return true;
					
				break;
				
				default:
					DPRINTF("%s is not integer.\n", name);
					return false;
			}
		}
	}
	
	return false;
}

void CpkFile::SetUnsorted()
{
	if (sorted)
	{
		if (!SetColumnDataInteger(utf, 0, "Sorted", 0))
		{
			DPRINTF("Couldn't set unsorted status!\n");
			return;
		}
		
		sorted = 0;
	}
}

void CpkFile::CorrectValueForOffset(uint64_t *offset)
{
	if (*offset == 0)
		*offset = (uint64_t)-1;	
}

void CpkFile::CorrectValueForSize(uint64_t *size)
{
	if (*size == (uint64_t)-1)
	{
		*size = 0;
	}
}

bool CpkFile::ParseHeaderData(uint8_t *buf)
{
	if (*(uint32_t *)buf != CPK_SIGNATURE)
	{
		return false;
	}
	
	if (!utf.Load(buf + 0x10))
	{
		return false;
	}
	
	for (size_t i = 0; i < utf.columns.size(); i++)
	{
		cpk_data.insert(std::pair<char *, void *>(utf.columns[i].name, utf.rows[0].rows[i].GetValue()));
	}
	
	toc_offset = GetColumnDataAsInteger(utf, 0, "TocOffset");
	CorrectValueForOffset(&toc_offset);
	
	toc_size = GetColumnDataAsInteger(utf, 0, "TocSize");	
	CorrectValueForSize(&toc_size);
	
	etoc_offset = GetColumnDataAsInteger(utf, 0, "EtocOffset");
	CorrectValueForOffset(&etoc_offset);
	
	etoc_size = GetColumnDataAsInteger(utf, 0, "EtocSize");
	CorrectValueForSize(&etoc_size);
	
	itoc_offset = GetColumnDataAsInteger(utf, 0, "ItocOffset");
	CorrectValueForOffset(&itoc_offset);
	
	itoc_size = GetColumnDataAsInteger(utf, 0, "ItocSize");
	CorrectValueForSize(&toc_size);
	
	gtoc_offset = GetColumnDataAsInteger(utf, 0, "GtocOffset");
	CorrectValueForOffset(&gtoc_offset);
	
	gtoc_size = GetColumnDataAsInteger(utf, 0, "GtocSize");
	CorrectValueForSize(&gtoc_size);
	
	content_offset = GetColumnDataAsInteger(utf, 0, "ContentOffset");
	CorrectValueForOffset(&content_offset);
	
	content_size = GetColumnDataAsInteger(utf, 0, "ContentSize");
	CorrectValueForSize(&content_size);
	
	sorted = GetColumnDataAsInteger(utf, 0, "Sorted");
	CorrectValueForSize(&sorted);
	
	DPRINTF("This CPK is sorted: %s\n", (sorted) ? "true" : "false");
	
	/*uint64_t toc_crc = GetColumnDataAsInteger(utf, 0, "TocCrc");	
	printf("Toc_crc=%I64x\n", toc_crc);	
		
	printf("Toc offs=%I64x, size=%I64x\n", toc_offset, toc_size);
	printf("Etoc offs=%I64x, size=%I64x\n", etoc_offset, etoc_size);
	printf("Itoc offs=%I64x, size=%I64x\n", itoc_offset, itoc_size);
	printf("Gtoc offs=%I64x, size=%I64x\n", gtoc_offset, gtoc_size);
	printf("Cont offs=%I64x, size=%I64x\n", content_offset, content_size);*/
	
	return true;
}

bool CpkFile::ParseTocData(uint8_t *buf)
{
	uint64_t add_offset = 0;

	if ((int64_t)content_offset < 0)
		add_offset = toc_offset;
	else
	{
		if ((int64_t)toc_offset < 0)
			add_offset = content_offset;
		else
		{
			if (content_offset < toc_offset)
				add_offset = content_offset;
			else
				add_offset = toc_offset;
		}
	}
	
	if (*(uint32_t *)buf != TOC_SIGNATURE)
	{
		DPRINTF("No toc signature!.\n");
		return false;
	}
	
	if (!toc.Load(buf + 0x10))
	{
		return false;
	}
	
	FileEntry temp;
	
	for (size_t i = 0; i < toc.rows.size(); i++)
	{
        temp.dir_name = (char *)GetColumnData(toc, (int)i, "DirName");
        temp.file_name = (char *)GetColumnData(toc,(int) i, "FileName");
        temp.file_size = GetColumnDataAsInteger(toc, (int)i, "FileSize");
        temp.extract_size = GetColumnDataAsInteger(toc, (int)i, "ExtractSize");
        temp.file_offset = GetColumnDataAsInteger(toc, (int)i, "FileOffset") + add_offset;
        temp.dir_name_pos = GetColumnPosition(toc, (int)i, "DirName");
        temp.file_name_pos = GetColumnPosition(toc, (int)i, "FileName");
		
		//DPRINTF("ID: %I64x\n", GetColumnDataAsInteger(toc, i, "ID"));
		
		/*if (i == 7)
		{
			printf("Dir = %s; file=%s\n", temp.dir_name, temp.file_name);
		}*/		
		
		files_table.push_back(temp);
	}
	
	return true;
}

void CpkFile::RevertEncryption(bool only_toc)
{
	if (!only_toc)
		utf.RevertEncryption();
	
	toc.RevertEncryption();
}

bool CpkFile::UnlinkFilename(int file_index)
{
	uint32_t file_name_pos = files_table[file_index].file_name_pos;
	
	if (!files_table[file_index].file_name || file_name_pos == 0 || file_name_pos == (uint32_t)-1)
	{
		DPRINTF("Cannot unlink this file name becase filename is NULL: 0x%x\n", file_index);
		return false;
	}
	
	uint8_t *toc_table = toc.GetPtrTable();	
	*(uint32_t *)(toc_table + file_name_pos) = 0; // will point the file name to go to "<NULL>" string
	
	SetUnsorted();
	return true;
}

bool CpkFile::UnlinkFileFromDirectory(int file_index)
{	
	uint32_t dir_name_pos = files_table[file_index].dir_name_pos;
	
	if (!files_table[file_index].dir_name || dir_name_pos == 0 || dir_name_pos == (uint32_t)-1)
	{
		DPRINTF("Cannot unlink this file from directory because directory is NULL: %s\n", files_table[file_index].file_name);
		return false;
	}
	
	uint8_t *toc_table = toc.GetPtrTable();	
	*(uint32_t *)(toc_table + dir_name_pos) = 0; // will point the dir name to go to "<NULL>" string
	
	SetUnsorted();
	return true;
}

