#ifndef __SSMINSCONFIGFILE_H__
#define __SSMINSCONFIGFILE_H__

#include <stdint.h>

#include <string>
#include <vector>

#include "SszFile.h"

struct SsssMod
{
	std::string name;
	std::string author;
	float version;
	uint8_t guid[16];
	SszType type;
	std::vector<std::string> files;

    // Specific for new characters
    uint32_t cms_entry;
    uint32_t cms_model_spec_idx;
    uint32_t model_id;
};

class SsminsConfigFile : public BaseFile
{
private:

	std::string ssss_dir;
    std::string last_dir;
	std::vector<SsssMod> mods;
	
	void Reset();
	
    void WriteMod(TiXmlElement *root, const SsssMod *mod) const;
	bool ReadMod(TiXmlElement *root, SsssMod *mod);

public:

	SsminsConfigFile();
	virtual ~SsminsConfigFile();

    inline bool HasMods()
    {
        return (mods.size() > 0);
    }

    inline void ClearMods()
    {
        mods.clear();
    }
	
    inline const std::string & GetSsssDir() { return ssss_dir; }
	inline void SetSsssdir(const std::string & dir) { ssss_dir = dir; }

    const std::string & GetLastDir() { return last_dir; }
    inline void SetLastDir(const std::string & dir) { last_dir = dir; }
	
	inline size_t GetNumMods() { return mods.size(); }
	inline SsssMod *GetMod(size_t index)
	{
		if (index >= mods.size())
            return nullptr;
		
		return &mods[index];
	}

    inline SsssMod *FindMod(uint32_t cms_entry, uint32_t cms_model_spec_idx)
    {
        for (SsssMod &mod : mods)
        {
            if (mod.cms_entry == cms_entry && mod.cms_model_spec_idx == cms_model_spec_idx)
                return &mod;
        }

        return nullptr;
    }

    inline bool ModExists(uint32_t cms_entry, uint32_t cms_model_spec_idx) { return (FindMod(cms_entry, cms_model_spec_idx) != nullptr); }
	
	inline void AppendMod(SsssMod *mod) { mods.push_back(*mod); }
	inline bool RemoveMod(size_t index)
	{
		if (index >= mods.size())
			return false;
		
		mods.erase(mods.begin()+index);
        return true;
	}
	
    virtual TiXmlDocument *Decompile() const override;
	virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
