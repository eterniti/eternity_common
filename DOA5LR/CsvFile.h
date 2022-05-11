#ifndef CSVFILE_H
#define CSVFILE_H

#include "BaseFile.h"

struct CsvFrame
{
    uint32_t frame;
    float posx, posy, posz;
    float rotx, roty, rotz;
    float scalex, scaley, scalez;

    CsvFrame()
    {
        frame = 0;
        posx = posy = posz = 0.0f;
        rotx = roty = rotz = 0.0f;
        scalex = scaley = scalez = 1.0f;
    }
};

struct CsvAnim
{
    std::string bone;
    std::vector<CsvFrame> frames;
};

class CsvFile : public BaseFile
{
private:

    std::vector<CsvAnim> anims;

protected:

    void Reset();

public:
    CsvFile();
    virtual ~CsvFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;

    inline size_t GetNumAnims() const { return anims.size(); }

    inline const CsvAnim &operator[](size_t n) const { return anims[n]; }
    inline const CsvAnim &operator[](size_t n) { return anims[n]; }

    inline std::vector<CsvAnim>::const_iterator begin() const { return anims.begin(); }
    inline std::vector<CsvAnim>::const_iterator end() const { return anims.end(); }

    inline std::vector<CsvAnim>::iterator begin() { return anims.begin(); }
    inline std::vector<CsvAnim>::iterator end() { return anims.end(); }
};

#endif // CSVFILE_H
