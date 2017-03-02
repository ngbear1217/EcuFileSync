#pragma once

DWORD AfxReadFile(const char* strFileName , BYTE** pBuf);

void GetLogIni();
void CreateFullDirectory(char* strPath);
void inet_ntoa(char* pOutIP, unsigned int nInIP);

void U_Log  (char* pLogDir, char* pLogBuff, ...);
void U_Log_Raw  (char* pLogDir, char* pLogStr, char* pLogBuff, int nLength);
void U_SubLog(char* pLogDir, char* pLogSubDir, char* pLogBuff, ...);
void U_SubLog_Raw(char* pLogDir, char* pLogSubDir, char* pLogStr, char* pLogBuff, int nLength);
