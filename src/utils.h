
#ifndef UTILS_H
#define UTILS_H

#include "islparser.h"
#ifdef _WIN32
# define to_tstring std::to_wstring
#else
# define to_tstring std::to_string
#endif

#define FUNCTION_INFO tstring(_T(__FUNCTION__)) + _T(" Line: ") + to_tstring(__LINE__)
#define DEFAULT_ERROR_MESSAGE tstring(_T("An error occurred: ")) + FUNCTION_INFO
#define ADVANCED_ERROR_MESSAGE DEFAULT_ERROR_MESSAGE + _T(" ") + NS_Utils::GetLastErrorAsString()

namespace NS_Args
{
void parseCmdArgs(int argc, tchar *argv[]);
bool cmdArgContains(const tstring &param);
tstring cmdArgValue(const tstring &param);
}

namespace NS_File
{
#ifdef __linux__
void setAppPath(const tstring &path);
#endif
bool readFile(const tstring &filePath, std::string &str);
bool writeFile(const tstring &filePath, std::string &str);
bool readBinFile(const tstring &filePath, std::unordered_map<tstring, LocaleMap> &translMap);
bool writeBinFile(const tstring &filePath, const std::unordered_map<tstring, LocaleMap> &translMap);
bool fileExists(const tstring &filePath);
#ifdef _WIN32
tstring fromNativeSeparators(const tstring &path);
tstring toNativeSeparators(const tstring &path);
#endif
tstring parentPath(const tstring &path);
tstring appPath();
}

namespace NS_Logger
{
void AllowWriteLog();
void WriteLog(const tstring &log, bool showMessage = false);
}

#endif // UTILS_H
