#ifndef BPEBOOT_H
#define BPEBOOT_H

#include <FileStream.h>

class BPEboot : public BaseFile
{
private:

    FileStream *elf;
    std::string self_path, elf_path, decoder_bin, injector_bin;

protected:
    void Reset();

public:
    BPEboot();
    virtual ~BPEboot() override;

    inline void SetExternal(const std::string &decoder, const std::string &injector)
    {
        decoder_bin = decoder; injector_bin = injector;
    }

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;

    bool VarExists(const std::string &var) const;
    bool VarFits(const std::string &var, const std::string &value) const;

    bool SetVar(const std::string &var, const std::string &value);
};

#endif // BPEBOOT_H
