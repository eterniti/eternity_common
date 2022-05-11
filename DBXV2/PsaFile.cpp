#include <math.h>

#include "PsaFile.h"
#include "debug.h"

PsaFile::PsaFile()
{
    this->big_endian = false;
}

PsaFile::~PsaFile()
{

}

void PsaFile::Reset()
{
    attributes.clear();
}

bool PsaFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(PSAHeader))
        return false;

    const PSAHeader *hdr = (const PSAHeader *)buf;
    if (hdr->signature != PSA_SIGNATURE)
        return false;

    attributes.resize(hdr->num_attributes+1);
    const PSAAttributes *file_attrs = (const PSAAttributes *)GetOffsetPtr(hdr, hdr->header_size+(uint32_t)attributes.size()*8, true);

    for (size_t i = 0; i < attributes.size(); i++)
    {
        PsaAttributes &attr = attributes[i];

        attr.hea = file_attrs[i].hea;
        attr.ki = file_attrs[i].ki;
        attr.sta = file_attrs[i].sta;
        attr.atk = file_attrs[i].atk;
        attr.str = file_attrs[i].str;
        attr.bla = file_attrs[i].bla;
    }

    return true;
}

float PsaFile::GetHea(uint16_t points, bool *error) const
{
    if (points >= attributes.size())
    {
        if (error)
            *error = true;

        return 0.0f;
    }

    if (error)
        *error = false;

    return attributes[points].hea;
}

float PsaFile::GetKi(uint16_t points, bool *error) const
{
    if (points >= attributes.size())
    {
        if (error)
            *error = true;

        return 0.0f;
    }

    if (error)
        *error = false;

    return attributes[points].ki;
}

float PsaFile::GetSta(uint16_t points, bool *error) const
{
    if (points >= attributes.size())
    {
        if (error)
            *error = true;

        return 0.0f;
    }

    if (error)
        *error = false;

    return attributes[points].sta;
}

float PsaFile::GetAtk(uint16_t points, bool *error) const
{
    if (points >= attributes.size())
    {
        if (error)
            *error = true;

        return 0.0f;
    }

    if (error)
        *error = false;

    return attributes[points].atk;
}

float PsaFile::GetStr(uint16_t points, bool *error) const
{
    if (points >= attributes.size())
    {
        if (error)
            *error = true;

        return 0.0f;
    }

    if (error)
        *error = false;

    return attributes[points].str;
}

float PsaFile::GetBla(uint16_t points, bool *error) const
{
    if (points >= attributes.size())
    {
        if (error)
            *error = true;

        return 0.0f;
    }

    if (error)
        *error = false;

    return attributes[points].bla;
}

#define MY_EPSILON  0.001

bool CompareFloat(float a, float b)
{
    return fabs(a - b) < MY_EPSILON;
}

uint16_t PsaFile::GetHeaPoints(float hea) const
{
    for (uint16_t points = 0; points < (uint16_t) attributes.size(); points++)
    {
        if (CompareFloat(attributes[points].hea, hea))
            return points;
    }

    return 0xFFFF;
}

uint16_t PsaFile::GetKiPoints(float ki) const
{
    for (uint16_t points = 0; points < (uint16_t) attributes.size(); points++)
    {
        if (CompareFloat(attributes[points].ki, ki))
            return points;
    }

    return 0xFFFF;
}

uint16_t PsaFile::GetStaPoints(float sta) const
{
    for (uint16_t points = 0; points < (uint16_t) attributes.size(); points++)
    {
        if (CompareFloat(attributes[points].sta, sta))
            return points;
    }

    return 0xFFFF;
}

uint16_t PsaFile::GetAtkPoints(float atk) const
{
    for (uint16_t points = 0; points < (uint16_t) attributes.size(); points++)
    {
        if (CompareFloat(attributes[points].atk, atk))
            return points;
    }

    return 0xFFFF;
}

uint16_t PsaFile::GetStrPoints(float str) const
{
    for (uint16_t points = 0; points < (uint16_t) attributes.size(); points++)
    {
        if (CompareFloat(attributes[points].str, str))
            return points;
    }

    return 0xFFFF;
}

uint16_t PsaFile::GetBlaPoints(float bla) const
{
    for (uint16_t points = 0; points < (uint16_t) attributes.size(); points++)
    {
        if (CompareFloat(attributes[points].bla, bla))
            return points;
    }

    return 0xFFFF;
}




