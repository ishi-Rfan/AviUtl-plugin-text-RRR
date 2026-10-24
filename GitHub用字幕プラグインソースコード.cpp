#include<windows.h>
#include<fstream>
#include"filter.h"
#include<tchar.h>
#include<sstream>
#include<cstring>
#include<shlobj.h>
#include<functional>
#include<vector>
#include <cmath>
#include <gdiplus.h>
using namespace std;
using namespace Gdiplus;

HWND hw;
HWND edit;
HWND pul;
HWND shift_check;
HWND sarcle;
HWND choose;
HWND sarcle_text[8];
HINSTANCE hI;
int hi_int;
char tenta_serif[256];
POINT pt;
vector<function<bool()>> key_bools;

void debug(LPCSTR a){MessageBoxA(NULL,a,"デバッグメニュー",MB_OK);}

void OutputImage(HWND rend_hw,int number,POINT theP,double scale){
    HBITMAP hBitmap = (HBITMAP)LoadImageA(hI, MAKEINTRESOURCEA(number), IMAGE_BITMAP, 0, 0,LR_CREATEDIBSECTION);
    BITMAP bm = {};GetObjectA(hBitmap, sizeof(bm), &bm);HDC hdcScreen = GetDC(nullptr);HDC out_img = CreateCompatibleDC(hdcScreen);COLORREF colorKey = RGB(255, 255, 255);
    int w = (int)(bm.bmWidth / scale);int h = (int)(bm.bmHeight / scale);
    HDC smallDC = CreateCompatibleDC(hdcScreen);
    HBITMAP smallBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
    HGDIOBJ oldSmall = SelectObject(smallDC, smallBitmap);
    HGDIOBJ hOldBitmap = SelectObject(out_img, hBitmap);
    SetStretchBltMode(smallDC, COLORONCOLOR);
    StretchBlt(smallDC, 0, 0, w, h,out_img, 0, 0, bm.bmWidth, bm.bmHeight,SRCCOPY);
    SIZE size={w,h};
    POINT ptSrc = { 0, 0 };
    POINT pt2={theP.x-w/2,theP.y-h/2};
    ShowWindow(rend_hw, SW_SHOWNOACTIVATE);
    if(!UpdateLayeredWindow(rend_hw,hdcScreen,&pt2,&size,smallDC,&ptSrc,colorKey,nullptr,ULW_COLORKEY)) debug("画像がひょう示できませんでした");
}
void SetTextTP(HWND rend_hw){
    HFONT font = CreateFontA(
        24, 0, 0, 0,
        FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "メイリオ"
    );
    SendMessage(
        rend_hw,
        WM_SETFONT,
        (WPARAM)font,
        TRUE
    );
    SetLayeredWindowAttributes(rend_hw,RGB(0,0,0),0,LWA_COLORKEY);
}

void drop_exo(HWND exedit_hwnd, const char* path){
    ScreenToClient(exedit_hwnd, &pt);
    DROPFILES df = {};
    df.pFiles = sizeof(DROPFILES);
    df.pt.x = pt.x;
    df.pt.y = pt.y;
    df.fNC = FALSE;
    df.fWide = FALSE;
    int len = strlen(path) + 2;
    HGLOBAL hMem = GlobalAlloc(GHND, sizeof(DROPFILES) + len);
    void* pMem = GlobalLock(hMem);
    memcpy(pMem, &df, sizeof(DROPFILES));
    strcpy((char*)pMem + sizeof(DROPFILES), path);
    GlobalUnlock(hMem);
    PostMessage(exedit_hwnd, WM_DROPFILES, (WPARAM)hMem, 0);
}
void change_text(string serif,char put[MAX_PATH],ifstream& exo,bool drop_){
    string s=serif;
    string out;
    wchar_t n[256];
    MultiByteToWideChar(CP_ACP,0,s.c_str(),-1,n,256);
    for(wchar_t c:(wstring)n){
        unsigned short v = (unsigned short)c;
        char buf[5];
        sprintf(buf,"%02x%02x",v&0xFF,(v >> 8)&0xFF);
        out+=buf;
    }
    stringstream a;a<<exo.rdbuf();
    string value=a.str();
    string befor=value;
    string s2=out.append(4096-out.size(),'0');
    value.replace(value.find("text")+5,4096,s2);
    ofstream exo2(put, ios::binary | ios::trunc);
    exo2.write(value.data(), value.size());
    if(drop_) drop_exo(edit,put);
    ofstream exo3(put, ios::binary | ios::trunc);
    exo3.write(value.data(), value.size());
    exo.close();
    exo2.close();
    exo3.close();
}
void open_(int path_plus){
    HKEY hKey;
    DWORD type;
    DWORD size=DWORD(MAX_PATH);
    char put[MAX_PATH]={};
    if(RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\AviTentaPlu",0,KEY_READ,&hKey) != ERROR_SUCCESS){
        MessageBoxA(NULL,"設定されてません","オブジェクトが",MB_OK);
        return;
    }
    RegQueryValueExA(hKey,("tenta_path"+to_string(1+path_plus)).c_str(),NULL,&type,(LPBYTE)&put,&size);
    RegCloseKey(hKey);
    {
        ifstream exo(put,ios::in | ios::out | ios::binary);
        if(!exo.is_open()){  
            MessageBoxA(NULL,"設定されてません","オブジェクトが",MB_OK);
            return;
        }
        change_text(tenta_serif,put,exo,true);
    }
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    if(msg == WM_INITDIALOG) {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd,NULL,(GetSystemMetrics(SM_CXSCREEN)-rc.right+rc.left)/2, (GetSystemMetrics(SM_CYSCREEN)-rc.bottom+rc.top)/2, 0, 0,SWP_NOZORDER|SWP_NOSIZE);
        return TRUE;
    }else if(msg==WM_COMMAND){
        switch(LOWORD(wParam)){
            case 11:
                if(GetAsyncKeyState(VK_RETURN)&0x8000)
                if(SendMessage(shift_check,BM_GETCHECK,0,0)!=BST_CHECKED?(GetKeyState(VK_SHIFT)&0x8000):!(GetKeyState(VK_SHIFT)&0x8000)){
                    SetFocus(GetDlgItem(hwnd, 109310));
                    SendMessageA(GetDlgItem(hwnd, 109310), EM_REPLACESEL, TRUE, (LPARAM)"\r\n");
                    return FALSE;
                }
                GetDlgItemTextA(hwnd,109310,tenta_serif, 256);
                if(tenta_serif[0]=='\0') EndDialog(hwnd, 2);
                else EndDialog(hwnd, 1);
                return TRUE;
            case 12:
                EndDialog(hwnd, 0);
                return TRUE;
        }
    }
    return FALSE;
}

bool after_key_bool;
WNDPROC oldButtonProc;
LRESULT CALLBACK MyButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    if(msg==WM_COMMAND&&LOWORD(wParam)>=1002&&LOWORD(wParam)<=1009){
        OPENFILENAMEA ofn = {};
        char path[MAX_PATH] = {};
        ofn.lStructSize = sizeof(ofn);ofn.hwndOwner=hw;
        ofn.lpstrFilter="EXOファイル(*.exo)\0*.exo\0";
        ofn.lpstrFile = path;ofn.nMaxFile = MAX_PATH;ofn.Flags = OFN_FILEMUSTEXIST;
        if(GetOpenFileNameA(&ofn)){
            HKEY hKey;
            if(RegCreateKeyExA(HKEY_CURRENT_USER,"Software\\AviTentaPlu",0,NULL,0,KEY_WRITE,NULL,&hKey,NULL)==ERROR_SUCCESS){
                RegSetValueExA(hKey,("tenta_path"+to_string(LOWORD(wParam)-1001)).c_str(),0,REG_SZ,(const BYTE*)path,(DWORD)strlen(path) + 1);
                RegCloseKey(hKey);
            }
            char* filename=strrchr(path,'\\');
            filename+=1;
            SetDlgItemTextA(hw,1001+(LOWORD(wParam)-1001),filename);
        }
    }else if(msg == WM_CTLCOLORSTATIC){
        HWND ctrl = (HWND)lParam;
        for(int i=0;i<8;i++)
        if(ctrl==sarcle_text[i]){
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetStockObject(HOLLOW_BRUSH);
        }
    }else if(msg==WM_TIMER){
        if(!after_key_bool&&key_bools[SendMessage(pul,CB_GETCURSEL,0,0)]()){
            after_key_bool=key_bools[SendMessage(pul,CB_GETCURSEL,0,0)]();
            GetCursorPos(&pt);

            DestroyWindow(sarcle);sarcle = nullptr;
            DestroyWindow(choose);choose = nullptr;
            for(int i_b_loop=0;i_b_loop<8;i_b_loop++){
                DestroyWindow(sarcle_text[i_b_loop]);sarcle_text[i_b_loop] = nullptr;
            }

            sarcle=CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "STATIC","", WS_POPUP, 100, 100, 200, 200, nullptr, nullptr, hI, nullptr);
            OutputImage(sarcle,1391,pt,3);
            choose=CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "STATIC","", WS_POPUP, 100, 100, 200, 200, nullptr, nullptr, hI, nullptr);
            OutputImage(choose,1392,pt,2.5);
            ShowWindow(choose, SW_HIDE);
            for(int i_b_loop=0;i_b_loop<8;i_b_loop++){
                HKEY hKey;
                DWORD type;
                char put[MAX_PATH]={};
                DWORD size=sizeof(put);
                if(RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\AviTentaPlu",0,KEY_READ,&hKey)==ERROR_SUCCESS)
                {
                    RegQueryValueExA(hKey,("tenta_path"+to_string(i_b_loop+1)).c_str(),NULL,&type,(LPBYTE)&put,&size);
                    RegCloseKey(hKey);
                    char* filename=strrchr(put,'\\');
                    filename+=1;
                    if(put[0]!='\0'){
                        double angle_d=(i_b_loop*45-90)*3.14159265358979323846/180.0;
                        POINT new_pos={(int)(cos(angle_d)*120)+pt.x,(int)(sin(angle_d)*120)+pt.y};
                        sarcle_text[i_b_loop] = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,"STATIC",filename,WS_POPUP | SS_CENTER,new_pos.x-50, new_pos.y-30, 100, 130, hw, nullptr, hI, nullptr);
                        SetTextTP(sarcle_text[i_b_loop]);
                        ShowWindow(sarcle_text[i_b_loop], SW_SHOWNOACTIVATE);
                    }
                }
            }
        }
        if(after_key_bool&&key_bools[SendMessage(pul,CB_GETCURSEL,0,0)]()){
            POINT now_p;
            GetCursorPos(&now_p);
            double thex=pt.x-now_p.x;double they=pt.y-now_p.y;
            if(sqrt(thex*thex+they*they)<60){
                ShowWindow(choose, SW_HIDE);
                goto click_if;
            }else{
                ShowWindow(choose, SW_SHOWNOACTIVATE);
            }
            int angle=((atan2(they,thex)*180.0/3.14159265358979323846)-67.5);
            if(angle<0) angle+=360;
            double angle_d=((int)(angle/45)*45-90)*3.14159265358979323846/180.0;
            POINT new_pos={(int)(cos(angle_d)*110)+pt.x,(int)(sin(angle_d)*110)+pt.y};
            OutputImage(choose,1392,new_pos,2.5);
        }
        if(after_key_bool&&!key_bools[SendMessage(pul,CB_GETCURSEL,0,0)]()){
            after_key_bool=key_bools[SendMessage(pul,CB_GETCURSEL,0,0)]();

            DestroyWindow(sarcle);sarcle = nullptr;
            DestroyWindow(choose);choose = nullptr;
            for(int i_b_loop=0;i_b_loop<8;i_b_loop++){
                DestroyWindow(sarcle_text[i_b_loop]);sarcle_text[i_b_loop] = nullptr;
            }

            POINT now_p;
            GetCursorPos(&now_p);
            double thex=pt.x-now_p.x;double they=pt.y-now_p.y;
            if(sqrt(thex*thex+they*they)<60) goto click_if;

            HKEY hKey;
            DWORD num=SendMessage(pul,CB_GETCURSEL,0,0);
            DWORD bo=SendMessage(shift_check,BM_GETCHECK,0,0)==BST_CHECKED?1:0;
            if(RegCreateKeyExA(HKEY_CURRENT_USER,"Software\\AviTentaPlu",0,NULL,0,KEY_WRITE,NULL,&hKey,NULL)==ERROR_SUCCESS){
                RegSetValueExA(hKey,"tenta_key",0,REG_DWORD,(const BYTE*)&num,sizeof(num));
                RegSetValueExA(hKey,"tenta_shift",0,REG_DWORD,(const BYTE*)&bo,sizeof(bo));
                RegCloseKey(hKey);
            }

            re_dialog:
            INT_PTR re=DialogBoxParamW(hI,MAKEINTRESOURCEW(10962),nullptr,DialogProc,0);
            if(re==2){
                MessageBoxA(NULL,"何も入力されていません。もう一度ダイアログを開きます。\nやめたい場合はキャンセルを押してください。","編集ミス？",MB_OK);
                goto re_dialog;
            }

            int angle=((atan2(they,thex)*180.0/3.14159265358979323846)-67.5);
            if(angle<0) angle+=360;
            if(re==1)
                open_((int)(angle/45));
        }
        click_if:
        after_key_bool=key_bools[SendMessage(pul,CB_GETCURSEL,0,0)]();
    }
    return CallWindowProc(oldButtonProc, hwnd, msg, wParam, lParam);
}


static char name[] = "テンタクルズプラグイン";
static char info[] = "a";

static HDC hdc;
static HBITMAP bmp;
static HFONT font;
static void* bits;
static int cur_w = 0, cur_h = 0;

BOOL func_proc(void* fp,FILTER_PROC_INFO* fpip){
    if (!fpip->pixelp) return TRUE;
    return TRUE;
}

BOOL func_exit(void* fp){
    if (font) DeleteObject(font);
    if (bmp) DeleteObject(bmp);
    if (hdc) DeleteDC(hdc);
    return TRUE;
}

BOOL func_init(void* fp){
    GdiplusStartupInput gdiplusStartupInput;
    
    hdc = CreateCompatibleDC(nullptr);
    font = CreateFontW(
        32,
        0, 0, 0, 
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        0,0,0,0,
        (LPCWSTR)"Meiryo"
    );
    SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    FILTER* f = (FILTER*)fp;
    hw = f->hwnd;
    SetTimer(hw, 1, 2, nullptr);
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(&hi_int),&hI);
    edit=FindWindowA(NULL, "拡張編集");
    CreateWindowA(
        "STATIC","プラグイン発動キー",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,0,150,30,
        hw,(HMENU)2001,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    pul = CreateWindowA(
        "COMBOBOX",
        NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
        0, 30, 150, 360,
        hw,
        (HMENU)2005,
        (HINSTANCE)GetModuleHandle(NULL),
        NULL
    );
    CreateWindowA(
        "STATIC","EXO設定",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,60,150,30,
        hw,(HMENU)2000,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（上）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        90,90,90,90,
        hw,(HMENU)1002,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（右上）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        180,90,90,90,
        hw,(HMENU)1003,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（右）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        180,180,90,90,
        hw,(HMENU)1004,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（右下）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        180,270,90,90,
        hw,(HMENU)1005,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（下）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        90,270,90,90,
        hw,(HMENU)1006,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（左下）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        0,270,90,90,
        hw,(HMENU)1007,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（左）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        0,180,90,90,
        hw,(HMENU)1008,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    CreateWindowA(
        "BUTTON","EXOを選択\n（左上）",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
        0,90,90,90,
        hw,(HMENU)1009,(HINSTANCE)GetModuleHandle(NULL),nullptr
    );
    shift_check = CreateWindowA(
        "BUTTON","Shift+EnterとEnterを入れ替える",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        0, 360, 250, 30,hw,(HMENU)1020,(HINSTANCE)GetModuleHandle(NULL),NULL
    );

    for(int i_b_loop=0;i_b_loop<8;i_b_loop++){
        HKEY hKey;
        DWORD type;
        char put[MAX_PATH]={};
        DWORD size=sizeof(put);
        if(RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\AviTentaPlu",0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
            RegQueryValueExA(hKey,("tenta_path"+to_string(i_b_loop+1)).c_str(),NULL,&type,(LPBYTE)&put,&size);
            RegCloseKey(hKey);
            char* filename=strrchr(put,'\\');
            filename+=1;
            if(put[0]!='\0')
                SetDlgItemTextA(hw,1002+i_b_loop,filename);
        }
    }

    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F13");RegisterHotKey(hw, 0, 0, VK_F13);key_bools.push_back([](){return (GetAsyncKeyState(VK_F13)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F14");RegisterHotKey(hw, 1, 0, VK_F14);key_bools.push_back([](){return (GetAsyncKeyState(VK_F14)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F15");RegisterHotKey(hw, 2, 0, VK_F15);key_bools.push_back([](){return (GetAsyncKeyState(VK_F15)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F16");RegisterHotKey(hw, 3, 0, VK_F16);key_bools.push_back([](){return (GetAsyncKeyState(VK_F16)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F17");RegisterHotKey(hw, 4, 0, VK_F17);key_bools.push_back([](){return (GetAsyncKeyState(VK_F17)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F18");RegisterHotKey(hw, 5, 0, VK_F18);key_bools.push_back([](){return (GetAsyncKeyState(VK_F18)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F19");RegisterHotKey(hw, 6, 0, VK_F19);key_bools.push_back([](){return (GetAsyncKeyState(VK_F19)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F20");RegisterHotKey(hw, 7, 0, VK_F20);key_bools.push_back([](){return (GetAsyncKeyState(VK_F20)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F21");RegisterHotKey(hw, 8, 0, VK_F21);key_bools.push_back([](){return (GetAsyncKeyState(VK_F21)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F22");RegisterHotKey(hw, 9, 0, VK_F22);key_bools.push_back([](){return (GetAsyncKeyState(VK_F22)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F23");RegisterHotKey(hw, 10, 0, VK_F23);key_bools.push_back([](){return (GetAsyncKeyState(VK_F23)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"F24");RegisterHotKey(hw, 11, 0, VK_F24);key_bools.push_back([](){return (GetAsyncKeyState(VK_F24)&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"Ctrl Alt A");RegisterHotKey(hw, 12, MOD_CONTROL | MOD_ALT, 'A');key_bools.push_back([](){return (GetAsyncKeyState(VK_CONTROL)&0x8000)&&(GetAsyncKeyState(VK_MENU)&0x8000)&&(GetAsyncKeyState('A')&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"Ctrl Shift Z");RegisterHotKey(hw, 13, MOD_CONTROL | MOD_SHIFT, 'Z');key_bools.push_back([](){return (GetAsyncKeyState(VK_CONTROL)&0x8000)&&(GetAsyncKeyState(VK_SHIFT)&0x8000)&&(GetAsyncKeyState('Z')&0x8000);});
    SendMessage(pul, CB_ADDSTRING, 0, (LPARAM)"ホイールクリック");key_bools.push_back([](){return (GetAsyncKeyState(VK_MBUTTON)&0x8000);});
    HKEY hKey;
    DWORD type,put,type2,put2;
    DWORD size=sizeof(put);
    DWORD size2=sizeof(put2);
    if(RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\AviTentaPlu",0,KEY_READ,&hKey)==ERROR_SUCCESS)
    {
        RegQueryValueExA(hKey,"tenta_key",NULL,&type,(LPBYTE)&put,&size);
        RegQueryValueExA(hKey,"tenta_shift",NULL,&type2,(LPBYTE)&put2,&size2);
        RegCloseKey(hKey); 
        SendMessage(pul, CB_SETCURSEL, put, 0);
        SendMessage(shift_check, BM_SETCHECK, put2==1?BST_CHECKED:BST_UNCHECKED, 0);
    }

    oldButtonProc = (WNDPROC)SetWindowLongPtr(hw, GWLP_WNDPROC, (LONG_PTR)MyButtonProc);
    SetWindowPos(hw,NULL,0,0,280,/*160*/40+360+30,SWP_NOMOVE | SWP_NOZORDER);
    return TRUE;
}

static FILTER filter = {0};

extern "C" __declspec(dllexport)
FILTER* __stdcall GetFilterTable(void) {

    filter.name = name;
    filter.information = info;

    filter.func_proc = func_proc;
    filter.func_init = func_init;
    filter.func_exit = func_exit;

    filter.flag = 0;

    filter.track = NULL;
    filter.check = NULL;
    filter.x = filter.y = 0;

    return &filter;
}