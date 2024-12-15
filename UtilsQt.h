#ifndef UTILSQT_H
#define UTILSQT_H

#ifdef QT_VERSION

#include "UtilsMisc.h"

namespace Utils
{
#ifdef _WIN32

    static inline std::string QStringToStdString(const QString &str, bool locale=true)
    {
        if (locale && !IsUtf8())
        {
            QByteArray array = str.toLocal8Bit();
            return std::string(array.data());
        }

        return str.toStdString();
    }

    static inline QString StdStringToQString(const std::string &str, bool locale=true)
    {
        if (locale && !IsUtf8())
            return QString::fromLocal8Bit(str.c_str());

        return QString::fromStdString(str);
    }

#else

    static inline std::string QStringToStdString(const QString &str)
    {
        return str.toStdString();
    }

    static inline QString StdStringToQString(const std::string &str)
    {
        return QString::fromStdString(str));
    }

#endif
}

#endif // QT_VERSION

#endif // UTILSQT_H
