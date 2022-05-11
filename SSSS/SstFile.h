#ifndef __SSTFILE_H__
#define __SSTFILE_H__

#include "TdbFile.h"

class SstFile : public BaseFile
{
private:

	std::vector<std::string> sign_name;
	std::vector<std::string> robes_name;
	
    void WriteSignName(TiXmlElement *root) const;
    void WriteRobesName(TiXmlElement *root) const;
	
    size_t ReadSignName(const TiXmlElement *root);
    size_t ReadRobesName(const TiXmlElement *root);
	
	void Reset();

public:

	SstFile();
	virtual ~SstFile();
	
	inline bool HasSignName()
	{
		return (sign_name[ENGLISH].length() > 0);
	}
	
	inline bool HasRobesName()
	{
		return (robes_name[ENGLISH].length() > 0);
	}

    inline bool IsEmpty()
    {
        return (!HasSignName() && !HasRobesName());
    }
	
	std::string GetSignName(unsigned int language = ENGLISH, bool revert_to_english = true);
	std::string GetRobesName(unsigned int language = ENGLISH, bool revert_to_english = true);
	
    virtual TiXmlDocument *Decompile() const override;
	
    bool Compile(const TiXmlElement *root);
    virtual bool Compile(TiXmlDocument *doc, bool big_endian=false) override;
};

#endif
