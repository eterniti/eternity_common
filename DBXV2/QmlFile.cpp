#include "QmlFile.h"
#include "debug.h"

QmlFile::QmlFile()
{
    this->big_endian = false;
}

QmlFile::~QmlFile()
{

}

void QmlFile::Reset()
{
    entries.clear();
}

bool QmlFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(QMLHeader))
        return false;

    const QMLHeader *hdr = (const QMLHeader *)buf;
    if (hdr->signature != QML_SIGNATURE)
        return false;

    const QMLEntry *file_entry = (const QMLEntry *)GetOffsetPtr(hdr, hdr->data_start);
    entries.resize(hdr->num_entries);

    for (QmlEntry &entry : entries)
    {
        entry = *file_entry;
        file_entry++;
    }

    return true;
}

uint8_t *QmlFile::Save(size_t *psize)
{
    *psize = sizeof(QMLHeader) + entries.size()*sizeof(QMLEntry);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    QMLHeader *hdr = (QMLHeader *)buf;
    hdr->signature = QML_SIGNATURE;
    hdr->endianess_check = 0xFFFE;
    hdr->header_size = sizeof(QMLHeader);
    hdr->num_entries = (uint32_t)entries.size();
    hdr->data_start = sizeof(QMLHeader);

    QMLEntry *file_entry = (QMLEntry *)(hdr + 1);
    for (const QmlEntry &entry : entries)
    {
        *file_entry = entry;
        file_entry++;
    }

    return buf;
}

bool QmlFile::SpecialCompare(const QmlFile &rhs, QxdFile &qxd) const
{
    if (entries.size() != rhs.entries.size())
        return false;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (!entries[i].SpecialCompare(rhs.entries[i], qxd))
            return false;
    }

    return true;
}

QmlEntry *QmlFile::FindEntryById(uint32_t id)
{
    for (QmlEntry &entry : entries)
        if (entry.id == id)
            return &entry;

    return nullptr;
}

bool QMLEntry::SpecialCompare(const QMLEntry &rhs, QxdFile &qxd) const
{
    if (id != rhs.id)
        return false;

    if (!ComparePartial(rhs))
        return false;

    QxdCharacter *chara1, *chara2;
    bool special1 = false, special2 = false;

    chara1 = qxd.FindCharById(qxd_id);
    if (!chara1)
    {
        special1 = true;
        chara1 = qxd.FindSpecialCharById(qxd_id);
        if (!chara1)
            return false;
    }

    chara2 = qxd.FindCharById(rhs.qxd_id);
    if (!chara2)
    {
        special2 = true;
        chara2 = qxd.FindSpecialCharById(rhs.qxd_id);
        if (!chara2)
            return false;
    }

    if (special1 != special2)
        return false;

    return chara1->ComparePartial(*chara2);
}
