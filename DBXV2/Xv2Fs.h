#ifndef __XV2FS_H__
#define __XV2FS_H__

#include "Criware/CriFs.h"

class Xv2Fs : public CriFs
{
public:

    Xv2Fs(const std::string &loose_files_root, bool read_only=false);
};

#endif // __XV2FS_H__
