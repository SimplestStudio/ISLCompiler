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
  --input-dir=<path> Set directory containing multiple ISL files
  --output=<file>    Set path to the output BIN or ISL file
  --decode           Convert from BIN back to ISL
  --verify           Check ISL file syntax and structure

EXAMPLE:
  islcompiler --input=source.isl
  islcompiler --input-dir=lang --output=out.bin

NOTES:
  - --decode works only with --input
  - Overwrites the output file if it already exists.
)";


#ifdef _WIN32
int __cdecl _tmain (int argc, TCHAR *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    std::locale::global(std::locale(""));
    NS_Args::parseCmdArgs(argc, argv);
    if (argc < 2 || (!NS_Args::cmdArgContains(_T("--input")) && !NS_Args::cmdArgContains(_T("--input-dir")))) {
        printf("%s", pHelp);
        return 0;
    }

    printf("\nISL Translation Compiler (v1.0)\n");
    if (NS_Args::cmdArgContains(_T("--log")))
        NS_Logger::AllowWriteLog();

    tstring outPath;
    if (NS_Args::cmdArgContains(_T("--output")))
        outPath = NS_Args::cmdArgValue(_T("--output"));

    std::vector<tstring> inputFiles;
    if (NS_Args::cmdArgContains(_T("--input-dir"))) {
        tstring inputDir = NS_Args::cmdArgValue(_T("--input-dir"));

        inputFiles = NS_File::getFilesWithExtension(inputDir, _T(".isl"));
        if (inputFiles.empty()) {
            tprintf(_T("[ERROR] Directory does not contain ISL files: %s\n"), inputDir.c_str());
            return 0;
        }

    } else
    if (NS_Args::cmdArgContains(_T("--input"))) {
        tstring inputPath = NS_Args::cmdArgValue(_T("--input"));
        if (inputPath.empty() || !NS_File::fileExists(inputPath)) {
            tprintf(_T("[ERROR] Input file not found: %s\n"), inputPath.c_str());
            return 0;
        }

        if (NS_Args::cmdArgContains(_T("--decode"))) {
            if (outPath.empty())
                outPath = inputPath + _T(".isl");
            if (!ISLParser::binToTranslation(inputPath, outPath))
                tprintf(_T("[ERROR] Conversion failed: %s\n"), inputPath.c_str());
            else {
                tprintf(_T("[OK] Conversion succeeded: %s\n"), outPath.c_str());
            }
            return 0;
        }
        inputFiles.push_back(inputPath);
    }

    tstring err;
    ISLParser isl;
    if (NS_Args::cmdArgContains(_T("--verify"))) {
        isl.verify(inputFiles, err);
        tprintf(_T("%s\n"), err.c_str());

    } else {
        if (outPath.empty()) {
            tstring path = NS_File::parentPath(inputFiles.at(0));
#ifdef _WIN32
            path = NS_File::fromNativeSeparators(path);
#endif
            outPath = path + _T("/out.bin");
        }
        if (!isl.translationToBin(inputFiles, outPath, err))
            tprintf(_T("[ERROR] Conversion failed: %s\n"), err.c_str());
        else {
            tprintf(_T("[OK] Conversion succeeded: %s\n"), outPath.c_str());
        }
    }

    return 0;
}
