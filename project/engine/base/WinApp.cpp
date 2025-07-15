#include "WinApp.h"
#include"externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma comment(lib,"winmm.lib")

// メインスレッドではなくMTAでCOM使用
HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

void WinApp::Initialize() {

    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラス名
    wc.lpszClassName = L"CG2WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //ウィンドウクラスを登録する
    RegisterClass(&wc);
    //ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0,0,kClientWidth,kClientHeight };
    //クライアント領域を元に実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    //ウィンドウの生成
    hwnd = CreateWindow(
        wc.lpszClassName,        //利用するクラス名
        windowTitle_.c_str(),    //タイトルバーの文字
        WS_OVERLAPPEDWINDOW,     //よく見るウィンドウスタイル
        CW_USEDEFAULT,           //表示X座標(Windowsに任せる)
        CW_USEDEFAULT,           //表示Y座標(WindowsOSに任せる)
        wrc.right - wrc.left,    //ウィンドウ横幅
        wrc.bottom - wrc.top,    //ウィンドウ縦幅
        nullptr,                 //親ウィンドウハンドル
        nullptr,                 //メニューハンドル
        wc.hInstance,            //インスタンスハンドル
        nullptr);                //オプション 

    //ウィンドウを表示する
    ShowWindow(hwnd, SW_SHOW);
    // システムタイマーの分解度を上げる
    timeBeginPeriod(1);
}

//ウィンドウプロージャー
LRESULT CALLBACK WinApp::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    //メッセージ二応じてゲーム固有の処理を行う
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
        //ウィンドウが破棄された
    case WM_DESTROY:
        //OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }
    //標準のメッセージ処理を行う
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void WinApp::Finalize() {
    // CloseHandle(fenceEvent);
    CloseWindow(hwnd);
    ///COMの終了
    CoUninitialize();
}

bool WinApp::ProcessMessage() {
    
    MSG msg{};

    // Windowにメッセージが来ていたら最優先で処理させる
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT) 
    {  
        return true;
    }    
    return false;
}

void WinApp::SetWindowTitle(const std::wstring& title) {
    windowTitle_ = title;
}