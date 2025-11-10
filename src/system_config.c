#include "./include/system_config.h"

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef PlaySound
#undef DrawTextEx

void getScreenResolution(int *width, int *height) {
    *width  = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
}

void getSystemLanguage(char *lang, int size) {
    LANGID langId = GetUserDefaultUILanguage();
    DWORD ret = GetLocaleInfo(langId, LOCALE_SISO639LANGNAME, lang, size);
    if (ret == 0) lang[0] = '\0'; // fallback
}

#elif defined(__linux__)
#include <X11/Xlib.h>
#include <locale.h>
#include <string.h>

void getScreenResolution(int *width, int *height) {
    Display *d = XOpenDisplay(NULL);
    if (d) {
        Screen *s = DefaultScreenOfDisplay(d);
        *width  = s->width;
        *height = s->height;
        XCloseDisplay(d);
    } else {
        *width  = 800;  // fallback
        *height = 600;
    }
}

void getSystemLanguage(char *lang, int size) {
    const char *locale = setlocale(LC_ALL, "");
    if (locale) {
        strncpy(lang, locale, size-1);
        lang[size-1] = '\0';
    } else {
        strncpy(lang, "en", size);
    }
}

#endif
