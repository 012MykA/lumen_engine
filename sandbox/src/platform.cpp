#include "platform.hpp"

#include <lumen_engine/lumen_engine.hpp>

#if defined(LUMEN_PLATFORM_WINDOWS)

	#include <Windows.h>

	#include <dwmapi.h>
	#pragma comment(lib, "dwmapi.lib")

void SetWindowDarkMode(lm::Window *window) {
	HWND hwnd = static_cast<HWND>(window->GetNativeHandle());
	BOOL useDarkMode = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode,
			sizeof(useDarkMode));
}

bool IsSystemDarkTheme() {
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(
			HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0,
			KEY_READ, &hKey);

	if (lRes == ERROR_SUCCESS) {
		DWORD dwValue;
		DWORD dwSize = sizeof(dwValue);
		lRes = RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
				reinterpret_cast<LPBYTE>(&dwValue), &dwSize);
		RegCloseKey(hKey);

		if (lRes == ERROR_SUCCESS) {
			return dwValue == 0;
		}
	}

	return false;
}

#else

void SetWindowDarkMode(lm::Window *window, char value) {}

bool IsSystemDarkTheme() {
	return false;
}

#endif
