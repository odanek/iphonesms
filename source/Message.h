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
    public:
        sqlite3_int64 m_rowId;
        StringUtf8 m_address;
        sqlite3_int64 m_date;
        StringUtf8 m_text;
        sqlite3_int64 m_flags;
        StringUtf8 m_madridHandle;
        sqlite3_int64 m_madridFlags;
        sqlite3_int64 m_madridError;
        sqlite3_int64 m_isMadrid;
        sqlite3_int64 m_madridDateRead;
        sqlite3_int64 m_madridDateDelivered;

    public:
        /** Constructor. */
        Message()
        {}

        /** Check if message is valid. */
        bool IsValid() const
        {
            if (Address().IsEmpty())
            {
                printf("Skipping message rowId=%d. Reason: empty address, probably iMessage group chat.\n", m_rowId);
                return false;
            }

            if (m_isMadrid)
            {
                if (m_madridError)
                {
                    printf("Skipping message rowId=%d. Reason: iMessage error is nonzero.\n", m_rowId);
                    return false;
                }

                if (m_madridFlags == 32773 || m_madridFlags == 98309)
                {
                    printf("Skipping message rowId=%d. Reason: iMessage group chat.\n", m_rowId);
                    return false;
                }

                if (m_madridFlags != 36869 && m_madridFlags != 102405 && m_madridFlags != 12289 && m_madridFlags != 77825)
                {
                    printf("Skipping message rowId=%d. Reason: unknown iMessage flags.\n", m_rowId);
                    return false;
                }
            }

            return true;
        }

        /** Check whether messsage is sent or received. */
        bool IsReceived() const
        {
            if (m_isMadrid)
            {
                return (m_madridFlags == 12289 || m_madridFlags == 77825);
            }

            return (m_flags & 1) == 0;
        }

        /** Get message timestamp in unix time. */
        time_t Timestamp() const
        {
            if (m_isMadrid)
            {
                // Choose date field, only one should be nonzero
                sqlite3_int64 msgDate = (m_madridDateRead == 0) ? m_madridDateDelivered : m_madridDateRead;
                // Convert iMessage timestamp to unix time
                return (time_t)(msgDate + 978307200);
            }

            return (time_t)m_date;
        }

        /** Address. */
        const StringUtf8& Address() const
        {
            return m_isMadrid ? m_madridHandle : m_address;
        }

        /** Message text. */
        const StringUtf8& Text() const
        {
            return m_text;
        }
    };
}

#endif