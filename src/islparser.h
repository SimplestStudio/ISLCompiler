#ifndef ISLPARSER_H
#define ISLPARSER_H

#include <unordered_map>
#include <string>
#ifdef _WIN32
# include <tchar.h>
# define tchar wchar_t
  typedef std::wstring tstring;
#else
# define _T(str) str
# define tchar char
  typedef std::string tstring;
#endif

using std::unordered_map;

typedef unordered_map<tstring, tstring> LocaleMap;
typedef unordered_map<tstring, LocaleMap> TranslationsMap;


class ISLParser
{
public:
    ISLParser(const tstring &filePath);
    ~ISLParser();

    bool translationToBin(const tstring &filePath, tstring &error);
    static bool binToTranslation(const tstring &filePath);

private:
    void parseTranslations();

    TranslationsMap translMap;
    tstring  translations,
             error_substr;
    bool     is_translations_valid;

    enum TokenType {
        TOKEN_BEGIN_DOCUMENT = 0,
        TOKEN_END_DOCUMENT,
        TOKEN_BEGIN_STRING_ID,
        TOKEN_END_STRING_ID,
        TOKEN_BEGIN_LOCALE,
        TOKEN_END_LOCALE,
        TOKEN_BEGIN_VALUE,
        TOKEN_END_VALUE
    };
};

#endif // ISLPARSER_H
