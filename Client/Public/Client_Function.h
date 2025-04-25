#pragma once
#include "Engine_Defines.h"

// 와이드 문자열을 UTF-8로 변환 (std::string 반환해야 함)
static string WideToUtf8(const wchar_t* wstr)
{
    if (!wstr) return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    string strUtf8(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strUtf8[0], size_needed, NULL, NULL);
    strUtf8.resize(strUtf8.length() - 1); // 널 종료자 제거

    return strUtf8;
}

// UTF-8을 와이드 문자열로 변환 (반대 방향 변환도 필요함)
static wstring Utf8ToWide(const string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
    wstrTo.resize(wstrTo.length() - 1); // 널 종료자 제거

    return wstrTo;
}

static wchar_t* WStringToWChar(const wstring& wstr)
{
    if (wstr.empty()) return nullptr;
    size_t size = wstr.length() + 1;
    wchar_t* wchars = new wchar_t[size];
    wcscpy_s(wchars, size, wstr.c_str());
    return wchars; // 주의: 호출자가 delete[] 해야 함
}

static string WideToUtf8(const wstring& wstr)
{
    if (wstr.empty()) return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    string strUtf8(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strUtf8[0], size_needed, NULL, NULL);
    strUtf8.resize(strUtf8.length() - 1);

    return strUtf8;
}