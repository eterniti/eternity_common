#ifndef __UASSETFILE_H__
#define __UASSETFILE_H__

#include "BaseFile.h"
#include "UE4Common.h"

#define UASSET_SIGNATURE				0x9E2A83C1
#define UASSET_SIGNATURE_OTHER_ENDIAN	0xC1832A9E

#define CURRENT_LEGACY_VERSION	-6

struct FGenerationInfo
{
	uint32_t exports_count;
	uint32_t names_count;
};

struct FObjectImport
{
	int64_t class_package;
	int64_t class_name;
	int32_t outer_index;
	int64_t object_name;
};

struct FObjectExport
{
	int32_t class_index;
	int32_t super_index;
	int32_t outer_index;
	int64_t object_name;
	uint32_t save;
	uint32_t serial_size;
	off64_t serial_offset;
	int32_t forced_export;
	int32_t not_for_client;
	int32_t not_for_server;
	uint8_t package_guid[0x10];
	int32_t not_for_editor_game;
	////// Not real members
	std::vector<uint32_t> depends;
	uint8_t *serial_buf;
	
	FObjectExport()
	{
		serial_buf = nullptr;
	}
	
	~FObjectExport()
	{
		if (serial_buf)
			delete[] serial_buf;		
	}
};

class UassetFile : public BaseFile
{
private:

	FILE *r_handle;
	FILE *w_handle;
	
	int32_t legacy_version;
	int32_t ue3_legacy_version;
	int32_t ue4_file_version;
	int32_t ue4_file_version_licensee;
	
    UE4Common::FString folder_name;
	uint32_t package_flags;
	
    std::vector<UE4Common::FString> names;
	std::vector<FObjectImport> imports;
	std::vector<FObjectExport> exports;
	
	uint8_t guid[0x10];	
	std::vector<FGenerationInfo> gen_infos;
	
	uint16_t version_major;
	uint16_t version_minor;
	uint16_t version_patch;
	uint32_t version_changelist;
    UE4Common::FString version_branch;
	
	uint32_t package_source;
	
	off64_t bulk_data_offset;
	uint8_t *bulk_data_buf;
	size_t bulk_data_size;
	
    std::vector<UE4Common::FString> str_assets_refs;

    bool ReadFGenerationInfo(FGenerationInfo &info);
	bool ReadFObjectImport(FObjectImport &import);
	bool ReadFObjectExport(FObjectExport &exp);
	
    bool WriteFGenerationInfo(const FGenerationInfo &info);
	bool WriteFObjectImport(const FObjectImport &import);
	bool WriteFObjectExport(const FObjectExport &exp);
	
	void CalculateOffsetsAndSizes(uint32_t &header_size, uint32_t &names_offset, uint32_t &exports_offset, uint32_t &imports_offset, uint32_t &depends_offset, uint32_t &str_assets_refs_offset, uint32_t &assets_registry_offset, uint64_t &bulk_data_offset, std::vector<uint64_t> &serial_offsets) const;
	
	static bool CopyFile(FILE *src, FILE *dst, size_t size);
	
protected:

	void Reset();

public:

	UassetFile();
	virtual ~UassetFile();
	
	virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
	virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;	
	
	uint8_t *GetBulkData(size_t *psize) const;
	bool SetBulkData(const uint8_t *buf, size_t size);
	
	inline size_t GetNumExports() const { return exports.size(); }
	
    uint8_t *GetSerial(size_t idx, uint32_t *psize) const;
    bool SetSerial(size_t idx, const uint8_t *buf, uint32_t size);
};

#endif /* __UASSETFILE_H__ */
