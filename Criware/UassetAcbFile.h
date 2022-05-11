#ifndef __UASSETACBFILE_H__
#define __UASSETACBFILE_H__

#include "AcbFile.h"
#include "UE4/UassetFile.h"

class UassetAcbFile : public AcbFile, protected UassetFile
{
public:

	UassetAcbFile() : AcbFile(), UassetFile() { }
    virtual ~UassetAcbFile() override { }

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
	virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
	virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;	
	
	// This checks if the file path is possibly a .uasset file that has an .acb inside.
	static bool IsUassetAcb(const std::string &path);
};

#endif /* __UASSETACBFILE_H__ */
	
