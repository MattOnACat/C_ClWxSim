#include "ui.h"

// File ID: 01
// ID is used in error msgs (first 2 digits of code)

const char g_szClassName[] = "WindowClass"; // name of window class for identifying later

World wld;
HANDLE hWldMutex;
HANDLE hSimThread;
DWORD threadID;
boolean simRunning = FALSE;

// main loop; hInstance is handle for exe file in memory, hPrevInstance always NULL, lpCmdLine is command line args as str, nCmdShow is int for ShowWindow()
// LPSTR = windows char* (LP = long pointer, C would mean constant), UINT = windows unsigned int a
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    WNDCLASSEX wc;  // window class var
    HWND hwnd;      // window var
    MSG Msg;        // stores current message (keypress, mouse click etc)

    int wndSize[2] = {920, 680};  // default window size
    char wndTitle[] = "ClWxSim";  // title of main window (displayed in title bar)

    // setup world struct
    wld_init(&wld, 256, 256, 1012.0f, 0.0000727f, 10);

    // create mutexes
    hWldMutex = CreateMutexW(NULL, FALSE, NULL);

    // REGISTER WINDOW CLASS
#ifdef DEBUG_OUT
    printf("D0101 Registering Main Window Class\n");
#endif

    wc.cbSize = sizeof(WNDCLASSEX);               // size of structure
    wc.style = 0;                                // class styles
    wc.lpfnWndProc = WndProc;                          // pointer to window procedure for the class
    wc.cbClsExtra = 0;                                // extra memory allocated for the class
    wc.cbWndExtra = 0;                                // extra memory allocated per window
    wc.hInstance = hInstance;                        // Handle to the app instance
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // large icon shown when use presses alt tab (32x32)
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // cursor to be displayed over window
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);         // background brush to set colour of window
    wc.lpszMenuName = NULL;                             // name of menu resource for the windows of the class
    wc.lpszClassName = g_szClassName;                    // name to identify the class with
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);  // smaller icon, to show in top left of window and taskbar (16x16)

    if (!RegisterClassEx(&wc))   // Call RegisterClassEx and check for fail
    {
        MessageBox(NULL, "E0100: Window Registration Failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    #ifdef DEBUG_OUT
            printf("E0100: Window Registration Failed\n");
    #endif
        return 0;
    }

    // CREATE WINDOW
    #ifdef DEBUG_OUT
        printf("D0102 Successfully Registered Window Class, Creating Window\n");
    #endif
    //    extended wnds style                          window style       x, y coords for top left        x, y wnd size
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, wndTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                          CW_USEDEFAULT, wndSize[0], wndSize[1], NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "E0105: Window Creation Failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    #ifdef DEBUG_OUT
            printf("E0105: Window Creation Failed\n");
    #endif
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    #ifdef DEBUG_OUT
        printf("D0103 Showing Window\n");
    #endif

    // set process priority
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // run sim thread
    hSimThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) RunSimLoop, NULL, 0, &threadID);
    SetThreadPriority(hSimThread, THREAD_PRIORITY_HIGHEST);

    // MESSAGE/RUN loop
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        // check window messages
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

// Window procedure (where messages are processed)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    int updateView = 0;
    LPMINMAXINFO wnd_minmax_info;

    switch (msg) {
        case WM_CREATE:
            CreateMenuBar(hwnd);
            CreateButtons(hwnd);

            // updating graph requires wld object to be available, so check mutex
            WaitForSingleObject(hWldMutex, INFINITE);
            UpdateDrawGraph(hwnd, &wld, GRAPH_X_PADDING, GRAPH_TOP);
            ReleaseMutex(hWldMutex);

            InvalidateRect(hwnd, NULL, 1); // redraw window next loop
            break;

        case WM_SIZE:
            InvalidateRect(hwnd, NULL, 1); // redraw window next loop
            break;

        case WM_PAINT:
            DrawGraph(hwnd, GRAPH_X_PADDING, GRAPH_TOP);
            break;

        case WM_GETMINMAXINFO:
            wnd_minmax_info = (LPMINMAXINFO) lParam;

            wnd_minmax_info->ptMinTrackSize.x = WND_WIDTH_MIN;
            wnd_minmax_info->ptMinTrackSize.y = WND_HEIGHT_MIN;
            break;

        case WM_COMMAND:

            // Check Menus and Buttons
            switch (LOWORD(wParam)) {
                case IDM_SIM_START:
                    simRunning = TRUE;
                    break;
                case IDM_SIM_PAUSE:
                    simRunning = FALSE;
                    break;
                case IDM_FILE_NEW:
                case IDM_FILE_OPEN:
                case IDM_SIM_SETUP:
                    MessageBeep(MB_ICONINFORMATION);
                    break;

                case IDB_UPDATEVIEW:
                    // updating graph requires wld object to be available, so check mutex
                    WaitForSingleObject(hWldMutex, INFINITE);
                    UpdateDrawGraph(hwnd, &wld, GRAPH_X_PADDING, GRAPH_TOP);
                    ReleaseMutex(hWldMutex);

                    InvalidateRect(hwnd, NULL, 1); // redraw window next loop
                    break;

                case IDCB_AUTOUPDATEVIEW:
                    // check update view checkbox
                    updateView = IsDlgButtonChecked(hwnd, IDCB_AUTOUPDATEVIEW);

                    if (updateView) {
                        CheckDlgButton(hwnd, IDCB_AUTOUPDATEVIEW, BST_UNCHECKED);
                    } else {
                        CheckDlgButton(hwnd, IDCB_AUTOUPDATEVIEW, BST_CHECKED);
                    }

                    break;

                case IDM_FILE_QUIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(
                    0); // causes GetMessage to return false (0 is the return val, used to pass val if this wnd called by another program)
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    if (IsDlgButtonChecked(hwnd, IDCB_AUTOUPDATEVIEW)) {
        // updating graph requires wld object to be available, so check mutex
        WaitForSingleObject(hWldMutex, INFINITE);
        UpdateDrawGraph(hwnd, &wld, GRAPH_X_PADDING, GRAPH_TOP);
        ReleaseMutex(hWldMutex);

        InvalidateRect(hwnd, NULL, 1); // redraw window next loop
    }

    return 0;
}

void CreateMenuBar(HWND hwnd) {
#ifdef DEBUG_OUT
    printf("D0104 Adding Menus\n");
#endif

    HMENU hMenubar;
    HMENU hFileMenu;
    HMENU hSimMenu;

    hMenubar = CreateMenu();
    hFileMenu = CreateMenu();
    hSimMenu = CreateMenu();

    // Add menu options to file menu
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_NEW, L"&New");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

    // Add menu options to sim menu
    AppendMenuW(hSimMenu, MF_STRING, IDM_SIM_SETUP, L"&Setup");
    AppendMenuW(hSimMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hSimMenu, MF_STRING, IDM_SIM_START, L"&Start/Resume");
    AppendMenuW(hSimMenu, MF_STRING, IDM_SIM_PAUSE, L"&Pause");

    // Append menus to menu bar
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hFileMenu, L"&File");
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hSimMenu, L"&Sim");

    // Send the menubar to the window
    SetMenu(hwnd, hMenubar);
}

void CreateButtons(HWND hwnd) {
    CreateWindowW(L"Button", L"Update View",
                  WS_VISIBLE | WS_CHILD,
                  20, 50, 80, 25, hwnd, (HMENU) IDB_UPDATEVIEW,
                  NULL, NULL);

    CreateWindowW(L"Button", L"Auto Update View",
                  WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                  20, 20, 185, 35, hwnd, (HMENU) IDCB_AUTOUPDATEVIEW,
                  NULL, NULL);
}


// threaded call to sim_tick()
_Noreturn void RunSimLoop(void *pID) {
    #ifdef DEBUG_OUT
        printf("D0107 Sim loop thread started\n");
    #endif

    while (TRUE) {
        if (simRunning) {
            WaitForSingleObject(hWldMutex, INFINITE);
            int tickNum = sim_tick(&wld);
            ReleaseMutex(hWldMutex);

            #ifdef DEBUG_OUT
                        printf("D0106 Threaded sim tick ran: %d\n", tickNum);
            #endif
        }
    }
}