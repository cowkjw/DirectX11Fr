#include "Input_Device.h"


HRESULT CInput_Device::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    RAWINPUTDEVICE rid[2] = {};
    // 키보드
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hWnd;
    // 마우스
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x02;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hWnd;

    if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0])))
        return E_FAIL;
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);
    m_MousePos = pt;  // 이제 m_MousePos는 클라이언트 좌표 시작점
    return S_OK;
}

void CInput_Device::ProcessRawInput(LPARAM lParam)
{
    _uint size = 0;
    ::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    if (size == 0) return;

    vector<_byte> buffer(size);
    if (::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
        return;

    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        _ushort vk = raw->data.keyboard.VKey;
        _bool   down = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
        if (vk < m_keys.size())
            m_keys[vk] = down;
    }
    else if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        // 절대 위치 갱신: 스크린 좌표 → 클라이언트 좌표
        POINT pt;
        ::GetCursorPos(&pt);
        ::ScreenToClient(m_hWnd, &pt);
        m_MousePos = pt;

        auto& m = raw->data.mouse;
        // 버튼 처리
        if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)   m_mouseBtn[0] = true;
        if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)     m_mouseBtn[0] = false;
        if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)  m_mouseBtn[1] = true;
        if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)    m_mouseBtn[1] = false;
        if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) m_mouseBtn[2] = true;
        if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)   m_mouseBtn[2] = false;
        if (m.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)      m_mouseBtn[3] = true;
        if (m.usButtonFlags & RI_MOUSE_BUTTON_4_UP)        m_mouseBtn[3] = false;
        if (m.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)      m_mouseBtn[4] = true;
        if (m.usButtonFlags & RI_MOUSE_BUTTON_5_UP)        m_mouseBtn[4] = false;
        // 휠
        if (m.usButtonFlags & RI_MOUSE_WHEEL)
            m_WheelDelta += static_cast<_short>(m.usButtonData);
    }

}

void CInput_Device::Update()
{        
    m_prevKeys = m_keys;
    m_prevMouse = m_mouseBtn;
    m_WheelDelta = 0;
}

_bool CInput_Device::IsKeyDown(_ushort vkey) const
{
    return (vkey < m_keys.size()) && m_keys[vkey];
}

_bool CInput_Device::IsKeyPressed(_ushort vkey) const
{
    return (vkey < m_keys.size()) && m_keys[vkey] && !m_prevKeys[vkey];
} 

_bool CInput_Device::IsMouseDown(_ulonglong btn) const
{
    return (btn < m_mouseBtn.size()) && m_mouseBtn[btn];
}

_bool CInput_Device::IsMousePressed(_ulonglong btn) const
{
    return (btn < m_mouseBtn.size()) && m_mouseBtn[btn] && !m_prevMouse[btn];
}

POINT CInput_Device::GetMousePos() const
{
    return m_MousePos;
}

LONG CInput_Device::GetMouseWheel() const
{
    return m_WheelDelta;
}

CInput_Device* CInput_Device::Create(HWND hWnd)
{
    CInput_Device* pInstance = new CInput_Device();

    if (FAILED(pInstance->Initialize(hWnd)))
    {
        MSG_BOX("Failed to Created : pInstance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CInput_Device::Free()
{
    __super::Free();
}
