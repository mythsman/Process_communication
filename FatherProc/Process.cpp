#include<windows.h>
#include"resource.h"
HINSTANCE topInstance;
HWND dialog;
HWND nextHwnd;
wchar_t data[1000];
LPVOID lpdata = NULL;
HANDLE hmap = NULL;
LPWSTR ConvertCharToLPWSTR(char * szString)
{
	int dwLen = strlen(szString) + 1;
	int nwLen = MultiByteToWideChar(CP_ACP, 0, szString, dwLen, NULL, 0);
	LPWSTR lpszPath = new WCHAR[dwLen];
	MultiByteToWideChar(CP_ACP, 0, szString, dwLen, lpszPath, nwLen);
	return lpszPath;
}

INT_PTR CALLBACK DlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE)
		{
			DestroyWindow(hdlg);
			PostQuitMessage(0);
		}
		return 0;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON1:
			nextHwnd = FindWindow(L"Child Process", L"Child Process");
			SendMessage(GetDlgItem(dialog, IDC_EDIT1), WM_GETTEXT, 1000, (LPARAM)data);
			SendMessage(nextHwnd, LB_ADDSTRING, NULL, (LPARAM)data);
			break;
		case IDC_BUTTON2:
			nextHwnd = FindWindow(L"Child Process", L"Child Process");
				hmap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,PAGE_READWRITE | SEC_COMMIT, 0, 10000, "sharedFile");
				lpdata = MapViewOfFile(hmap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
			ZeroMemory(lpdata, sizeof(lpdata));
			SendMessage(GetDlgItem(dialog, IDC_EDIT3), WM_GETTEXT, 10000, (LPARAM)lpdata);
			SendMessage(nextHwnd, WM_SETTEXT, NULL, NULL);
			break;
		case IDC_BUTTON3://Choose file to load
			OPENFILENAME opfn;
			WCHAR strFilename[MAX_PATH];
			ZeroMemory(&opfn, sizeof(OPENFILENAME));
			opfn.lStructSize = sizeof(OPENFILENAME);
			opfn.lpstrFilter = L"*.*\0*.*\0*.txt\0*.txt\0";
			opfn.nFilterIndex = 1;
			opfn.lpstrFile = strFilename;
			opfn.lpstrFile[0] = '\0';
			opfn.nMaxFile = sizeof(strFilename);
			opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			if (GetOpenFileName(&opfn))
			{
				HWND hEdt = GetDlgItem(hdlg, IDC_EDIT2);
				SendMessage(hEdt, WM_SETTEXT, NULL, (LPARAM)strFilename);
			}
			HANDLE hFile;
			hFile = CreateFile(strFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			char buff[1000];
			DWORD dwReads;
			ReadFile(hFile, buff, 1000, &dwReads, NULL);
			buff[dwReads] = 0;
			SendMessage(GetDlgItem(hdlg, IDC_EDIT3), WM_SETTEXT, NULL, (LPARAM)ConvertCharToLPWSTR(buff));
			CloseHandle(hFile);

			break;
		case IDC_BUTTON4:
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			si.cb = sizeof(si);
			CreateProcess(NULL, ConvertCharToLPWSTR("ChildProc.exe"), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			break;

		default:
			break;
		}
	}
		return 0;
	}
	return (INT_PTR)FALSE;
}

LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	switch (uMsg){
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_CREATE:
	{
		HWND hdlg = CreateDialog(topInstance, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, (DLGPROC)DlgProc);
		if (hdlg)
		{
			dialog = hdlg;
			ShowWindow(hdlg, SW_NORMAL);
		}
		return 0;

	}
	case WM_SETTEXT:{
		hmap = OpenFileMappingA(FILE_MAP_READ, FALSE, "sharedFile");
		lpdata = MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
		SendMessage(GetDlgItem(dialog, IDC_EDIT4), uMsg, NULL, (LPARAM)lpdata);
		UnmapViewOfFile(lpdata);
		CloseHandle(hmap);
		return 0;

	}
	case LB_ADDSTRING:{
		SendMessage(GetDlgItem(dialog, IDC_LIST1), uMsg, wParam, lParam);
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
	WCHAR* cls_Name = L"Father Process";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = cls_Name;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);

	topInstance = hInstance;

	HWND hwnd = CreateWindow(
		cls_Name,           //类名，要和刚才注册的一致  
		L"Father Process",          //窗口标题文字 
		WS_OVERLAPPEDWINDOW,        //窗口外观样式  
		0,             //窗口相对于父级的X坐标  
		0,             //窗口相对于父级的Y坐标  
		0,                //窗口的宽度  
		0,                //窗口的高度  
		NULL,               //没有父窗口，为NULL
		NULL,               //没有菜单，为NULL  
		hInstance,          //当前应用程序的实例句柄  
		NULL);              //没有附加数据，为NULL  
	if (hwnd == NULL){                //检查窗口是否创建成功 
		return 0;
	}
	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
