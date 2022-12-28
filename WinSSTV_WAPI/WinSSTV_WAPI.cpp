#include <iostream>
#include <dinput.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <windows.h>

#include "framework.h"
#include "WinSSTV_WAPI.h"

#include "modes.h"
#include "wav.h"
#include "SSTV.h"
#include "textRendering.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_LOADSTRING 100

/*
    doing anything in win32 is an excersize in masochism.
    this is 50% spaghetti and 50% raw unsafe pointers. have fun.
*/

//allows for normal fonts
#pragma comment( lib, "comctl32.lib" )
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.19041.1110' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//HWNDs
HWND hWnd = 0;

HWND btn_openFile = 0;
#define ID_OPENFILE 1

HWND btn_encode = 0;
#define ID_ENCODE 2

HWND cmb_encodeType = 0;
HWND lbl_encodeType = 0;
#define ID_ENCODETYPE 3

HWND cbx_doVox = 0;
#define ID_DOVOX 4

HWND txt_overlay = 0;
HWND lbl_overlay = 0;
#define ID_OVERLAY 5

HWND cmb_sampleRate = 0;
HWND lbl_sampleRate = 0;
#define ID_SAMPLERATE 6

HWND cmb_rgbMode = 0;
HWND lbl_rgbMode = 0;
#define ID_RGBMODE 7

// Forward declarations of functions included in this code module:
ATOM registerClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

//image buffers
SSTV::simpleBitmap full; //users entire unprocessed image
SSTV::simpleBitmap resized; //users image, resized to match the SSTV method with overlays and stuff

//winAPI image handling stuff
HBITMAP hBitmap = 0;
HBITMAP hOldBitmap = 0;
HDC hdc = 0;
HDC hdcMem = 0;
BITMAP bm = {};
HINSTANCE hI = 0;
PAINTSTRUCT ps = {};
RECT rect = { 0, 0 };
RECT rc = { 0, 0 };

//positions for the image box
SSTV::vec2 dispImgPos = { 5, 6 };
SSTV::vec2 dispImgSize = { 320, 240 };

//UI selection structs and meta
struct sampleRatePair {
    int rate;
    const wchar_t* rateTxt;
};

sampleRatePair standardSampleRates[] = { 
    {8000,  L"8000hz"}, 
    {11025, L"11025hz"},
	{16000, L"16000hz"},
	{22050, L"22050hz"},
	{32000, L"32000hz"},
	{44100, L"44100hz"},
	{48000, L"48000hz"},
	{96000, L"96000hz"},
};

struct rgbModePair {
    SSTV::RGBMode mode;
    const wchar_t* modeTxt;
};

rgbModePair rgbModes[] = {
	{SSTV::RGBMode::RGB, L"RGB"},
	{SSTV::RGBMode::R, L"R"},
	{SSTV::RGBMode::G, L"G"},
	{SSTV::RGBMode::B, L"B"},
};

//UI selection variables
bool hasLoadedImage = false;
int sampleRate = 8000;
SSTV::RGBMode rgbMode = SSTV::RGBMode::RGB;
encMode* selectedEncMode = &modes[0];

void createConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);
	freopen_s(&f, "CONIN$", "r", stdin);
}

void updateFromRGBArray(SSTV::rgb* data, SSTV::vec2 size) {
    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = size.X;
    bmih.biHeight = -size.Y;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 10;
    bmih.biYPelsPerMeter = 10;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    BITMAPINFO dbmi;
    ZeroMemory(&dbmi, sizeof(dbmi));
    dbmi.bmiHeader = bmih;
    dbmi.bmiColors->rgbBlue = 0;
    dbmi.bmiColors->rgbGreen = 0;
    dbmi.bmiColors->rgbRed = 0;
    dbmi.bmiColors->rgbReserved = 0;
    unsigned char* bits = 0;

	HDC hdc = GetDC(hWnd);
    
    DeleteObject(SelectObject(hdcMem, hOldBitmap));
    
	hBitmap = CreateDIBSection(hdc, &dbmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
	if (!hBitmap || hBitmap == INVALID_HANDLE_VALUE) {
		printf_s("Error: CreateDIBSection failed with error %d\n", GetLastError());
        return;
	}

    int BytesPerLine = size.X * 3;
    if (BytesPerLine % 4 != 0) { BytesPerLine += 4 - BytesPerLine % 4; }
	for (int y = 0; y < size.Y; y++) {
        PBYTE line = bits;
		for (int x = 0; x < size.X; x++) {
            line[0] = data[y * size.X + x].b;
            line[1] = data[y * size.X + x].g;
            line[2] = data[y * size.X + x].r;
            line += 3;
		}
        bits += BytesPerLine;
	}

    GetObject(hBitmap, sizeof(BITMAP), &bm);
    hdc = GetDC(hWnd);
    hdcMem = CreateCompatibleDC(hdc);
    hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
    
    ReleaseDC(0, hdc);
    ReleaseDC(0, hdcMem);
}

bool loadImageFile() {
    full.data = 0;
    full.size = { 0, 0 };
    hBitmap = 0;
    hOldBitmap = 0;
    hdc = 0;
    hdcMem = 0;
    bm = {};
    hI = 0;
    ps = {};
    rect = { 0, 0 };
    rc = { 0, 0 };
    
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[260] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("Images\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE && ofn.lpstrFile)
    {
        if (full.data) { free(full.data); }
        char convertedStrBuffer[128];
        int imgChannels = 0;
        wcstombs_s(0, convertedStrBuffer, ofn.lpstrFile, 128);
        full.data = (SSTV::rgb*)stbi_load(convertedStrBuffer, &full.size.X, &full.size.Y, &imgChannels, 3);
        if (!full.data) { return false; }
        return true;
    }
    return false;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSSTVWAPI, szWindowClass, MAX_LOADSTRING);
    registerClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINSSTVWAPI));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM registerClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSSTVWAPI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BACKGROUND);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSSTVWAPI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, 0, 640, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void initUI(HWND parent) {    
    
    //font setup
    HFONT defFont;
    defFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    
    //open file button
    btn_openFile = CreateWindowW(L"Button", L"Open Image", WS_VISIBLE | WS_CHILD | WS_BORDER, dispImgSize.X + 10, 5, 100, 25, parent, (HMENU)ID_OPENFILE, NULL, NULL);
    SendMessage(btn_openFile, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    
    //encode method label
	lbl_encodeType = CreateWindowW(L"Static", L"Method:", WS_VISIBLE | WS_CHILD , dispImgSize.X + 15, 43, 100, 15, parent, (HMENU)(ID_ENCODETYPE & 0xFF), NULL, NULL);
    SendMessage(lbl_encodeType, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

    //encode method dropdown + add items
    cmb_encodeType = CreateWindowW(L"ComboBox", L"EDR", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST, dispImgSize.X + 65, 40, 200, 25, parent, (HMENU)ID_ENCODETYPE, NULL, NULL);
    SendMessage(cmb_encodeType, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    for (encMode& m : modes) {
        if (m.size != SSTV::vec2{ 0, 0 }) {
            wchar_t tempWriteBuffer[128] = {};
            swprintf_s(tempWriteBuffer, 128, L"%s (%i x %i)\n", m.desc, m.size.X, m.size.Y);
            SendMessage(cmb_encodeType, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)tempWriteBuffer);
        }
    }
    SendMessage(cmb_encodeType, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

    //overlay textbox label
    lbl_overlay = CreateWindowW(L"Static", L"Overlay:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 67, 100, 15, parent, (HMENU)(ID_OVERLAY & 0xFF), NULL, NULL);
    SendMessage(lbl_overlay, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    
    //overlay textbox
	txt_overlay = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, dispImgSize.X + 65, 65, 200, 20, parent, (HMENU)ID_OVERLAY, NULL, NULL);
	SendMessage(txt_overlay, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

    //samplerate dropdown label
    lbl_sampleRate = CreateWindowW(L"Static", L"Sample Rate:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 93, 100, 15, parent, (HMENU)(ID_SAMPLERATE & 0xFF), NULL, NULL);
    SendMessage(lbl_sampleRate, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    
    //samplerate dropdown and info
	cmb_sampleRate = CreateWindowW(L"ComboBox", L"EDR", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST, dispImgSize.X + 90, 90, 175, 25, parent, (HMENU)ID_SAMPLERATE, NULL, NULL);
	SendMessage(cmb_sampleRate, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
	for (sampleRatePair& srp : standardSampleRates) {
		SendMessage(cmb_sampleRate, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)srp.rateTxt);
	}
    SendMessage(cmb_sampleRate, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

    //RGB mode dropdown and info
	lbl_rgbMode = CreateWindowW(L"Static", L"Colours:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 119, 100, 15, parent, (HMENU)(ID_RGBMODE & 0xFF), NULL, NULL);
	SendMessage(lbl_rgbMode, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    cmb_rgbMode = CreateWindowW(L"ComboBox", L"EDR", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST, dispImgSize.X + 65, 116, 200, 25, parent, (HMENU)ID_RGBMODE, NULL, NULL);
	SendMessage(cmb_rgbMode, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
	for (rgbModePair& rm : rgbModes) {
		SendMessage(cmb_rgbMode, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)rm.modeTxt);
	}
	SendMessage(cmb_rgbMode, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        
    //vox checkbox
	cbx_doVox = CreateWindowW(L"Button", L"VOX Tone", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, dispImgSize.X + 15, 140, 100, 20, parent, (HMENU)ID_DOVOX, NULL, NULL);
	SendMessage(cbx_doVox, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    SendMessage(cbx_doVox, BM_SETCHECK, BST_CHECKED, 0);  
}

void drawRect(SSTV::vec2 p1, int width, int height) {
    Rectangle(hdc, p1.X, p1.Y, p1.X + width, p1.Y + height);
}

wchar_t overlayWideBuffer[128] = L"";
int overlayLen = 0;

void reprocessImage() {
    SSTV::resizeNN(&full, &resized);

    tr::bindToCanvas(&resized);
    tr::setTextOrigin({ 0, 0 });

    if (overlayLen > 0) {
        tr::drawString(tr::white, 1, overlayWideBuffer);
    }

    if (rgbMode != SSTV::RGBMode::RGB) {
        //set colours
        SSTV::setColours(&resized, rgbMode);
    }

    updateFromRGBArray(resized.data, resized.size);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icex);
    
    switch (message)
    {  
            case WM_COMMAND:
            {
                //Change encode mode
                if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_ENCODETYPE) {
                    int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    selectedEncMode = &modes[ItemIndex];
                    resized.size = selectedEncMode->size;
                    if (hasLoadedImage) {
                        reprocessImage();
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    }
                }

                //change sample rate
                if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_SAMPLERATE) {
                    int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    sampleRate = standardSampleRates[ItemIndex].rate;
                    printf_s("Sample rate changed to %i\n", sampleRate);
                }

                //overlay text changed
                if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == ID_OVERLAY) {
                    if (hasLoadedImage) {
                        overlayLen = GetWindowTextW(txt_overlay, (LPWSTR)&overlayWideBuffer, 128);

                        //clear if the delete character is inserted with ctrl-backspace
                        if (overlayWideBuffer[overlayLen - 1] == 127) {
                            overlayWideBuffer[0] = 0x00;
                            SetWindowTextW(txt_overlay, L"");
                        }

                        reprocessImage();
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    }
                }

                //Change RGB mode
                if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_RGBMODE) {
                    int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    rgbMode = rgbModes[ItemIndex].mode;
                    if (hasLoadedImage) {
                        reprocessImage();
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    }
                }

                //open file button
                if (LOWORD(wParam) == ID_OPENFILE) {
                    if (loadImageFile()) {
                        hasLoadedImage = true;
                        reprocessImage();
                        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    }
                }

                break;
            }
       
            case WM_CREATE: 
            {
                //create debug console
                createConsole();
                
                //assign initial size to the bitmaps
                resized.size = selectedEncMode->size;

                //init text rendering
                tr::initFont();

                //init GUI items
                initUI(hWnd);

                break;
            }

            case WM_PAINT:
            {        
                hdc = BeginPaint(hWnd, &ps);

                //draw the preview picture box
                SetStretchBltMode(hdc, STRETCH_HALFTONE);

                //outline for picture box
                SelectObject(hdc, CreatePen(0, 1, RGB(255, 0, 0)));
                SelectObject(hdc, CreateSolidBrush(RGB(200, 200, 200)));

				drawRect({ dispImgPos.X - 1, dispImgPos.Y - 1, }, dispImgSize.X + 2, dispImgSize.Y + 2);
                
                //image
                StretchBlt(hdc, dispImgPos.X, dispImgPos.Y, dispImgSize.X, dispImgSize.Y, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                
                //other bounding boxes
                SelectObject(hdc, CreatePen(0, 1, RGB(0, 0, 0)));
                SelectObject(hdc, CreateSolidBrush(RGB(200, 200, 200)));
                
                //draw the bounding box for the image settings
                drawRect({ dispImgPos.X + dispImgSize.X + 5, 35}, 260, 150);
                
                EndPaint(hWnd, &ps);
                
                break;
            }

            case WM_CTLCOLORSTATIC:
            {
                HDC hdcStatic = (HDC)wParam;
                SetBkColor(hdcStatic, RGB(200, 200, 200));
                return (INT_PTR)CreateSolidBrush(RGB(200, 200, 200));
            }
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

