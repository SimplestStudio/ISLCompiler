#include "islparser.h"
#include <sstream>
#ifdef _WIN32
# include "utils.h"
# include <Windows.h>
# include <cwctype>
# include <codecvt>
# define istalnum(c) std::iswalnum(c)
# define istalpha(c) std::iswalpha(c)
# define tistringstream std::wistringstream
#else
# include "utils.h"
# include <cctype>
# define istalnum(c) std::isalnum(c)
# define istalpha(c) std::isalpha(c)
# define tistringstream std::istringstream
#endif


static bool isSeparator(tchar c)
{
    return c == _T(' ') || c == _T('\t') || c == _T('\r') || c == _T('\n');
}

static bool isValidStringIdCharacter(tchar c)
{
    return istalnum(c) || istalpha(c) || c == _T('_');
}

static bool isValidLocaleCharacter(tchar c)
{
    return istalpha(c) || c == _T('_');
}

#ifdef _WIN32
static std::wstring Utf8ToWStr(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.from_bytes(str);
}

static std::string WStrToUtf8(const std::wstring &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(str);
}
#endif

ISLParser::ISLParser(const tstring &filePath) :
    is_translations_valid(false)
{
    std::string tr;
    if (NS_File::readFile(filePath, tr) && !tr.empty()) {
#ifdef _WIN32
        translations = Utf8ToWStr(tr);
#else
        translations = tr;
#endif
        parseTranslations();
        if (!is_translations_valid)
            NS_Logger::WriteLog(_T("Cannot parse translations, error in string: ") + error_substr + _T(" <---"));
    } else
        NS_Logger::WriteLog(_T("Error: translations is empty."));
}

ISLParser::~ISLParser()
{

}

bool ISLParser::translationToBin(const tstring &filePath, tstring &error)
{
    if (is_translations_valid) {
        if (translMap.empty()) {
            error = _T("Translations is empty!");
            return false;
        }
        if (!NS_File::writeBinFile(filePath, translMap)) {
            error = _T("Cannot write file!");
            return false;
        }
        return true;
    } else
        error = error_substr;
    return false;
}

bool ISLParser::binToTranslation(const tstring &filePath)
{
    std::string out;
    std::unordered_map<tstring, LocaleMap> translMap;
    NS_File::readBinFile(filePath, translMap);
    for (auto it = translMap.cbegin(); it != translMap.cend(); ++it) {
        tstring key = it->first;
        const LocaleMap &localeMap = it->second;
        for (auto it = localeMap.cbegin(); it != localeMap.cend(); ++it) {
            std::string val;
#ifdef _WIN32
            out.append(WStrToUtf8(it->first));
            out.append(".");
            out.append(WStrToUtf8(key));
            out.append(" =");
            val = WStrToUtf8(it->second);
#else
            out.append(it->first);
            out.append(".");
            out.append(key);
            out.append(" =");
            val = it->second;
#endif
            size_t p = val.find('\n');
            while (p != std::string::npos) {
                val.replace(p, 1, "\\n");
                p = val.find('\n', p + 2);
            }
            out.append(val);
            out.append("\n");
        }
        out.append("\n");
    }
    tstring outPath(filePath);
    outPath.append(_T(".isl"));
    return NS_File::writeFile(outPath, out);
}

void ISLParser::parseTranslations()
{
    int token = TOKEN_BEGIN_DOCUMENT;
    tstring stringId, currentLocale;
    size_t pos = 0, len = translations.length();
    while (pos < len) {
        size_t incr = 1;
        tchar ch = translations.at(pos);

        switch (token) {
        case TOKEN_BEGIN_DOCUMENT:
        case TOKEN_END_VALUE:
            if (!isSeparator(ch)) {
                if (ch == _T(';')) {
                    // string is comment
                    size_t end = translations.find_first_of(_T('\n'), pos);
                    incr = (end == tstring::npos) ? len - pos : end - pos + 1;
                } else {
                    size_t end;
                    for (end = pos; end < len; end++) {
                        tchar c = translations.at(end);
                        if (!isValidLocaleCharacter(c))
                            break;
                    }
                    size_t locale_len = end - pos;
                    if (locale_len < 12 && locale_len != 0 && locale_len != 1 && locale_len != 4 && locale_len != 9) {
                        token = TOKEN_BEGIN_LOCALE;
                        continue;
                    } else {
                        // TOKEN_ERROR
                        error_substr = translations.substr(0, pos + 1);
                        return;
                    }
                }
            }
            break;

        case TOKEN_BEGIN_STRING_ID:
            if (!isSeparator(ch)) {
                size_t end;
                tchar c;
                for (end = pos; end < len; end++) {
                    c = translations.at(end);
                    if (!isValidStringIdCharacter(c))
                        break;
                }
                c = translations.at(end);
                if (end < len && !isSeparator(c) && c != _T('=')) {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, end + 1);
                    return;
                }
                stringId = translations.substr(pos, end - pos);
                if (!stringId.empty() && translMap.find(stringId) == translMap.end())
                    translMap[stringId] = LocaleMap();

                token = TOKEN_END_STRING_ID;
                incr = end - pos;
            }
            break;

        case TOKEN_END_STRING_ID:
            if (!isSeparator(ch)) {
                if (ch == _T('=')) {
                    token = TOKEN_BEGIN_VALUE;
                } else {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, pos + 1);
                    return;
                }
            }
            break;

        case TOKEN_BEGIN_LOCALE: {
            size_t end;
            for (end = pos; end < len; end++) {
                tchar c = translations.at(end);
                if (!isValidLocaleCharacter(c))
                    break;
            }
            size_t locale_len = end - pos;
            currentLocale = translations.substr(pos, locale_len);
            if (pos + locale_len == len) {
                error_substr = translations.substr(0, pos + locale_len);
                return;
            }
            token = TOKEN_END_LOCALE;
            incr = locale_len;
            break;
        }

        case TOKEN_END_LOCALE:
            if (!isSeparator(ch)) {
                if (ch == _T('.')) {
                    token = TOKEN_BEGIN_STRING_ID;
                } else {
                    // TOKEN_ERROR
                    error_substr = translations.substr(0, pos + 1);
                    return;
                }
            }
            break;

        case TOKEN_BEGIN_VALUE: {
            size_t end = translations.find_first_of(_T('\n'), pos);
            tstring val;
            if (end == tstring::npos) {
                val = translations.substr(pos);
                incr = len - pos;
            } else {
                val = translations.substr(pos, end - pos);
                incr = end - pos;
            }

            if (!val.empty() && val.back() == _T('\r'))
                val.pop_back();

            size_t p = val.find(_T("\\n"));
            while (p != std::string::npos) {
                val.replace(p, 2, _T("\\"));
                val[p] = _T('\n');
                p = val.find(_T("\\n"), p + 1);
            }
            if (!currentLocale.empty() && translMap.find(stringId) != translMap.end())
                translMap[stringId][currentLocale] = val;

            token = TOKEN_END_VALUE;
            break;
        }

        default:
            break;
        }
        pos += incr;
        if (pos == len)
            token = TOKEN_END_DOCUMENT;
    }

    if (token == TOKEN_END_DOCUMENT)
        is_translations_valid = true;
}
