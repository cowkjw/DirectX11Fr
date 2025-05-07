#pragma once
#include "Base.h"

BEGIN_NAMESPACE(Engine)

class CInput_Device final : public CBase
{
public:
    CInput_Device() = default;
    virtual ~CInput_Device() = default;

    // 윈도우 메시지 수신 핸들러 등록 및 초기화
    HRESULT Initialize(HWND hWnd);

    // WndProc에서 WM_INPUT 메시지 전달
    void ProcessRawInput(LPARAM lParam);

    // 매 프레임 호출: 이전 상태 복사
    void Update();

    // 키보드 입력
    _bool IsKeyDown(_ushort vkey)   const;
    _bool IsKeyPressed(_ushort vkey) const; // 한번만 눌렸을 때

    // 마우스 버튼 (0: Left, 1: Right)
    _bool IsMouseDown(_ulonglong btn)    const;
    _bool IsMousePressed(_ulonglong btn) const; // 한번만 눌렸을 때

    // 마우스 위치 및 휠
    POINT GetMousePos()   const;
    _long  GetMouseWheel() const;
    POINT GetMouseDelta() const;



private:
    HWND m_hWnd = nullptr;

    // 입력 상태 저장용 배열
    array<_bool, 256> m_keys = {};
    array<_bool, 256> m_prevKeys = {};
    array<_bool, 5> m_mouseBtn = {};
    array<_bool, 5> m_prevMouse = {};
    POINT                 m_MousePos = { 0,0 };
    POINT                 m_PrevMousePos = { 0,0 };
    LONG                  m_WheelDelta = 0;
public:

    static CInput_Device* Create(HWND hWnd);
    virtual void Free() override;
};

END_NAMESPACE

