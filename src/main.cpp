#include "islparser.h"
#include "utils.h"
#include <locale>
#ifdef _WIN32
# define tstrcmp wcscmp
# define tprintf wprintf
#else
# include <cstring>
# define tstrcmp strcmp
# define tprintf printf
#endif


static const char pHelp[] = R"(
ISL Translation Compiler (v1.0)
===========================================
Converts ISL translation files to BIN format

ARGUMENTS:
  --input=<file>     Set path to a single ISL file
  --decode           Convert from BIN back to ISL

EXAMPLE:
  islcompiler --input=source.isl

NOTES:
  - --decode works only with --input
  - Overwrites the output file if it already exists.
)";


static tstring replaceExtension(const tstring &filePath, const tstring &newExt)
{
    auto pos = filePath.find_last_of('.');
    tstring outPath = (pos == tstring::npos) ? filePath : filePath.substr(0, pos);
    outPath += newExt;
    return outPath;
}

#ifdef _WIN32
int __cdecl _tmain (int argc, TCHAR *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    std::locale::global(std::locale(""));
    NS_Args::parseCmdArgs(argc, argv);
    if (argc < 2 || !NS_Args::cmdArgContains(_T("--input"))) {
        printf("%s", pHelp);
        return 0;
    }

    printf("\nISL Translation Compiler (v1.0)\n");
    if (NS_Args::cmdArgContains(_T("--log")))
        NS_Logger::AllowWriteLog();

    if (NS_Args::cmdArgContains(_T("--input"))) {
        tstring inputPath = NS_Args::cmdArgValue(_T("--input"));
        if (inputPath.empty() || !NS_File::fileExists(inputPath)) {
            tprintf(_T("[ERROR] Input file not found: %s\n"), inputPath.c_str());
            return 0;
        }

        if (NS_Args::cmdArgContains(_T("--decode"))) {
            if (!ISLParser::binToTranslation(inputPath))
                tprintf(_T("[ERROR] Conversion failed: %s\n"), inputPath.c_str());
            else {
                tprintf(_T("[OK] Conversion succeeded: %s\n"), inputPath.c_str());
            }
        } else {
            tstring err;
            tstring outPath = replaceExtension(inputPath, _T(".bin"));
            ISLParser isl(inputPath);
            if (!isl.translationToBin(outPath, err))
                tprintf(_T("[ERROR] Conversion failed: %s\n"), err.c_str());
            else {
                tprintf(_T("[OK] Conversion succeeded: %s\n"), outPath.c_str());
            }
        }
    }
    return 0;
}
