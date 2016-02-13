// Warning: Quick and dirty code using a lot of nasty windows-specific stuff.

#include <Windows.h>
#include <Shlobj.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <time.h>
#include "sqlite3.h"
#include "StringUtf8.h"
#include "Message.h"

namespace iPhoneSmsExport
{
    /*******************************************************************************/

    // Static member declaration
    StringUtf8 Message::ms_unknown("Unknown");

    /*******************************************************************************/

    // Name of the file containing backup of SMS database
    const std::wstring sms_file(L"\\3d0d7e5fb2ce288813306e4d4636395e047a3d28");

    /*******************************************************************************/

    // Backup structure
    struct Backup
    {
        std::wstring m_dir;
        SYSTEMTIME m_date;
    };

    // Message comparison operator
    static bool msg_comp(const Message &a, const Message &b)
    {
        // For some reason timestamps are sometimes incorrect - ignore them and use 
        // stable sort using phone numbers only (messages are already sorted according
        // to rowid)
        return a.Address() < b.Address();
    }

    /*******************************************************************************/

    static std::wstring RoamingProfilePath()
    {
        // Get windows roaming profile folder path - updated version, should work in XP as well
        TCHAR path[MAX_PATH];

        if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path) != S_OK)
        {
            MessageBox(NULL, L"Retrieving user roaming profile directory failed.", L"Error", MB_OK);
            exit(1);
        }

        // Copy path to string
        std::wstring tp(path);

        // Return path
        return tp;
    }

    /*******************************************************************************/

    static void GetBackupList(const std::wstring &dir, std::vector<Backup> &bu)
    {    
        WIN32_FIND_DATA fdFile; 
        HANDLE hFind = NULL; 

        if ((hFind = FindFirstFile((dir + L"*.*").c_str(), &fdFile)) == INVALID_HANDLE_VALUE) 
        { 
            std::wstringstream msg;
            msg << "Unable to list contents of directory:\n" << dir;
            MessageBox(NULL, msg.str().c_str(), L"Error", MB_OK);
            exit(1);
        } 

        do
        {
            if ((fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && fdFile.cFileName[0] != '.')
            {            
                HANDLE hFl = CreateFile((dir + fdFile.cFileName + sms_file).c_str(), 
                    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFl != INVALID_HANDLE_VALUE)
                {
                    FILETIME ct, at, wt;                

                    if (GetFileTime(hFl, &ct, &at, &wt))
                    {
                        Backup nbu;
                        SYSTEMTIME stUTC;

                        FileTimeToSystemTime(&wt, &stUTC);
                        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &nbu.m_date);
                    
                        nbu.m_dir = std::wstring(fdFile.cFileName);
                        bu.push_back(nbu);
                    }
                }
            }
        } 
        while (FindNextFile(hFind, &fdFile));

        FindClose(hFind);
    }

    /*******************************************************************************/

    static size_t ChooseBackup(const std::vector<Backup> &bu)
    {
        if (!bu.size())
        {
            MessageBox(NULL, L"No iPhone backups found.", L"Error", MB_OK);
            exit(1);
        }
        else if (bu.size() == 1)
        {
            std::wcout << "Exporting backup: " << bu[0].m_dir << std::endl;
            return 0;
        }

        // Choose a backup
        std::wcout << "List of available iPhone backups:" << std::endl;

        for (size_t i = 0; i < bu.size(); i++)
        {
            std::wcout << (i + 1) << ". Modified: " << bu[i].m_date.wDay << "." << bu[i].m_date.wMonth << "." << bu[i].m_date.wYear;
            std::wcout << " Directory: " << bu[i].m_dir << " " << std::endl;
        }

        std::wcout << std::endl << "Enter number of the backup to be exported: ";
        int bidx;
        std::wcin >> bidx;

        if (bidx < 1 || bidx > (int)bu.size())
        {
            MessageBox(NULL, L"Invalid backup number.", L"Error", MB_OK);
            exit(1);
        }

        return bidx - 1;
    }

    /*******************************************************************************/

    static bool Sqlite_GetString(sqlite3_stmt *stm, int col, StringUtf8 &str)
    {
        const unsigned char *sa = sqlite3_column_text(stm, col);

        if (sa == NULL)
        {
            return false;
        }

        str.FromString((const char *)sa); 
        return true;
    }

    /*******************************************************************************/

    static void ReadSmsDatabase(const std::wstring &fname, std::vector<Message> &msg)
    {
        sqlite3 *db;
        char *zErrMsg = 0;
        int rc;

        rc = sqlite3_open16(fname.c_str(), &db);
        if (rc)
        {
            std::wstringstream msg;
            msg << "Unable to open SMS database:\n" << fname << "\nSQLite error code: " << (const wchar_t *)sqlite3_errmsg16(db);
            MessageBox(NULL, msg.str().c_str(), L"Error", MB_OK);
            sqlite3_close(db);
            exit(1);
        }
        else
        {
            sqlite3_stmt *stm;

            // SQL Select
            int err = sqlite3_prepare_v2(db, 
                "SELECT message.rowid, message.text, message.date, message.is_from_me, handle.id "
                "FROM message "
                "JOIN chat_message_join ON message.rowid = chat_message_join.message_id " 
                "JOIN chat_handle_join ON chat_message_join.chat_id = chat_handle_join.chat_id "
                "JOIN handle ON chat_handle_join.handle_id = handle.rowid "
                "ORDER BY message.rowid", 
                -1, &stm, NULL);

            if (err == SQLITE_OK)
            {
                while (sqlite3_step(stm) == SQLITE_ROW)
                {
                    Message m;

                    // Read message data
                    m.m_rowId = sqlite3_column_int64(stm, 0);                                        
                    Sqlite_GetString(stm, 1, m.m_text);
                    m.m_text.RemoveObjChar();
                    m.m_date = sqlite3_column_int64(stm, 2);
                    m.m_isFromMe = sqlite3_column_int64(stm, 3);
                    Sqlite_GetString(stm, 4, m.m_handle);
                    m.m_handle.StripSpaces();

                    msg.push_back(m);
                }

                sqlite3_finalize(stm);
            }
            else
            {
                std::wstringstream msg;
                msg << "SQL statement preparation failed. \nSQLite error code: " << err;
                MessageBox(NULL, msg.str().c_str(), L"Error", MB_OK);
                sqlite3_close(db);
                exit(1);
            }
        }

        sqlite3_close(db);
    }

    /*******************************************************************************/

    static void ExportWriteAddress(FILE *fl, const StringUtf8 &address, StringUtf8 &lastAddress)
    {
        if (ftell(fl) != 0 && (address < lastAddress || lastAddress < address))
        {
            fwrite("\r\n", 1, 2, fl);
        }
        lastAddress = address;

        fwrite(&address[0], 1, address.Length(), fl);
        fwrite("\t", 1, 1, fl);
    }

    /*******************************************************************************/

    static void ExportMessages(const char *fileName, const std::vector<Message> &messageList)
    {    
        FILE *f = fopen(fileName, "wb");

        if (f == NULL)
        {
            std::wstringstream msg;
            msg << "Unable to create output file: " << fileName;
            MessageBox(NULL, msg.str().c_str(), L"Error", MB_OK);
            exit(1);
        }

        StringUtf8 lastAddress;
		for (const Message& msg : messageList)
        {
            if (!msg.IsValid())
            {
                continue;
            }

            // Address
            ExportWriteAddress(f, msg.Address(), lastAddress);

            // Sent/received
            fwrite(msg.IsReceived() ? "R\t" : "S\t", 1, 2, f);

            // Timestamp
            char tbuf[50];
            time_t msgUnixTime = msg.Timestamp();
            const struct tm *ts = localtime(&msgUnixTime);
            sprintf(tbuf, "%02d.%02d.%d %02d:%02d\t", ts->tm_mday, ts->tm_mon + 1,
                ts->tm_year + 1900, ts->tm_hour, ts->tm_min);
            fwrite(tbuf, 1, strlen(tbuf), f);

            // Message
			if (!msg.Text().IsEmpty())
			{
				fwrite(&msg.Text()[0], 1, msg.Text().Length(), f);
			}            

            // New line
            fwrite("\r\n", 1, 2, f);
        }

        fclose(f);
    }
}

/*******************************************************************************/

int main(int argc, char **argv)
{
    // Get iPhone backups folder
    std::wstring bu_path = iPhoneSmsExport::RoamingProfilePath() + L"\\Apple Computer\\MobileSync\\Backup\\";
    
    // Get list of iPhone backups and choose one
    std::vector<iPhoneSmsExport::Backup> bu;
    iPhoneSmsExport::GetBackupList(bu_path, bu);
    size_t bu_idx = iPhoneSmsExport::ChooseBackup(bu);

    // Read messages
    std::vector<iPhoneSmsExport::Message> msg;
    iPhoneSmsExport::ReadSmsDatabase(bu_path + bu[bu_idx].m_dir + iPhoneSmsExport::sms_file, msg);

    // Sort messages
    std::stable_sort(msg.begin(), msg.end(), iPhoneSmsExport::msg_comp);

    // Export data (UTF-8)
    iPhoneSmsExport::ExportMessages("sms.txt", msg);

    MessageBox(NULL, L"Export successful.", L"Message", MB_OK);

    return 0;
}

/*******************************************************************************/