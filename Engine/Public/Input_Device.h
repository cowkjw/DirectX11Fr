#pragma once
#include "Base.h"

BEGIN(Engine)
class CInput_Device : public CBase
{
private:
    CInput_Device() = default;
    virtual ~CInput_Device() = default;

public:
    HRESULT Initialize(HWND hWnd);

    // 매 프레임 호출: 내부 상태 갱신
    void Update();
    // 키보드 조회
    _bool IsKeyDown(Keyboard::Keys key) const;
    _bool IsKeyPressed(Keyboard::Keys key) const;

    POINT GetMousePos() const;
    _bool IsMouseButtonDown(Mouse::ButtonStateTracker b) const;

private:
    Mouse* m_mouse{ nullptr };
    Keyboard*                       m_keyboard{ nullptr };
    Mouse::ButtonStateTracker *     m_mouseTracker{ nullptr };
    Keyboard::KeyboardStateTracker* m_kbdTracker{ nullptr };

public:
    static CInput_Device* Create(HWND hWnd);
    virtual void Free() override;
};
END