
#include "utils.h"
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#ifdef _WIN32
# include <Windows.h>
# include <shlwapi.h>
# include <algorithm>
# include <codecvt>
  typedef std::wstringstream tstringstream;
  typedef std::wofstream tofstream;
#else
# include <dirent.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <cstdint>
  typedef std::stringstream tstringstream;
  typedef std::ofstream tofstream;
  typedef unsigned short WORD;
#endif

static const char ISL_MAGIC[] = "ISL";


static tstring Utf8ToTStr(const std::string &str)
{
#ifdef _WIN32
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.from_bytes(str);
#else
    return str;
#endif
}

namespace NS_Args
{
    std::vector<tstring> cmd_args;

    void parseCmdArgs(int argc, tchar *argv[])
    {
        for (int i = 0; i < argc; i++)
            cmd_args.push_back(argv[i]);
    }

    bool cmdArgContains(const tstring &param)
    {
        auto len = param.length();
        return std::any_of(cmd_args.cbegin(), cmd_args.cend(), [&param, len](const tstring &arg) {
            return arg.find(param) == 0 && (len == arg.length() || arg[len] == '=' || arg[len] == ':' || arg[len] == '|');
        });
    }

    tstring cmdArgValue(const tstring &param)
    {
        auto len = param.length();
        for (const auto &arg : cmd_args) {
            if (arg.find(param) == 0 && len < arg.length() && (arg[len] == '=' || arg[len] == ':' || arg[len] == '|'))
                return arg.substr(len + 1);
        }
        return _T("");
    }
}

namespace NS_File
{
#ifdef __linux__
    tstring app_path;

    void setAppPath(const std::string &path)
    {
        app_path = parentPath(path);
    }
#endif

    bool readFile(const tstring &filePath, std::string &str)
    {
        std::ifstream file(filePath.c_str(), std::ios_base::in);
        if (!file.is_open()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return false;
        }
        std::stringstream stream;
        stream << file.rdbuf();
        if (file.fail() || stream.fail()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            file.close();
            return false;
        }
        str = stream.str();
        file.close();
        return true;
    }

    bool writeFile(const tstring &filePath, std::string &str)
    {
        std::ofstream file(filePath.c_str(), std::ios_base::out);
        if (!file.is_open()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return false;
        }
        file.write(&str[0], str.length());
        if (file.fail()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            file.close();
            return false;
        }
        file.close();
        return true;
    }

    bool readBinFile(const tstring &filePath, std::unordered_map<tstring, LocaleMap> &translMap)
    {
        std::ifstream file(filePath, std::ios_base::out | std::ios::binary);
        if (!file.is_open()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return false;
        }

        char magic[sizeof(ISL_MAGIC)] = { 0 };
        file.read(magic, sizeof(magic));
        if (file.fail() || strncmp(magic, ISL_MAGIC, sizeof(magic) - 1) != 0) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            file.close();
            return false;
        }

        WORD mapSize = 0;
        file.read((char*)&mapSize, sizeof(mapSize));
        if (file.fail()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            file.close();
            return false;
        }

        for (WORD i = 0; i < mapSize; i++) {
            uint8_t len = 0;
            file.read((char*)&len, sizeof(len));
            if (file.fail()) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                file.close();
                return false;
            }

            std::string key;
            key.resize(len);
            file.read(&key[0], len);
            if (file.fail()) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                file.close();
                return false;
            }

            WORD localeSize = 0;
            file.read((char*)&localeSize, sizeof(localeSize));
            if (file.fail()) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                file.close();
                return false;
            }

            LocaleMap localeMap;
            for (WORD j = 0; j < localeSize; j++) {
                uint8_t len = 0;
                file.read((char*)&len, sizeof(len));
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }

                std::string str;
                str.resize(len);
                file.read(&str[0], len);
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }

                WORD len2 = 0;
                file.read((char*)&len2, sizeof(len2));
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }

                std::string str2;
                str2.resize(len2);
                file.read(&str2[0], len2);
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }
                localeMap[Utf8ToTStr(str)] = Utf8ToTStr(str2);
            }
            translMap[Utf8ToTStr(key)] = localeMap;
        }
        file.close();
        return true;
    }

    bool writeBinFile(const tstring &filePath, const std::unordered_map<tstring, LocaleMap> &translMap)
    {
        std::ofstream file(filePath, std::ios_base::out | std::ios::binary);
        if (!file.is_open()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return false;
        }
        file.write(ISL_MAGIC, sizeof(ISL_MAGIC));
        if (file.fail()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            file.close();
            return false;
        }
        WORD mapSize = translMap.size();
        file.write((char*)&mapSize, sizeof(mapSize));
        if (file.fail()) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            file.close();
            return false;
        }
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
#endif
        for (auto it = translMap.cbegin(); it != translMap.cend(); ++it) {
#ifdef _WIN32
            std::string key = utf8_conv.to_bytes(it->first);
#else
            std::string key = it->first;
#endif
            uint8_t len = key.length();
            file.write((char*)&len, sizeof(len));
            if (file.fail()) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                file.close();
                return false;
            }
            file.write((char*)key.c_str(), key.length());
            if (file.fail()) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                file.close();
                return false;
            }

            const LocaleMap &localeMap = it->second;
            WORD localeSize = localeMap.size();
            file.write((char*)&localeSize, sizeof(localeSize));
            if (file.fail()) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                file.close();
                return false;
            }
            for (auto it = localeMap.cbegin(); it != localeMap.cend(); ++it) {
#ifdef _WIN32
                std::string str[2] = {utf8_conv.to_bytes(it->first), utf8_conv.to_bytes(it->second)};
#else
                std::string str[2] = {it->first, it->second};
#endif
                uint8_t len = str[0].length();
                file.write((char*)&len, sizeof(len));
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }
                file.write((char*)str[0].c_str(), str[0].length());
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }

                WORD len2 = str[1].length();
                file.write((char*)&len2, sizeof(len2));
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }
                file.write((char*)str[1].c_str(), str[1].length());
                if (file.fail()) {
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
                    file.close();
                    return false;
                }
            }
        }
        file.close();
        return true;
    }

    bool fileExists(const tstring &filePath)
    {
#ifdef _WIN32
        DWORD attr = ::GetFileAttributes(filePath.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
#else
        struct stat st;
        if (stat(filePath.c_str(), &st) != 0)
            return false;
        return S_ISREG(st.st_mode);
#endif
    }

    std::vector<tstring> getFilesWithExtension(const tstring &folderPath, const tstring &ext)
    {
        std::vector<tstring> files;
        if (folderPath.empty())
            return files;
        tstring outPath(folderPath);
#ifdef _WIN32
        std::replace(outPath.begin(), outPath.end(), L'/', L'\\');
        if (outPath.back() != L'\\')
            outPath.push_back(L'\\');
        tstring searchPath(outPath);
        searchPath.push_back(L'*');
        searchPath.append(ext);

        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(searchPath.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            return files;

        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                files.push_back(outPath + fd.cFileName);
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
#else
        DIR *dir = opendir(folderPath.c_str());
        if (!dir) return files;

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            tstring name = entry->d_name;
            if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
                auto pos = name.find_last_of('.');
                if (pos != tstring::npos && name.substr(pos) == ext)
                    files.push_back(folderPath + "/" + name);
            }
        }
        closedir(dir);
#endif
        return files;
    }

#ifdef _WIN32
    tstring fromNativeSeparators(const tstring &path)
    {
        tstring _path(path);
        std::replace(_path.begin(), _path.end(), L'\\', L'/');
        return _path;
    }

    tstring toNativeSeparators(const tstring &path)
    {
        tstring _path(path);
        std::replace(_path.begin(), _path.end(), L'/', L'\\');
        return _path;
    }
#endif

    tstring parentPath(const tstring &path)
    {
        size_t len = path.length();
        if (len > 1) {
            const tchar *buf = path.c_str();
            const tchar *it = buf + len - 1;
            while (*it == '/'
#ifdef _WIN32
                   || *it == '\\'
#endif
                   ) {
                if (it == buf)
                    return _T("");
                it--;
            }
            while (*it != '/'
#ifdef _WIN32
                   && *it != '\\'
#endif
                   ) {
                if (it == buf)
                    return _T("");
                it--;
            }
            if (it == buf)
#ifdef _WIN32
                return _T("");
#else
                return _T("/");
#endif
            return tstring(buf, it - buf);
        }
        return _T("");
    }

    tstring appPath()
    {
#ifdef _WIN32
        WCHAR buff[MAX_PATH];
        DWORD res = ::GetModuleFileName(NULL, buff, MAX_PATH);
        return (res != 0) ? fromNativeSeparators(parentPath(buff)) : L"";
#else
        return app_path;
#endif
    }
}

namespace NS_Logger
{
    bool allow_write_log = false;

    void AllowWriteLog()
    {
        allow_write_log = true;
    }

    void WriteLog(const tstring &log, bool showMessage)
    {
        if (allow_write_log) {
            tstring filpPath(NS_File::appPath() + _T("/log.txt"));
            tofstream file(filpPath.c_str(), std::ios::app);
            if (!file.is_open()) {
                return;
            }
            file << log << std::endl;
            file.close();
        }
    }
}
