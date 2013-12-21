/*
 * UTF8 String class.
 */

#ifndef IPHONESMSEXPORT_STRINGUTF8_H
#define IPHONESMSEXPORT_STRINGUTF8_H

#include <algorithm>
#include <stdint.h>
#include <vector>

class StringUtf8
{
protected:
    typedef StringUtf8 MyType;

private:
    std::vector<uint8_t> m_data;

public:
    /** Constructor. */
    StringUtf8()
    {}

    /** Constructor. */
    explicit StringUtf8(const char *str)
    {
        FromByteStream((const uint8_t *)str, strlen(str));
    }

    /** Constructor. */
    StringUtf8(const uint8_t *byteStream, size_t length)
    {
        FromByteStream(byteStream, length);
    }

    /** Assignment operator. */
    MyType& operator=(const MyType& str)
    {
        m_data = str.m_data;
        return *this;
    }

    /** Create string from a byte stream. */
    MyType& FromByteStream(const uint8_t *byteStream, size_t length)
    {
        m_data.resize(length);
        memcpy(&m_data[0], byteStream, length);
        return *this;
    }

    /** From C string. */
    MyType& FromString(const char *str)
    {
        return FromByteStream((const uint8_t *)str, strlen(str));
    }

    /** String length. */
    size_t Length() const
    {
        return m_data.size();
    }

    /** CHeck whether the string is empty. */
    bool IsEmpty() const
    {
        return (Length() == 0);
    }

    /** Subscript operator. */
    const uint8_t& operator[](size_t pos) const
    {
        return m_data[pos];
    }

    /** Subscript operator. */
    uint8_t& operator[](size_t pos)
    {
        return m_data[pos];
    }

    /** Naive lexicographical comparison. */
    bool operator<(const MyType& str) const
    {
        size_t len = (Length() < str.Length()) ? Length() : str.Length();

        for (size_t i = 0; i < len; ++i)
        {
            if (m_data[i] < str.m_data[i])
            {
                return true;
            }
            
            if (m_data[i] > str.m_data[i])
            {
                return false;
            }
        }

        return Length() < str.Length();
    }

    /** Remove spaces. */
    MyType& StripSpaces()
    {
        std::vector<uint8_t>::iterator ch = m_data.begin();

        while (ch != m_data.end())
        {
            if (*ch == ' ')
            {
                ch = m_data.erase(ch);
            }
            else
            {
                ++ch;
            }
        }

        return *this;
    }

    /** Concatenate two strings. */
    MyType& operator+=(const MyType& str)
    {
        m_data.insert(m_data.end(), str.m_data.begin(), str.m_data.end());
        return *this;
    }
};

#endif