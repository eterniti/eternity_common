#include "UassetAcbFile.h"

bool UassetAcbFile::LoadFromFile(const std::string &path, bool show_error) 
{
	if (!UassetFile::LoadFromFile(path, show_error))
		return false;	
	
	if (GetNumExports() < 1)
	{
		if (show_error)
		{
			DPRINTF("%s: uasset must have at least one export for this .acb format.\n", FUNCNAME);
		}
		
		return false;
	}
	
	size_t bulk_data_size;
	uint8_t *bulk_data = GetBulkData(&bulk_data_size);
	
	if (!bulk_data)
	{
		if (show_error)
		{
			DPRINTF("%s: This file doesn't contain bulk data.\n", FUNCNAME);
		}
		
		return false;
	}
	
	if (bulk_data_size < sizeof(uint32_t) || *(uint32_t *)bulk_data != UTF_SIGNATURE)
	{
		if (show_error)
		{
			DPRINTF("%s: This file doesn't contain .acb data.\n", FUNCNAME);
		}
		
		delete[] bulk_data;
		return false;
	}
	
	bool ret = AcbFile::Load(bulk_data, bulk_data_size);
	if (!ret)
	{
		if (show_error)
		{
			DPRINTF("%s: AcbFile load failed. Either this file doesn't containt acb data or is currently unsupported.\n", FUNCNAME);
		}		
	}
	else
	{
        // Commented. Checking this should be done by a higher level entity
        /*if (!HasAwb())
		{
			if (show_error)
			{
				DPRINTF("%s: For .acb in .uasset, only standalone ones are supported.\n", FUNCNAME);
			}
			
			ret = false;
        }*/
	}
	
	delete[] bulk_data;		
	return ret;
}

bool UassetAcbFile::SaveToFile(const std::string &path, bool show_error, bool build_path) 
{
    size_t size;
    uint8_t *buf = AcbFile::Save(&size);
	
	if (!buf)
		return false;
	
	if (!SetBulkData(buf, size))
	{
		delete[] buf;
		return false;
	}
	
	delete[] buf;
	
    uint32_t serial_size;
	uint8_t *serial;
	
	serial = GetSerial(0, &serial_size);
	if (!serial)
		return false;
	
	if (serial_size < 0x10)
	{
		if (show_error)
		{
			DPRINTF("%s: Serial for export 0 is too small.\n", FUNCNAME);
		}
		
		delete[] serial;
		return false;
	}
	
	// FIXME: this is just a quick hack to get the thing done. 
    *(uint32_t *)(serial+serial_size-0x10) = (uint32_t)size;
    *(uint32_t *)(serial+serial_size-0xC) = (uint32_t)size;
	
	if (!SetSerial(0, serial, serial_size))
	{
		delete[] serial;
		return false;
	}
	
	delete[] serial;
	
	if (!UassetFile::SaveToFile(path, show_error, build_path))
		return false;
	
	// Apend dummy signature at end
	uint32_t signature = UASSET_SIGNATURE;	
	
	FILE *f = fopen(path.c_str(), "ab");
	if (!f)
	{
		return false;
	}
	
	if (fwrite(&signature, 1, sizeof(uint32_t), f) != sizeof(uint32_t))
		return false;
	
	fclose(f);	
	return true;	
}

// TODO: a proper Load should be create in UassetFile to avoid this temp file thing.
bool UassetAcbFile::Load(const uint8_t *buf, size_t size)
{
	std::string temp_path = Utils::GetTempFile("UassetAcbFile", ".tmp");
	
	if (!Utils::WriteFileBool(temp_path, buf, size))
		return false;
	
	return LoadFromFile(temp_path);	
}

// TODO: a proper Load should be create in UassetFile to avoid this temp file thing.
uint8_t *UassetAcbFile::Save(size_t *psize)
{
	std::string temp_path = Utils::GetTempFile("UassetAcbFile", ".tmp");
	
	if (!SaveToFile(temp_path))
		return nullptr;
	
	size_t size;
	uint8_t *buf = Utils::ReadFile(temp_path, &size);	
	if (!buf)
		return nullptr;
	
	if (size > 0xFFFFFFFF)
	{
		delete[] buf;
		return nullptr;
	}
	
	*psize = (unsigned int)size;
	return buf;
}

bool UassetAcbFile::IsUassetAcb(const std::string &path)
{
	UassetFile uasset;
	UtfFile utf;
	uint8_t *acb_buf;
	size_t acb_size;
	
	if (!uasset.LoadFromFile(path, false))
		return false;
	
	if (uasset.GetNumExports() < 1)
		return false;
	
	acb_buf = uasset.GetBulkData(&acb_size);
	if (!acb_buf)
		return false;
	
	bool ret = utf.Load(acb_buf, acb_size);
	delete[] acb_buf;
	
	if (ret)
	{
        if (!utf.ColumnExists("CueTable") && !utf.ColumnExists("TrackTable") && !utf.ColumnExists("AwbFile") && !utf.ColumnExists("StreamAwbAfs2Header"))
		{
			ret = false;
		}
	}
	
	return ret;
}
