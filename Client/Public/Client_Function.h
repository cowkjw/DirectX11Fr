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

inline XMVECTOR RotateVectorByQuaternion(const XMVECTOR& v, const XMVECTOR& q) {
    return XMVector3Rotate(v, q);
}

/// 축(axis)과 라디안 각(angle) 으로부터 쿼터니언 생성
inline XMVECTOR QuaternionFromAxisAngle(const XMVECTOR& axis, float angle) {
    return XMQuaternionRotationAxis(axis, angle);
}

/// Yaw(pivot around Y), Pitch, Roll 순서로부터 쿼터니언 생성
inline XMVECTOR QuaternionFromYawPitchRoll(float yaw, float pitch, float roll) {
    return XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

inline string MakeRelativePath(const string& absPath)
{
    CHAR cwd[MAX_PATH];
    if (!GetCurrentDirectoryA(MAX_PATH, cwd))
        return absPath;  // 못 구했으면 절대경로 반환

    string baseDir = cwd;
    // 뒤에 '\\' 붙여서 "C:\proj\" 식으로
    if (baseDir.back() != '\\')
        baseDir += '\\';

    // absPath이 baseDir로 시작하면 잘라내고, 아니면 그대로
    if (absPath.compare(0, baseDir.size(), baseDir) == 0)
        return absPath.substr(baseDir.size());
    else
        return absPath;
}