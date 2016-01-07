#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <allheaders.h>
#include <capi.h>

#pragma comment(lib, "libtesseract302.lib")
#pragma comment(lib, "liblept168.lib")

struct OptsArg {
    const char * const name;
    char *             value;
    bool               required;
    const char * const descr;
} args[] = {
    { "data",     nullptr,      false,  "" },
    { "lang",     nullptr,      true,   "" },
    { "name",     "GlobalName", false,  "" },
    { "filename", nullptr,      true,   "" },
};

enum OptsName {
    DATA = 0,
    LANG,
    NAME,
    FILE_NAME
};

static void 
die(const char *format...) {
    va_list list;
    char    buffer[2048];

    va_start(list, format);
    vsprintf(buffer, format, list);
    va_end(list);
    fputs(buffer, stderr);
    fputc('\n', stderr);
    exit(1);
}

static bool 
__exists(const char * const filename)
{
    FILE *fd = fopen(filename, "r");
    if (fd) {
        fclose(fd);
        return true;
    }
    return false;
}

static void
__validate_args()
{
    for (int i = 0; i < sizeof(args) / sizeof(args[0]); ++i) {
        if ((args[i].value == nullptr || strlen(args[i].value) == 0) && args[i].required)
            die("Argument not defined: %s", args[i].name);
    }
}

static void
__parse_args(int argc, char **argv)
{
    for (auto i = 1; i < argc; i++) {
        auto it = strchr(argv[i], '=');
        if (it == nullptr)
            continue;
        (*it) = 0;
        auto name  = argv[i] + 2;
        auto value = it + 1;
        for (auto k = 0; k < sizeof(args)/sizeof(args[0]); k++) {
            if (!strcmp(args[k].name, name)) {
                args[k].value = value;
            }
        }
    }
}

int
main(int argc, char **argv)
{
    TessBaseAPI *handle    = nullptr;
    PIX         *pix       = nullptr;
    char        *text      = nullptr;
    char         ch        = '\n';
    int          ret       = 0;

    /*{
        args[DATA].value = "C:\\work\\fetcher\\.tesseract";
        args[LANG].value = "eng";
        args[FILE_NAME].value = "C:\\work\\fetcher\\.uncompressed\\10B279C6E674A1088591BF921DA2A3AD83A6C3F1A07F8BBCB790FE0EAF31E8D7.jpg";
        args[NAME].value = "TestingName";
    }*/
    __parse_args(argc, argv);
    __validate_args();
    if ((pix = pixRead(args[FILE_NAME].value)) == nullptr)
        die("Error reading image: %s", args[FILE_NAME].value);

    handle = TessBaseAPICreate();
    ret = TessBaseAPIInit3(handle, args[DATA].value, args[LANG].value);
    if (ret == 0) {
        TessBaseAPISetPageSegMode(handle, PSM_AUTO_ONLY);
        TessBaseAPISetImage2(handle, pix);
        TessBaseAPISetInputName(handle, args[NAME].value);
        ret = TessBaseAPIRecognize(handle, nullptr);
        if (ret == 0) {
            text = TessBaseAPIGetUTF8Text(handle);
            fputs(text, stdout);
            TessDeleteText(text);
        }
        TessBaseAPIEnd(handle);
    }    
    TessBaseAPIDelete(handle);
    pixDestroy(&pix);
    ExitProcess(EXIT_SUCCESS);
    exit(EXIT_SUCCESS);
    //fscanf(stdin, "%c", &ch);
    return EXIT_SUCCESS;
}

