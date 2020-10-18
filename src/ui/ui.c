#include <stdio.h>
#include <windows.h>

#include <ui.h>

#include <config.h>

// File ID: 0
// ID is used in error msgs (first 2 digits of code)

const char g_szClassName[] = "WindowClass"; // name of window class for identifying later

// main loop; hInstance is handle for exe file in memory, hPrevInstance always NULL, lpCmdLine is command line args as str, nCmdShow is int for ShowWindow()
// LPSTR = windows char* (LP = long pointer, C would mean constant), UINT = windows unsigned int
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{

  WNDCLASSEX wc;  // window class var
  HWND hwnd;      // window var
  MSG Msg;        // stores current message (keypresse, mouse click etc)

  int wndSize[2] = {920, 680};  // default window size
  char wndTitle[] = "ClWxSim";  // title of main window (displayed in title bar)

  // REGISTER WINDOW CLASS
  #ifdef DEBUG_OUT
    printf("01 Registering Main Window Class\n");
  #endif

  wc.cbSize         =   sizeof(WNDCLASSEX);               // size of structure
  wc.style          =   0;                                // class styles
  wc.lpfnWndProc    =   WndProc;                          // pointer to window procedure for the class
  wc.cbClsExtra     =   0;                                // extra memory allocated for the class
  wc.cbWndExtra     =   0;                                // extra memory allocated per window
  wc.hInstance      =   hInstance;                        // Handle to the app instance
  wc.hIcon          =   LoadIcon(NULL, IDI_APPLICATION);  // large icon shown when use presses alt tab (32x32)
  wc.hCursor        =   LoadCursor(NULL, IDC_ARROW);      // cursor to be displayed over window
  wc.hbrBackground  =   (HBRUSH)(COLOR_WINDOW+1);         // background brush to set colour of window
  wc.lpszMenuName   =   NULL;                             // name of menu resourcefor the windows of the class
  wc.lpszClassName  =   g_szClassName;                    // name to identify the class with
  wc.hIconSm        =   LoadIcon(NULL, IDI_APPLICATION);  // smaller icon, to show in top left of window and taskbar (16x16)

  if(!RegisterClassEx(&wc))   // Call RegisterClassEx and check for fail
  {
    MessageBox(NULL, "E0100: Window Registration Failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    #ifdef DEBUG_OUT
      printf("E0100: Window Registration Failed\n");
    #endif
    return 0;
  }

  // CREATE WINDOW
  #ifdef DEBUG_OUT
    printf("02 Successfully Registered Window Class, Creating Window\n");
  #endif
                  //    extended wnds style                          window style       x, y coords for top left        x, y wnd size
  hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, wndTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, wndSize[0], wndSize[1], NULL, NULL, hInstance, NULL);

  if(hwnd == NULL)
  {
    MessageBox(NULL, "E0101: Window Creation Failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    #ifdef DEBUG_OUT
      printf("E0101: Window Creation Failed\n");
    #endif
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  #ifdef DEBUG_OUT
    printf("03 Showing Window\n");
  #endif

  // MESSAGE loop
  while (GetMessage(&Msg, NULL, 0, 0) > 0)
  {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }

  return Msg.wParam;
}

// Window procedure (where messages are processed)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
      AddMenus(hwnd);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)){
        case IDM_FILE_NEW:
        case IDM_FILE_OPEN:
          MessageBeep(MB_ICONINFORMATION);
          break;

        case IDM_FILE_QUIT:
          SendMessage(hwnd, WM_CLOSE, 0 ,0);
          break;
      }
      break;

    case WM_CLOSE:
      DestroyWindow(hwnd);
      break;

    case WM_DESTROY:
      PostQuitMessage(0); // causes GetMessage to return false (0 is the rutrn val, used to pass val if this wnd caleld by another prgoram)
      break;

    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;
}

void AddMenus(HWND hwnd) {
  HMENU hMenubar;
  HMENU hFileMenu;

  #ifdef DEBUG_OUT
    printf("04 Adding Menus\n");
  #endif

  hMenubar = CreateMenu();
  hFileMenu = CreateMenu();

  // Add menu options to file menu
  AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_NEW, L"&New");
  AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open");
  AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
  AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

  // Append menus to menu bar
  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hFileMenu, L"&File");

  // Send the menubar to the window
  SetMenu(hwnd, hMenubar);
}