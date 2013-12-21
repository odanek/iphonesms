/*
 * Message container.
 */

#ifndef IPHONESMSEXPORT_MESSAGE_H
#define IPHONESMSEXPORT_MESSAGE_H

#include "StringUtf8.h"

namespace iPhoneSmsExport
{
    class Message
    {
    private:
        static StringUtf8 ms_unknown;

    public:
        sqlite3_int64 m_rowId;
        StringUtf8 m_text;
        sqlite3_int64 m_date;
        StringUtf8 m_handle;
        sqlite3_int64 m_isFromMe;

    public:
        /** Constructor. */
        Message()
        {}

        /** Check if message is valid. */
        bool IsValid() const
        {
            return true;
        }

        /** Check whether messsage is sent or received. */
        bool IsReceived() const
        {
            return !m_isFromMe;
        }

        /** Get message timestamp in unix time. */
        time_t Timestamp() const
        {
            return (time_t)(m_date + 978307200);
        }

        /** Address. */
        const StringUtf8& Address() const
        {
            return m_handle.IsEmpty() ? ms_unknown : m_handle;
        }

        /** Message text. */
        const StringUtf8& Text() const
        {
            return m_text;
        }
    };
}

#endif