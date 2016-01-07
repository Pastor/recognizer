#include <Windows.h>
#include <CommCtrl.h>
#include <objbase.h>
#include <allheaders.h>
#include <capi.h>
#include "resource.h"

#pragma comment(lib, "libtesseract302.lib")
#pragma comment(lib, "liblept168.lib")
#pragma comment(lib, "comctl32.lib")

int CALLBACK
MainWindow(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LPCSTR
__CurrentDirectory();

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    InitCommonControls();
    DialogBoxParam(hInstance, TEXT("MAIN"), nullptr, MainWindow, reinterpret_cast<LPARAM>(hInstance));
    return EXIT_SUCCESS;
}

static LPCSTR
__CurrentDirectory()
{
    static CHAR   cBuffer[2048];
    TCHAR  wBuffer[sizeof(cBuffer)];

    auto wBufferSize = sizeof(wBuffer) / sizeof(wBuffer[0]);
    auto ret = GetModuleFileName(GetModuleHandle(nullptr), wBuffer, wBufferSize);
    ret = WideCharToMultiByte(CP_ACP, 0, wBuffer, wBufferSize, cBuffer, sizeof(cBuffer), nullptr, nullptr);
    return cBuffer;
}

int CALLBACK
MainWindow(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HINSTANCE hInstance = nullptr;
    static TessBaseAPI *handle = nullptr;
    static LPCSTR lpstrCurrentDirectory = nullptr;

    switch (uMsg) {
    case WM_INITDIALOG: {
        hInstance = reinterpret_cast<HINSTANCE>(lParam);
        handle = TessBaseAPICreate();
        lpstrCurrentDirectory = __CurrentDirectory();
        return TRUE;
    }
    case WM_COMMAND: {
        auto idCtrl = LOWORD(wParam);
        if (idCtrl == IDC_SELECT_IMAGE) {
            PIX         *pix  = nullptr;
            char        *text = nullptr;
            int          ret  = 0;

            MessageBox(hWnd, TEXT("Select"), nullptr, MB_OK);
#if 0
            ret = TessBaseAPIInit3(handle, datadirectory, language);
            if (ret == 0) {
                __try {
                    TessBaseAPISetPageSegMode(handle, PSM_AUTO_ONLY);
                    TessBaseAPISetImage2(handle, pix);
                    TessBaseAPISetInputName(handle, "RecognizerName1");
                    ret = TessBaseAPIRecognize(handle, nullptr);
                    if (ret == 0) {
                        text = TessBaseAPIGetUTF8Text(handle);
                        fputs(text, stdout);
                        TessDeleteText(text);
                    }
                } __finally {
                    TessBaseAPIEnd(handle);
                }
            }
            if (pix != nullptr) {
                pixDestroy(&pix);
            }
#endif
        }
        break;
    }
    case WM_CLOSE: {
        if (handle != nullptr)
            TessBaseAPIDelete(handle);
        EndDialog(hWnd, 0);
        handle = nullptr;
        break;
    }
    default:
        break;
    }
    return FALSE;
}
