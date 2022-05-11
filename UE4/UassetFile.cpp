// WARNING: this file is totally WIP. It only supports a small subset of .uasset files.

// We always want assertions for this totally wip code
#undef NDEBUG

#include "UassetFile.h"
#include "debug.h"

#define COPY_BUF_SIZE	(16*1024*1024)

using namespace UE4Common;

UassetFile::UassetFile()
{
	r_handle = w_handle = nullptr;
	bulk_data_buf = nullptr;
	Reset();
}

UassetFile::~UassetFile()
{
    Reset();
}

void UassetFile::Reset()
{
	if (r_handle)
		fclose(r_handle);
	
	if (w_handle)
		fclose(w_handle);
	
	r_handle = w_handle = nullptr;
	
	legacy_version = -5;
	ue3_legacy_version = 0;
	ue4_file_version = 0;
	ue4_file_version_licensee = 0;
	
	folder_name.Reset();
	package_flags = 0;
	
	names.clear();
	imports.clear();	
	exports.clear();
	
	memset(guid, 0, sizeof(guid));
	gen_infos.clear();
	
	version_major = 0;
	version_minor = 0;
	version_patch = 0;
	version_changelist = 0;
	version_branch.Reset();
	
	package_source = 0;
	
	if (bulk_data_buf)
		delete[] bulk_data_buf;
	
	bulk_data_offset = 0;
	bulk_data_buf = nullptr;
	bulk_data_size = 0;
	
	str_assets_refs.clear();
}

// DO NOT USE _ftelli64 or _fseeki64, they do weird things in my system.
#define SAVE_POSITION() { position = ftello64(r_handle); }
#define RESTORE_POSITION() {fseeko64(r_handle, position, SEEK_SET); }

bool UassetFile::ReadFGenerationInfo(FGenerationInfo &info)
{
	if (fread(&info.exports_count, 1, sizeof(uint32_t), r_handle) != sizeof(uint32_t))
		return false;
	
	if (fread(&info.names_count, 1, sizeof(uint32_t), r_handle) != sizeof(uint32_t))
		return false;
	
	return true;
}

bool UassetFile::WriteFGenerationInfo(const FGenerationInfo &info)
{
	if (fwrite(&info.exports_count, 1, sizeof(uint32_t), w_handle) != sizeof(uint32_t))
		return false;
	
	if (fwrite(&info.names_count, 1, sizeof(uint32_t), w_handle) != sizeof(uint32_t))
		return false;
	
	return true;
}

bool UassetFile::ReadFObjectImport(FObjectImport &import)
{
	if (fread(&import.class_package, 1, sizeof(int64_t), r_handle) != sizeof(int64_t))
		return false;
	
	if (fread(&import.class_name, 1, sizeof(int64_t), r_handle) != sizeof(int64_t))
		return false;
	
	if (fread(&import.outer_index, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;
	
	if (fread(&import.object_name, 1, sizeof(int64_t), r_handle) != sizeof(int64_t))
		return false;
	
	return true;
}

bool UassetFile::WriteFObjectImport(const FObjectImport &import)
{
	if (fwrite(&import.class_package, 1, sizeof(int64_t), w_handle) != sizeof(int64_t))
		return false;
	
	if (fwrite(&import.class_name, 1, sizeof(int64_t), w_handle) != sizeof(int64_t))
		return false;
	
	if (fwrite(&import.outer_index, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;
	
	if (fwrite(&import.object_name, 1, sizeof(int64_t), w_handle) != sizeof(int64_t))
		return false;
	
	return true;
}

bool UassetFile::ReadFObjectExport(FObjectExport &exp)
{	
	if (fread(&exp.class_index, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;	
	
	if (fread(&exp.super_index, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;
	
	if (fread(&exp.outer_index, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;
	
	if (fread(&exp.object_name, 1, sizeof(int64_t), r_handle) != sizeof(int64_t))
		return false;
	
	if (fread(&exp.save, 1, sizeof(uint32_t), r_handle) != sizeof(uint32_t))
		return false;	
	
	if (fread(&exp.serial_size, 1, sizeof(uint32_t), r_handle) != sizeof(uint32_t))
		return false;	
	
	if (fread(&exp.serial_offset, 1, sizeof(off64_t), r_handle) != sizeof(off64_t))
		return false;	
	
	if (fread(&exp.forced_export, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;	
	
	if (fread(&exp.not_for_client, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;	
	
	if (fread(&exp.not_for_server, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;	
	
	if (fread(exp.package_guid, 1, sizeof(exp.package_guid), r_handle) != sizeof(exp.package_guid))
		return false;
	
	if (fread(&exp.not_for_editor_game, 1, sizeof(int32_t), r_handle) != sizeof(int32_t))
		return false;	
	
	return true;
}

bool UassetFile::WriteFObjectExport(const FObjectExport &exp)
{	
	if (fwrite(&exp.class_index, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;	
	
	if (fwrite(&exp.super_index, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;
	
	if (fwrite(&exp.outer_index, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;
	
	if (fwrite(&exp.object_name, 1, sizeof(int64_t), w_handle) != sizeof(int64_t))
		return false;
	
	if (fwrite(&exp.save, 1, sizeof(uint32_t), w_handle) != sizeof(uint32_t))
		return false;	
	
	if (fwrite(&exp.serial_size, 1, sizeof(uint32_t), w_handle) != sizeof(uint32_t))
		return false;	
	
	if (fwrite(&exp.serial_offset, 1, sizeof(off64_t), w_handle) != sizeof(off64_t))
		return false;	
	
	if (fwrite(&exp.forced_export, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;	
	
	if (fwrite(&exp.not_for_client, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;	
	
	if (fwrite(&exp.not_for_server, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;	
	
	if (fwrite(exp.package_guid, 1, sizeof(exp.package_guid), w_handle) != sizeof(exp.package_guid))
		return false;
	
	if (fwrite(&exp.not_for_editor_game, 1, sizeof(int32_t), w_handle) != sizeof(int32_t))
		return false;	
	
	return true;
}

#define READFILE(buf, size) { \
	if (fread(buf, 1, size, r_handle) != size) { \
		if (show_error) { DPRINTF("%s: Failed to read from file \"%s\"\n", FUNCNAME, path.c_str()); } \
		return false; \
} \
}

#define READ16(pi) READFILE(pi, 2)
#define READ32(pi) READFILE(pi, 4)
#define READ64(pi) READFILE(pi, 8)

#define READSTRING(str) { \
    if (!str.Read(r_handle)) { \
		if (show_error) { DPRINTF("%s: Failed to read string from file \"%s\"\n", FUNCNAME, path.c_str()); } \
		return false; \
	} \
} 

bool UassetFile::LoadFromFile(const std::string &path, bool show_error)
{
	Reset();
	
	r_handle = fopen(path.c_str(), "rb");
	
	if (!r_handle)
	{
		if (show_error)
		{
			DPRINTF("%s: Cannot open file \"%s\"\n", FUNCNAME, path.c_str());
		}
		
		return false;
	}
	
	off64_t position;
	uint32_t signature, array_size, header_size;
	uint32_t names_count, names_offset, exports_count, exports_offset, imports_count, imports_offset, depends_offset;
	uint32_t str_assets_refs_count, str_assets_refs_offset, thumbnail_table_offset;
	uint32_t compression_flags, unknown;
	uint32_t assets_registry_offset, world_title_info_offset;
	uint32_t objects_count;
	
	READ32(&signature);	
	if (signature != UASSET_SIGNATURE)
	{
		if (show_error)
		{
			if (signature == UASSET_SIGNATURE_OTHER_ENDIAN)
			{
				DPRINTF("%s: No support for opposite endianess yet.\n", FUNCNAME);
			}
			else
			{
				DPRINTF("%s: File is not a UE4 Package file.\n", FUNCNAME);
			}
		}
		
		return false;
	}
	
	READ32(&legacy_version);	
	if (legacy_version < CURRENT_LEGACY_VERSION)
	{
		if (show_error)
		{
			DPRINTF("%s: This file is from the future.\n", FUNCNAME);			
		}
		
		return false;
	}
	else if (legacy_version >= 0)
	{
		if (show_error)
		{
			DPRINTF("%s: UE3 files not supported.\n", FUNCNAME);
		}
		
		return false;
	}
	
	if (legacy_version != -4)
	{
		READ32(&ue3_legacy_version);
	}
	
	READ32(&ue4_file_version);
	READ32(&ue4_file_version_licensee); 
	READ32(&array_size); 
	
	if (array_size != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: I don't know how to proceed (array_size != 0)\n", FUNCNAME);
		}
		
		return false;
	}
	
	READ32(&header_size);
	READSTRING(folder_name);
	READ32(&package_flags);
	READ32(&names_count);
	READ32(&names_offset);
	READ32(&exports_count);
	READ32(&exports_offset);
	READ32(&imports_count);
	READ32(&imports_offset);
	READ32(&depends_offset);
	
	READ32(&str_assets_refs_count);
	READ32(&str_assets_refs_offset);
	READ32(&thumbnail_table_offset);
	
	if (thumbnail_table_offset != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: I don't know how to proceed (thumbnail_table_offset != 0)\n", FUNCNAME);
		}
		
		return false;
	}
	
	READFILE(guid, sizeof(guid));	
	READ32(&array_size);
	
	if (array_size > 0)
	{
		gen_infos.resize(array_size);
		
		for (FGenerationInfo &info : gen_infos)
		{
			if (!ReadFGenerationInfo(info))
			{
				if (show_error)
				{
					DPRINTF("%s Failed to read FGenerationInfo.\n", FUNCNAME);
				}
				
				return false;
			}
		}
	}
	
	READ16(&version_major);
	READ16(&version_minor);
	READ16(&version_patch);
	READ32(&version_changelist); 
	READSTRING(version_branch);
	
	READ32(&compression_flags);
	READ32(&array_size);
	READ32(&package_source);
	
	if (compression_flags != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: Compression flags not 0. Aborting.\n", FUNCNAME);
		}
		
		return false;
	}
	
	if (array_size != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: compressed_chunks array_size != 0, aborting.\n", FUNCNAME);
		}
		
		return false;
	}
	
	READ32(&array_size);
	
	if (array_size != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: additional packages array_size != 0, I cannot proceed.\n", FUNCNAME);
		}
		
		return false;
	}
	
	READ32(&unknown); // Texture allocations? but that should be 0x18 bytes...
	
	if (unknown != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: I cannot proceed (unknown != 0).\n", FUNCNAME);
		}
		
		return false;
	}
	
	READ32(&assets_registry_offset);
	READ64(&bulk_data_offset);	
	READ32(&world_title_info_offset);
	READ32(&array_size);
	
	if (world_title_info_offset != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: world_title_info_offset != 0, I cannot proceed.\n", FUNCNAME);
		}
		
		return false;
	}
	
	if (array_size != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: chunk id array size not 0, I cannot proceed.\n", FUNCNAME);
		}
		
		return false;
	}
	
	SAVE_POSITION();
	assert(names_count == 0 || position == names_offset); 
	
	// First part of the header is over now
	
	if (names_count > 0)
	{	
		names.resize(names_count);
		fseeko64(r_handle, names_offset, SEEK_SET);
		
		for (FString &name : names)
		{
			READSTRING(name);
			//DPRINTF("%x: %s\n",name.c_str()); 
		}
	}
	
	SAVE_POSITION();
	assert(imports_count == 0 || position == imports_offset);
	
	if (imports_count > 0)
	{
		imports.resize(imports_count);
		fseeko64(r_handle, imports_offset, SEEK_SET);
		
		for (FObjectImport &import : imports)
		{
			if (!ReadFObjectImport(import))
			{
				if (show_error)
				{
					DPRINTF("%s: Failed to read FObjectImport.\n", FUNCNAME);
				}
				
				return false;
			}
		}
	}
	
	SAVE_POSITION();
	assert(exports_count == 0 || position == exports_offset);
	
	if (exports_count > 0)
	{
		exports.resize(exports_count);
		fseeko64(r_handle, exports_offset, SEEK_SET);
		
		for (FObjectExport &exp : exports)
		{
			if (!ReadFObjectExport(exp))
			{
				if (show_error)
				{
					DPRINTF("%s Failed to read FObjectExport.\n", FUNCNAME);
				}
				
				return false;
			}
		}
	}
	
	SAVE_POSITION();
	assert(exports_count == 0 || position == depends_offset);
	
	if (exports_count > 0)
	{
		fseeko64(r_handle, depends_offset, SEEK_SET);

		for (uint32_t i = 0; i < exports_count; i++)
		{
			READ32(&array_size);
			
			exports[i].depends.resize(array_size);
			
			for (auto &dep : exports[i].depends)
			{
				READ32(&dep);
			}
		}
	}
	
	SAVE_POSITION();
	assert(str_assets_refs_offset == 0 ||position == str_assets_refs_offset);
	
	if (str_assets_refs_count > 0)
	{
		str_assets_refs.resize(str_assets_refs_count);
		fseeko64(r_handle, str_assets_refs_offset, SEEK_SET);
		
		for (FString &str : str_assets_refs)
		{
			READSTRING(str);
		}
	}
	
	SAVE_POSITION();
	assert(assets_registry_offset == 0 || position == assets_registry_offset);
	
	fseeko64(r_handle, assets_registry_offset, SEEK_SET);
	READ32(&objects_count);
	
	if (objects_count != 0)
	{
		if (show_error)
		{
			DPRINTF("%s: objects_count != 0, cannot proceed.\n", FUNCNAME);
		}
		
		return false;
	}
	
	SAVE_POSITION();
	assert(position == header_size);
	
	for (size_t i = 0; i < exports.size(); i++)
	{
		position += exports[i].serial_size;
	}
	
	
	fseeko64(r_handle, 0, SEEK_END);
	bulk_data_size = ftello64(r_handle) - position;
	
	return true;
}

void UassetFile::CalculateOffsetsAndSizes(uint32_t &header_size, uint32_t &names_offset, uint32_t &exports_offset, uint32_t &imports_offset, uint32_t &depends_offset,
											  uint32_t &str_assets_refs_offset, uint32_t &assets_registry_offset, uint64_t &bulk_data_offset, std::vector<uint64_t> &serial_offsets) const
{
	header_size = 0x18;
	
	if (legacy_version != -4)
		header_size += 4;
	
	// So far, up to field "header_size"
		
    header_size += folder_name.TotalSize();
	header_size += 0x40;
    header_size += 8 * (uint32_t)gen_infos.size();
	header_size += 0xA;
    header_size += version_branch.TotalSize();
	header_size += 0x28;
	
	names_offset = (names.size() == 0) ? 0 : header_size;
	
	for (const FString &str : names)
	{
        header_size += str.TotalSize();
	}
	
	imports_offset = (imports.size() == 0) ? 0 : header_size;	
    header_size += 0x1C * (uint32_t)imports.size();
	
	exports_offset = (exports.size() == 0) ? 0 : header_size;
    header_size += 0x44 * (uint32_t)exports.size();
	
	depends_offset = (exports.size() == 0) ? 0 : header_size;
    header_size += 4 * (uint32_t)exports.size();
	
	for (const FObjectExport &exp : exports)
	{
        header_size += 4 * (uint32_t)exp.depends.size();
	}
	
	str_assets_refs_offset = header_size;
	
	for (const FString&str : str_assets_refs)
	{
		header_size += str.TotalSize();
	}
	
	assets_registry_offset = header_size;
	
	header_size += 4;	
	
	bulk_data_offset = header_size;
	
	serial_offsets.resize(exports.size());	
	for (size_t i = 0; i < serial_offsets.size(); i++)
	{
		if (i == 0)
		{
			serial_offsets[i] = header_size;
		}
		else
		{
			serial_offsets[i] = serial_offsets[i-1] + exports[i-1].serial_size;
		}
		
		if (i == (serial_offsets.size()-1))
		{
			bulk_data_offset = serial_offsets[i] + exports[i].serial_size;
		}
	}
}

bool UassetFile::CopyFile(FILE *src, FILE *dst, size_t size)
{
	uint8_t *copy_buf;
	size_t remaining;
		
    if (size < COPY_BUF_SIZE)
    {
        copy_buf = new uint8_t[size];
    }
    else
    {
        copy_buf = new uint8_t[COPY_BUF_SIZE];
    }

	remaining = size;
	
	while (remaining > 0)
	{
		size_t r = (remaining > COPY_BUF_SIZE) ? COPY_BUF_SIZE : remaining;
		
		if (fread(copy_buf, 1, r, src) != r)
		{
			delete[] copy_buf;
			return false;
		}
		
		if (fwrite(copy_buf, 1, r, dst) != r)
		{
			delete[] copy_buf;						
			return false;
		}
		
		remaining -= r;
	}
	
	delete[] copy_buf;	
	return true;
}

#define WRITEFILE(buf, size) { \
	if (fwrite(buf, 1, size, w_handle) != size) { \
		if (show_error) { DPRINTF("%s: Failed to write to file \"%s\"\n", FUNCNAME, path.c_str()); } \
		return false; \
	} \
	offset += size; \
}

#define WRITE16(i) { uint16_t val = (uint16_t)i;  WRITEFILE(&val, 2); }
#define WRITE32(i) { uint32_t val = (uint32_t)i;  WRITEFILE(&val, 4); }
#define WRITE64(i) { uint64_t val = (uint64_t)i;  WRITEFILE(&val, 8); }

#define WRITESTRING(str) { \
    if (!str.Write(w_handle)) { \
		if (show_error) { DPRINTF("%s: Failed to write string to file \"%s\"\n", FUNCNAME, path.c_str()); } \
		return false; \
	} \
	offset += str.TotalSize(); \
} 

bool UassetFile::SaveToFile(const std::string &path, bool show_error, bool build_path) 
{
	uint64_t offset = 0;
	uint32_t header_size, names_offset, exports_offset, imports_offset, depends_offset, str_assets_refs_offset, assets_registry_offset;
	uint64_t bulk_data_offset;
	std::vector<uint64_t> serial_offsets;

    if (!r_handle)
	{
		DPRINTF("%s: Bad function usage, file must be opened before.\n", FUNCNAME);
		return false;
	}
	
	if (w_handle)
		fclose(w_handle);
	
	w_handle = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
	if (!w_handle)
	{
		if (show_error)
		{
			DPRINTF("%s: Failed to open/create file %s\n", FUNCNAME, path.c_str());
		}
		
		return false;
	}
	
	CalculateOffsetsAndSizes(header_size, names_offset, exports_offset, imports_offset, depends_offset, str_assets_refs_offset, assets_registry_offset, bulk_data_offset, serial_offsets);
	
    WRITE32(UASSET_SIGNATURE);
	WRITE32(legacy_version);
	
	if (legacy_version != -4)
	{
		WRITE32(ue3_legacy_version);
	}
	
	WRITE32(ue4_file_version);
	WRITE32(ue4_file_version_licensee); 
	WRITE32(0); 	
	
	WRITE32(header_size);
	WRITESTRING(folder_name);
	WRITE32(package_flags);
	WRITE32(names.size());
	WRITE32(names_offset);
	WRITE32(exports.size());
	WRITE32(exports_offset);
	WRITE32(imports.size());
	WRITE32(imports_offset);
	WRITE32(depends_offset);
	
	WRITE32(str_assets_refs.size());
	WRITE32(str_assets_refs_offset);
	WRITE32(0);	
	
	WRITEFILE(guid, sizeof(guid));	
	WRITE32(gen_infos.size());
	
	for (const FGenerationInfo &info : gen_infos)
	{
		if (!WriteFGenerationInfo(info))
		{
			if (show_error)
			{
				DPRINTF("%s: Cannot write FGenerationInfo.\n", FUNCNAME);
			}
			
			return false;
		}
	}
	
	WRITE16(version_major);
	WRITE16(version_minor);
	WRITE16(version_patch);
	WRITE32(version_changelist); 
	WRITESTRING(version_branch);
	
	WRITE32(0);
	WRITE32(0);
	WRITE32(package_source);
	
	WRITE32(0);
	WRITE32(0);
	
	WRITE32(assets_registry_offset);
	WRITE64(bulk_data_offset);	
	WRITE32(0);
	WRITE32(0);
	
	for (const FString &str : names)
	{
		WRITESTRING(str);
	}
	
	for (const FObjectImport &import : imports)
	{
		if (!WriteFObjectImport(import))
		{
			if (show_error)
			{
				DPRINTF("%s: Cannot write FObjectImport.\n", FUNCNAME);
			}
			
			return false;
		}
	}

    for (size_t i = 0; i < exports.size(); i++)
	{
		FObjectExport exp = exports[i]; // must copy, we don't want to mod original one        
        exp.serial_buf = nullptr; // WE really need this in the copy or we'll crash in destructor
		
		exp.serial_offset = serial_offsets[i];
		
		if (!WriteFObjectExport(exp))
		{
			if (show_error)
			{
				DPRINTF("%s: Cannot write FObjectExport.\n", FUNCNAME);
			}
			
			return false;
		}
	}   
	
	for (const FObjectExport &exp : exports)
	{
		WRITE32(exp.depends.size());
		
		for (auto &dep : exp.depends)
		{
			WRITE32(dep);
		}
	}
	
	for (const FString &str : str_assets_refs)
	{
		WRITESTRING(str);
	}
	
	WRITE32(0);    
	
	for (size_t i = 0; i < exports.size(); i++)
	{
		if (!exports[i].serial_buf)
		{
			fseeko64(r_handle, exports[i].serial_offset, SEEK_SET);
			if (!CopyFile(r_handle, w_handle, exports[i].serial_size))
			{
				if (show_error)
				{
					DPRINTF("%s: Failed to copy portion of file.\n", FUNCNAME);
				}
				
				return false;
			}
		}
		else
		{            
            WRITEFILE(exports[i].serial_buf, exports[i].serial_size);
		}
	}
	
	if (this->bulk_data_offset != 0 || this->bulk_data_buf)
	{
		if (this->bulk_data_buf)
		{
			WRITEFILE(this->bulk_data_buf, this->bulk_data_size);
		}
		else
		{		
			fseeko64(r_handle, this->bulk_data_offset, SEEK_SET);
			if (!CopyFile(r_handle, w_handle, this->bulk_data_size))
			{
				if (show_error)
				{
					DPRINTF("%s: Failed to copy portion of file (2).\n", FUNCNAME);
				}
				
				return false;
			}
		}
	}
	
	fclose(w_handle);
	w_handle = nullptr;
		
	return true;
}

uint8_t *UassetFile::GetBulkData(size_t *psize) const
{
	if (!bulk_data_offset || !bulk_data_size)
		return nullptr;
	
    uint8_t *buf = new uint8_t[bulk_data_size];

	if (bulk_data_buf)
	{
		memcpy(buf, bulk_data_buf, bulk_data_size);
	}
	else
	{
		if (!r_handle)
		{
			delete[] buf;
			return nullptr;
		}
		
		fseeko64(r_handle, bulk_data_offset, SEEK_SET);
		if (fread(buf, 1, bulk_data_size, r_handle) != bulk_data_size)
		{
			delete[] buf;
			return nullptr;
		}
	}
	
	*psize = bulk_data_size;	
	return buf;
}

bool UassetFile::SetBulkData(const uint8_t *buf, size_t size)
{
	if (bulk_data_buf)
    {
		delete[] bulk_data_buf;
        bulk_data_buf = nullptr;
    }
	
    bulk_data_buf = new uint8_t[size];
	memcpy(bulk_data_buf, buf, size);
	bulk_data_size = size;
	
	return true;
}

uint8_t *UassetFile::GetSerial(size_t idx, uint32_t *psize) const
{
	if (idx >= exports.size())
		return nullptr;
	
	if (exports[idx].serial_size == 0)
		return nullptr;
	
	*psize = exports[idx].serial_size;	

    uint8_t *buf = new uint8_t[*psize];
	
	if (exports[idx].serial_buf)
	{
		memcpy(buf, exports[idx].serial_buf, *psize);
	}
	else
	{
		if (!r_handle || exports[idx].serial_offset == 0)
		{
			delete[] buf;
			return nullptr;
		}
		
		fseeko64(r_handle, exports[idx].serial_offset, SEEK_SET);
		if (fread(buf, 1, *psize, r_handle) != *psize)
		{
			delete[] buf;
			return nullptr;
		}        
	}
	
	return buf;
}

bool UassetFile::SetSerial(size_t idx, const uint8_t *buf, uint32_t size)
{
	if (idx >= exports.size())
		return false;
	
	if (exports[idx].serial_buf)
    {
		delete[] exports[idx].serial_buf;
        exports[idx].serial_buf = nullptr;
    }
	
    exports[idx].serial_buf = new uint8_t[size];
	memcpy(exports[idx].serial_buf, buf, size);
    exports[idx].serial_size = size;    
	
	return true;
}

