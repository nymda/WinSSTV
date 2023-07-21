#include <iostream>
#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "distortions.h"
#include "wav.h"

COLORREF bgDistortions = RGB(200, 200, 200);

HWND pbr_noise = 0;
HWND lbl_noise = 0;
#define ID_NOISEBAR 1

HWND lbl_frShift = 0;
HWND edt_frShift = 0;
HWND nud_frShift = 0;
#define ID_FRSHIFT 2
int iFrShift = 1;

wav::wavDistortions* distortionPkg;

void initDistortionsUI(HWND parent) {
    HFONT defFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    
    pbr_noise = CreateWindowW(L"msctls_trackbar32", L"", WS_VISIBLE | WS_CHILD, 80, 5, 200, 25, parent, (HMENU)ID_NOISEBAR, NULL, NULL);
    SendMessage(pbr_noise, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELPARAM(0, 100));
	SendMessage(pbr_noise, TBM_SETPOS, (WPARAM)1, (LPARAM)(distortionPkg->noiseLvl * 100));

    lbl_noise = CreateWindowW(L"Static", L"Noise (000%):", WS_VISIBLE | WS_CHILD, 5, 5 + 3, 75, 15, parent, (HMENU)(ID_NOISEBAR & 0xFF), NULL, NULL);
    SendMessage(lbl_noise, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    
    wchar_t volTxt[100];
    swprintf_s(volTxt, 100, L"Noise (%03d%%):", (int)(distortionPkg->noiseLvl * 100));
    SetWindowText(lbl_noise, volTxt);

    nud_frShift = CreateWindowW(L"msctls_updown32", L"frOffset", WS_VISIBLE | WS_CHILD | UDS_ALIGNRIGHT | UDS_ARROWKEYS, 5, 35, 180, 20, parent, (HMENU)ID_FRSHIFT, NULL, NULL);
    SendMessage(nud_frShift, UDM_SETRANGE32, (WPARAM)-100, (LPARAM)100);

    edt_frShift = CreateWindowW(L"edit", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 110, 35, 170, 20, parent, (HMENU)(ID_FRSHIFT & 0xff), NULL, NULL);
    SendMessage(edt_frShift, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
    SendMessage(nud_frShift, UDM_SETBUDDY, (WPARAM)edt_frShift, 0);

    wchar_t frShiftTxt[33];
    frShiftTxt[32] = 0x00;
	wsprintf(frShiftTxt, L"%d", distortionPkg->timingOffset);
    SendMessage(edt_frShift, WM_SETTEXT, 32, (LPARAM)frShiftTxt);
    
	lbl_frShift = CreateWindowW(L"Static", L"Frequency shift (Hz):", WS_VISIBLE | WS_CHILD, 5, 35 + 3, 100, 15, parent, (HMENU)(ID_FRSHIFT & 0xFF), NULL, NULL);
    SendMessage(lbl_frShift, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
}

// Declare the callback function for the new window
LRESULT CALLBACK NewWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

        case WM_COMMAND:
        {
            if (lParam == (LPARAM)edt_frShift) {
                wchar_t frShiftTxt[33];
                frShiftTxt[32] = 0x00;
                SendMessage(edt_frShift, WM_GETTEXT, 32, (LPARAM)frShiftTxt);
                int conv = _wtoi(frShiftTxt);
                if (conv > 1000) { SendMessage(edt_frShift, WM_SETTEXT, 0, (LPARAM)L"1000"); }
                else if (conv < -1000) { SendMessage(edt_frShift, WM_SETTEXT, 0, (LPARAM)L"-1000"); }
                distortionPkg->timingOffset = conv;
            }
        }
        
        case WM_VSCROLL:
        {
            if (lParam == (LPARAM)nud_frShift) {
                iFrShift = (int)SendMessage(nud_frShift, UDM_GETPOS32, 0, 0);
                wchar_t frShiftTxt[16];
                swprintf_s(frShiftTxt, 16, L"%d", iFrShift);
                SetWindowText(edt_frShift, frShiftTxt);
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkColor(hdcStatic, bgDistortions);
            return (INT_PTR)CreateSolidBrush(bgDistortions);
        }
        
        case WM_CREATE:
        {
            //init GUI items
            initDistortionsUI(hWnd);
            break;
        }

        case WM_DESTROY:
        {
            DestroyWindow(hWnd);
            break;       
        }

        case WM_NOTIFY:
        {
            if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
                LPNMCUSTOMDRAW lpNMCD = (LPNMCUSTOMDRAW)lParam;
                UINT idc = lpNMCD->hdr.idFrom;

                if (lpNMCD->dwDrawStage == CDDS_PREPAINT) {
                    return CDRF_NOTIFYITEMDRAW;
                }
                else if (lpNMCD->dwDrawStage == CDDS_ITEMPREPAINT && lpNMCD->dwItemSpec == TBCD_THUMB) {
                    SelectObject(lpNMCD->hdc, CreatePen(0, 1, RGB(0, 120, 215)));
                    SelectObject(lpNMCD->hdc, CreateSolidBrush(RGB(0, 120, 215)));

                    Rectangle(lpNMCD->hdc, lpNMCD->rc.left, lpNMCD->rc.top, lpNMCD->rc.right, lpNMCD->rc.bottom);
                    return CDRF_SKIPDEFAULT;
                }
            }
            break;
        }
        
        case WM_HSCROLL:
        {
            if (lParam == (LPARAM)pbr_noise) { //volume bar control
                int iNoise = SendMessage((HWND)pbr_noise, (UINT)TBM_GETPOS, (WPARAM)0, (LPARAM)0);
                distortionPkg->noiseLvl = (float)((float)iNoise / 100.f);

                wchar_t volTxt[32];
                swprintf_s(volTxt, 32, L"Noise (%03d%%):", iNoise);
                SetWindowText(lbl_noise, volTxt);
            }
            break;
        }

        
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);

    }

    return 0;
}

// Declare the function to create the new window
HWND createDistortionsWnd(HINSTANCE hInstance, wav::wavDistortions* pDistortionPkg)
{
    if(!pDistortionPkg){ return 0; }
    distortionPkg = pDistortionPkg;
    
	const wchar_t CLASS_NAME[] = L"Distortions Window Class";

    // Register the window class for the new window
    WNDCLASS wc = { };
    wc.lpfnWndProc = NewWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(bgDistortions);
    RegisterClass(&wc);

    // Create the new window
    HWND hWnd = CreateWindowW((LPCWSTR)CLASS_NAME, L"Distortions", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX, CW_USEDEFAULT, 0, 500, 300, nullptr, nullptr, hInstance, nullptr);

    if (hWnd == NULL)
    {
        MessageBox(NULL, L"Window creation failed!", L"Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    // Show the new window
    UpdateWindow(hWnd);

    return hWnd;
}