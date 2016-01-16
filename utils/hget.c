#include <stdio.h>
#include <stdlib.h>
#include <urlmon.h>
#include <windows.h>

#pragma  comment(lib, "urlmon.lib")

int
main(int argc, char **argv)
{
    HRESULT hr = URLDownloadToFileA(NULL, argv[1], argv[2], 0, NULL);
    return hr == S_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
