#include "StdAfx.h"
#include "logWriter.h"


CString strLogINIPath = ".\\LogWriter.ini";
CString strLogRoot;

void GetLogIni()
{
	char buf[1024] = {0,};
	GetPrivateProfileString("System", "LogRoot", ".\\", buf, sizeof(buf), strLogINIPath);
	strLogRoot.Format("%s", buf);
	strLogRoot.TrimRight("\\");
}

void U_Log(char* pLogDir, char* pLogBuff, ...)
{

	char  szLogFile[256];
    char  szLogDir [256];
	char  szLogPath[256];

	SYSTEMTIME st;
	GetLocalTime(&st);
	GetLogIni(); 
	sprintf_s(szLogDir, "%s\\%s\\M%02d\\", strLogRoot, pLogDir, st.wMonth);

	if(PathIsDirectory(szLogDir) == false)
	{
		CreateFullDirectory(szLogDir);
	}

	sprintf_s(szLogFile, "%4d%02d%02d.log", st.wYear, st.wMonth, st.wDay);
	sprintf_s(szLogPath, "%s%s", szLogDir, szLogFile);

	HANDLE hFile;
    for( int i = 0; i < 10; i++ )
    {
        hFile = CreateFile(
			szLogPath                       ,
			GENERIC_READ|GENERIC_WRITE      ,
			FILE_SHARE_READ                 , //FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL                            ,
			OPEN_ALWAYS                     ,
			FILE_ATTRIBUTE_ARCHIVE          ,
			NULL                            );

		if( INVALID_HANDLE_VALUE == hFile )
        {
            CloseHandle(hFile);
            Sleep(10);

            continue;
        }
        break;
    }

	if( hFile == INVALID_HANDLE_VALUE )
    {
        return;
    }

	/******************************************************************************/
	/*                                                                            */
	/*                                                                            */
	/*                                                                            */
	/******************************************************************************/
	try{
		char LogData[4096];

		memset(LogData, 0x00, sizeof(LogData));

		sprintf(LogData, "[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		char* argp = (char*)&pLogBuff + sizeof(pLogBuff);
		vsprintf(LogData+strlen(LogData), pLogBuff, argp);

		strcat(LogData, "\x0d\x0a");

		ULONG Bytes;
		ULONG NewPointer = 0;

		SetFilePointer(hFile, NewPointer, NULL, FILE_END);
		WriteFile(hFile, LogData, strlen(LogData), &Bytes, NULL);
	}catch (DWORD err){
	}
	/******************************************************************************/
	/*                                                                            */
	/*                                                                            */
	/*                                                                            */
	/******************************************************************************/
    CloseHandle(hFile);
}


void U_SubLog(char* pLogDir, char* pLogSubDir, char* pLogBuff, ...)
{
	char  szLogFile[256];
    char  szLogDir [256];
	char  szLogPath[256];

	SYSTEMTIME st;
	GetLocalTime(&st);
	GetLogIni(); 

	memset(szLogPath,0x00,sizeof(szLogPath));

	sprintf(szLogDir, "%s\\%s\\M%02d\\%s\\", strLogRoot, pLogDir, st.wMonth, pLogSubDir);

	if(PathIsDirectory(szLogDir) == false)
	{
		CreateFullDirectory(szLogDir);
	}

	sprintf(szLogFile, "%4d%02d%02d.log", st.wYear, st.wMonth, st.wDay);
	sprintf(szLogPath, "%s%s", szLogDir, szLogFile);

	HANDLE hFile;
    for( int i = 0; i < 10; i++ )
    {
        hFile = CreateFile(
			szLogPath                       ,
			GENERIC_READ|GENERIC_WRITE      ,
			FILE_SHARE_READ                 , //FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL                            ,
			OPEN_ALWAYS                     ,
			FILE_ATTRIBUTE_ARCHIVE          ,
			NULL                            );

		if( INVALID_HANDLE_VALUE == hFile )
        {
            CloseHandle(hFile);
            Sleep(10);

            continue;
        }
        break;
    }

	if( hFile == INVALID_HANDLE_VALUE )
    {
        return;
    }

	/******************************************************************************/
	/*                                                                            */
	/*                                                                            */
	/*                                                                            */
	/******************************************************************************/
	char LogData[5000];

    memset(LogData, 0x00, sizeof(LogData));

	sprintf(LogData, "[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    char* argp = (char*)&pLogBuff + sizeof(pLogBuff);
    vsprintf(LogData+strlen(LogData), pLogBuff, argp);

    strcat(LogData, "\x0d\x0a");

    ULONG Bytes;
    ULONG NewPointer = 0;

	SetFilePointer(hFile, NewPointer, NULL, FILE_END);
    WriteFile(hFile, LogData, strlen(LogData), &Bytes, NULL);
	/******************************************************************************/
	/*                                                                            */
	/*                                                                            */
	/*                                                                            */
	/******************************************************************************/
    CloseHandle(hFile);
}



void CreateFullDirectory(char* strPath)
{
	int nPathLen = strlen(strPath);

	int nStartIndex = 0;

	for(int i=0; i<nPathLen; i++)
	{
		if(strPath[i] == '\\')
		{
			nStartIndex = i;
			break;
		}
	}

	for(int i=nStartIndex; i<nPathLen; i++)
	{
		if(strPath[i] == '\\')
		{
			strPath[i] = 0;
			
			if(PathIsDirectory(strPath) == false)
				CreateDirectory(strPath, 0);

			strPath[i] = '\\';
		}
	}
}