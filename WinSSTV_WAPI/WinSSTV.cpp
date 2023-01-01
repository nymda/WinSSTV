#include <iostream>
#include <dinput.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <windows.h>

#include "framework.h"
#include "WinSSTV.h"

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

COLORREF bg = RGB(200, 200, 200);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//HWNDs
HWND hWnd = 0;

HWND btn_openFile = 0;
#define ID_OPENFILE 1

HWND btn_play = 0;
#define ID_PLAY 2

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

HWND cmb_pbDevice = 0;
HWND lbl_pbDevice = 0;
#define ID_DEVICE 8

#define ID_TIMER 101

HWND pbr_playbackBar = 0;
HWND lbl_playbackTime = 0;
#define ID_PLAYBACKBAR 9

HWND btn_stopPlayback = 0;
#define ID_STOPPLAYBACK 10

HWND pbr_volBar = 0;
HWND lbl_volBar = 0;
#define ID_VOLUMEBAR 11

HWND btn_save = 0;
#define ID_SAVE 12

HWND nud_fontSize = 0;
#define ID_FONTSIZE 13
int iFontSize = 1;

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
BITMAP bm = {};

HDC hdc = 0;
HDC hdcMem = 0;
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
bool voxTone = true;
wav::wasapiDevicePkg* wasapiPackage = 0;
int wasapiSelectedDevice = 0;

void createConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);
	freopen_s(&f, "CONIN$", "r", stdin);
}

wav::playbackReporter pr = {};

int rsCountdown = 0;
VOID CALLBACK timerCallback(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
	if (!pr.running && pr.playedPercent > 0) {
		pr = {}; //reset pr
	}

	int iVol = SendMessage((HWND)pbr_volBar, (UINT)TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	pr.volume = (float)((float)iVol / 100.f);

	//RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME | RDW_NOINTERNALPAINT);
	SendMessage(pbr_playbackBar, TBM_SETPOS, (WPARAM)1, (LPARAM)pr.playedPercent);

	//update the label
	wchar_t timeTxt[100];
	swprintf_s(timeTxt, 100, L"%02d:%02d:%02d / %02d:%02d:%02d", pr.currentMin, pr.currentSec, (pr.currentMs / 100) * 10, pr.totalMin, pr.totalSec, (pr.totalMs / 100) * 10);

	if (pr.playedPercent == 100) {
		rsCountdown++;
		if (rsCountdown >= 5) {
			pr = {};
			rsCountdown = 0;
		}
	}
	SetWindowText(lbl_playbackTime, timeTxt);
}

DWORD WINAPI beginPlaybackThreaded(LPVOID lpParameter)
{
	wav::beginPlayback(wasapiSelectedDevice, &pr);
	return 0;
}

void beginEncode(bool playAudio) {

	if (pr.running) { return; }

	if (!wav::init(sampleRate)) {
		printf_s("[ERR] Could not allocate WAV memory\n");
		return;
	}

	//add 500ms header
	wav::addTone(0, 500.f);

	//add VOX tone
	SSTV::addVoxTone();

	//call actual encode function
	selectedEncMode->ec(resized.data);

	//add 500ms footer
	wav::addTone(0, 500.f);

	printf_s("[Encode complete, storing %i bytes]\n", wav::header.fileSize);
	printf_s(" Expected time: %f MS\n", wav::expectedDurationMS);
	printf_s(" Actual time  : %f MS\n", wav::actualDurationMS);
	printf_s(" Added: %i, Skipped: %i\n", wav::balance_AddedSamples, wav::balance_SkippedSamples);

	if (playAudio) {
		HANDLE thread = CreateThread(NULL, 0, beginPlaybackThreaded, NULL, 0, NULL);
	}
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

		char convertedStrBuffer[128];
		int imgChannels = 0;
		wcstombs_s(0, convertedStrBuffer, ofn.lpstrFile, 128);
		full.data = (SSTV::rgb*)stbi_load(convertedStrBuffer, &full.size.X, &full.size.Y, &imgChannels, 3);
		if (!full.data) { return false; }
		return true;
	}
	return false;
}

bool saveWavFile () {
	OPENFILENAME ofn = { 0 };
	TCHAR szFile[260] = { 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = (L"Waveform Audio File Format (*.wav)\0*.wav\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = L"wav";

	if (GetSaveFileName(&ofn) == TRUE && ofn.lpstrFile)
	{
		FILE* ofptr = 0;
		int openErrNo = _wfopen_s(&ofptr, ofn.lpstrFile, L"wb");
		if (openErrNo != 0 || !ofptr) {
			char errBuffer[256] = {};
			strerror_s(errBuffer, openErrNo);
			printf_s("[ERR] Could not open output file (%s)\n", errBuffer);
			return 0;
		}

		beginEncode(false);

		if (wav::save(ofptr) <= 0) {
			char errBuffer[256] = {};
			strerror_s(errBuffer, errno);
			printf_s("[ERR] Issue writing to output file (%s)\n", errBuffer);
			fclose(ofptr);
			return 0;
		}

		fclose(ofptr);
		
		wchar_t saveTxt[100];
		swprintf_s(saveTxt, 100, L"Exported audio as %s", ofn.lpstrFile);

		MessageBox(NULL, saveTxt, L"Saved", MB_OK);

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
	if (!InitInstance(hInstance, nCmdShow))
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

	return (int)msg.wParam;
}

ATOM registerClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSSTVWAPI));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(bg);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINSSTVWAPI);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX, CW_USEDEFAULT, 0, 610, 344, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void initUI(HWND parent) {

	/*
		  +---+
		  |   |
		  O   |
		 /|\  |
		 / \  |
			  |
		=========
	*/

	//font setup
	HFONT defFont;
	defFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	//open file button
	btn_openFile = CreateWindowW(L"Button", L"Open Image", WS_VISIBLE | WS_CHILD | WS_BORDER, dispImgSize.X + 10, 5, 260, 25, parent, (HMENU)ID_OPENFILE, NULL, NULL);
	SendMessage(btn_openFile, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//encode method label
	lbl_encodeType = CreateWindowW(L"Static", L"Method:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 40 + 3, 50, 15, parent, (HMENU)(ID_ENCODETYPE & 0xFF), NULL, NULL);
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
	lbl_overlay = CreateWindowW(L"Static", L"Overlay:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 65 + 3, 50, 15, parent, (HMENU)(ID_OVERLAY & 0xFF), NULL, NULL);
	SendMessage(lbl_overlay, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//overlay textbox
	txt_overlay = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_HSCROLL | ES_AUTOHSCROLL, dispImgSize.X + 65, 65, 180, 20, parent, (HMENU)ID_OVERLAY, NULL, NULL);
	SendMessage(txt_overlay, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
	SendMessage(txt_overlay, EM_SETLIMITTEXT, (WPARAM)512, (LPARAM)0);
	ShowScrollBar(txt_overlay, 0, false);

	//RGB mode dropdown and info
	lbl_rgbMode = CreateWindowW(L"Static", L"Colours:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 89 + 3, 50, 15, parent, (HMENU)(ID_RGBMODE & 0xFF), NULL, NULL);
	SendMessage(lbl_rgbMode, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	cmb_rgbMode = CreateWindowW(L"ComboBox", L"EDR", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST, dispImgSize.X + 65, 89, 200, 25, parent, (HMENU)ID_RGBMODE, NULL, NULL);
	SendMessage(cmb_rgbMode, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
	for (rgbModePair& rm : rgbModes) {
		SendMessage(cmb_rgbMode, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)rm.modeTxt);
	}
	SendMessage(cmb_rgbMode, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	//samplerate dropdown label
	lbl_sampleRate = CreateWindowW(L"Static", L"Sample Rate:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 114 + 3, 75, 15, parent, (HMENU)(ID_SAMPLERATE & 0xFF), NULL, NULL);
	SendMessage(lbl_sampleRate, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//samplerate dropdown and info
	cmb_sampleRate = CreateWindowW(L"ComboBox", L"EDR", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST, dispImgSize.X + 90, 114, 175, 25, parent, (HMENU)ID_SAMPLERATE, NULL, NULL);
	SendMessage(cmb_sampleRate, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
	for (sampleRatePair& srp : standardSampleRates) {
		SendMessage(cmb_sampleRate, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)srp.rateTxt);
	}
	SendMessage(cmb_sampleRate, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	//playback device dropdown label
	lbl_pbDevice = CreateWindowW(L"Static", L"Device:", WS_VISIBLE | WS_CHILD, dispImgSize.X + 15, 139 + 3, 50, 15, parent, (HMENU)(ID_DEVICE & 0xFF), NULL, NULL);
	SendMessage(lbl_pbDevice, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//playback device dropdown and info
	cmb_pbDevice = CreateWindowW(L"ComboBox", L"PBD", WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST, dispImgSize.X + 65, 139, 200, 25, parent, (HMENU)ID_DEVICE, NULL, NULL);
	SendMessage(cmb_pbDevice, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
	for (int wd = 0; wd < wasapiPackage->deviceCount; wd++) {
		SendMessage(cmb_pbDevice, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)wasapiPackage->devices[wd].name);
	}
	SendMessage(cmb_pbDevice, CB_SETCURSEL, (WPARAM)0, (LPARAM)wasapiPackage->defaultDevice);

	//play button
	btn_play = CreateWindowW(L"Button", L"PLAY", WS_VISIBLE | WS_CHILD | WS_BORDER, dispImgSize.X + 15, 217, 175, 25, parent, (HMENU)ID_PLAY, NULL, NULL);
	SendMessage(btn_play, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//save button
	btn_save = CreateWindowW(L"Button", L"SAVE", WS_VISIBLE | WS_CHILD | WS_BORDER, dispImgSize.X + 194, 217, 71, 25, parent, (HMENU)ID_SAVE, NULL, NULL);
	SendMessage(btn_save, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//playback time label
	lbl_playbackTime = CreateWindowW(L"Static", L"00:00:00 / 00:00:00", WS_VISIBLE | WS_CHILD, 5, 280 + 5, 200, 15, parent, (HMENU)(ID_PLAYBACKBAR & 0xFF), NULL, NULL);

	pbr_playbackBar = CreateWindowW(L"msctls_trackbar32", L"", WS_VISIBLE | WS_CHILD | WS_DISABLED, 4, 250, 561, 25, parent, (HMENU)ID_PLAYBACKBAR, NULL, NULL);
	SendMessage(pbr_playbackBar, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, 1000));

	btn_stopPlayback = CreateWindowW(L"Button", L"\x25A0", WS_VISIBLE | WS_CHILD | WS_BORDER, 565, 250, 25, 25, parent, (HMENU)ID_STOPPLAYBACK, NULL, NULL);
	SendMessage(btn_stopPlayback, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	pbr_volBar = CreateWindowW(L"msctls_trackbar32", L"", WS_VISIBLE | WS_CHILD, 440, 280, 150, 25, parent, (HMENU)ID_VOLUMEBAR, NULL, NULL);
	SendMessage(pbr_volBar, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, 100));
	SendMessage(pbr_volBar, TBM_SETPOS, (WPARAM)1, (LPARAM)(100));

	lbl_volBar = CreateWindowW(L"Static", L"Volume (100%):", WS_VISIBLE | WS_CHILD, 365, 280 + 3, 75, 15, parent, (HMENU)(ID_VOLUMEBAR & 0xFF), NULL, NULL);
	SendMessage(lbl_volBar, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));

	//numeric up/down for setting the font size
	nud_fontSize = CreateWindowW(L"msctls_updown32", L"Font size", WS_VISIBLE | WS_CHILD | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS, dispImgSize.X + 248, 65, 180, 20, parent, (HMENU)ID_FONTSIZE, NULL, NULL);
	SendMessage(nud_fontSize, UDM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(3, 1));
	
}

void drawRect(SSTV::vec2 p1, int width, int height) {
	Rectangle(hdc, p1.X, p1.Y, p1.X + width, p1.Y + height);
}

wchar_t overlayWideBuffer[512] = L"";
int overlayLen = 0;

void reprocessImage() {
	if (!hasLoadedImage) { return; }
	
	SSTV::resizeNN(&full, &resized);

	tr::bindToCanvas(&resized);
	tr::setTextOrigin({ 0, 0 });

	if (overlayLen > 0) {
		tr::drawString(tr::white, iFontSize, overlayWideBuffer);
	}

	if (rgbMode != SSTV::RGBMode::RGB) {
		SSTV::setColours(&resized, rgbMode);
	}
	else if (selectedEncMode->ID == encModeID::EM_BW8 || selectedEncMode->ID == encModeID::EM_BW12) {
		SSTV::setColours(&resized, SSTV::RGBMode::MONO);
	}

	updateFromRGBArray(resized.data, resized.size);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{

		case WM_HSCROLL:
		{
			if (lParam == (LPARAM)pbr_volBar) {
				int iVol = SendMessage((HWND)pbr_volBar, (UINT)TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				pr.volume = (float)((float)iVol / 100.f);

				wchar_t volTxt[100];
				swprintf_s(volTxt, 100, L"Volume (%03d%%):", iVol);
				SetWindowText(lbl_volBar, volTxt);

			}
			break;
		}

		case WM_VSCROLL:
		{
			if (lParam == (LPARAM)nud_fontSize && hasLoadedImage) {
				iFontSize = LOWORD(SendMessage((HWND)nud_fontSize, (UINT)UDM_GETPOS, (WPARAM)0, (LPARAM)0));
				reprocessImage();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		}

		case WM_NOTIFY: //makes the trackbar tick always blue, allowing for it to be disabled to ignore user input
		{
			if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
				LPNMCUSTOMDRAW lpNMCD = (LPNMCUSTOMDRAW)lParam;
				UINT idc = lpNMCD->hdr.idFrom;

				if (lpNMCD->dwDrawStage == CDDS_PREPAINT) {
					return CDRF_NOTIFYITEMDRAW;
				}
				else if (lpNMCD->dwDrawStage == CDDS_ITEMPREPAINT && lpNMCD->dwItemSpec == TBCD_THUMB) {
					if (wParam == ID_PLAYBACKBAR) {
						SelectObject(lpNMCD->hdc, CreatePen(0, 1, RGB(100, 0, 0)));
						SelectObject(lpNMCD->hdc, CreateSolidBrush(RGB(100, 0, 0)));
					}
					else {
						SelectObject(lpNMCD->hdc, CreatePen(0, 1, RGB(0, 120, 215)));
						SelectObject(lpNMCD->hdc, CreateSolidBrush(RGB(0, 120, 215)));
					}

					Rectangle(lpNMCD->hdc, lpNMCD->rc.left, lpNMCD->rc.top, lpNMCD->rc.right, lpNMCD->rc.bottom);
					return CDRF_SKIPDEFAULT;
				}
			}
			break;
		}

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
			}

			//overlay text changed
			if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == ID_OVERLAY) {
				if (hasLoadedImage) {
					overlayLen = GetWindowTextW(txt_overlay, (LPWSTR)&overlayWideBuffer, 512);

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

			//Change selected playback device
			if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_DEVICE) {
				int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				wasapiSelectedDevice = wasapiPackage->devices[ItemIndex].ID;
			}

			//Change vox enabled
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_DOVOX) {
				if (SendMessage((HWND)lParam, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED) {
					voxTone = true;
				}
				else {
					voxTone = false;
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

			//open file button
			if (LOWORD(wParam) == ID_PLAY) {
				if (hasLoadedImage) {
					beginEncode(true);
				}
			}

			if (LOWORD(wParam) == ID_SAVE) {
				if (hasLoadedImage) {
					saveWavFile();
				}
			}

			if (LOWORD(wParam) == ID_STOPPLAYBACK) {
				if (pr.running) {
					pr.abort = true;
				}
			}

			break;
		}

		case WM_CREATE:
		{
			//no idea
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icex.dwICC = ICC_STANDARD_CLASSES;
			InitCommonControlsEx(&icex);

			//create debug console
			createConsole();

			//assign initial size to the bitmaps
			resized.size = selectedEncMode->size;

			//init text rendering
			tr::initFont();

			//get wasapi devices
			wasapiPackage = wav::WASAPIGetDevices();
		
			//start the playback update timer
			::SetTimer(hWnd, ID_TIMER, 100, (TIMERPROC)timerCallback);

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
			SelectObject(hdc, CreateSolidBrush(bg));

			drawRect({ dispImgPos.X - 1, dispImgPos.Y - 1, }, dispImgSize.X + 2, dispImgSize.Y + 2);

			//image
			StretchBlt(hdc, dispImgPos.X, dispImgPos.Y, dispImgSize.X, dispImgSize.Y, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

			//other bounding boxes
			SelectObject(hdc, CreatePen(0, 1, RGB(0, 0, 0)));
			SelectObject(hdc, CreateSolidBrush(bg));

			//draw the bounding box for the image settings
			drawRect({ dispImgPos.X + dispImgSize.X + 5, 35 }, 260, 212);

			EndPaint(hWnd, &ps);

			break;
		}

		case WM_CTLCOLORSTATIC:
		{
			HDC hdcStatic = (HDC)wParam;
			SetBkColor(hdcStatic, bg);
			return (INT_PTR)CreateSolidBrush(bg);
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

