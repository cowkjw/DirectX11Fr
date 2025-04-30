#include "Input_Device.h"

HRESULT CInput_Device::Initialize(HWND hWnd)
{
    // DirectXTK 키보드 객체 생성 및 트래커 초기화
    m_keyboard = new Keyboard();
    m_kbdTracker = new Keyboard::KeyboardStateTracker();

    // DirectXTK 마우스 객체 생성, 윈도우 핸들 연결 및 트래커 초기화
    m_mouse = new Mouse();
    m_mouse->SetWindow(hWnd);
    m_mouseTracker = new Mouse::ButtonStateTracker();

    return S_OK;
}

void CInput_Device::Update()
{
    // 키보드 상태를 가져와 트래커에 반영
    auto kbState = m_keyboard->GetState();
    m_kbdTracker->Update(kbState);

    // 마우스 상태를 가져와 트래커에 반영
    auto msState = m_mouse->GetState();
    m_mouseTracker->Update(msState);
}

_bool CInput_Device::IsKeyDown(Keyboard::Keys key) const
{
    // 키가 현재 눌려 있는지 반환
	return m_keyboard->GetState().IsKeyDown(key);
}

_bool CInput_Device::IsKeyPressed(Keyboard::Keys key) const
{
    // 키를 이번 프레임에 "딱" 눌렀을 때만 true 반환
	return m_kbdTracker->IsKeyPressed(key);
}

POINT CInput_Device::GetMousePos() const
{
    auto ms = m_mouse->GetState();
    return { static_cast<LONG>(ms.x), static_cast<LONG>(ms.y) };
}

//_bool CInput_Device::IsMouseButtonDown(Mouse::State state) const
//{
//    auto ms = m_mouse->GetState();    // Mouse::State
//	return ms.leftButton == state.leftButton
//        || ms.rightButton
//        || ms.middleButton;
//}

CInput_Device* CInput_Device::Create(HWND hWnd)
{
    CInput_Device* pInstance = new CInput_Device();
    if (FAILED(pInstance->Initialize(hWnd)))
    {
        Safe_Release(pInstance);
        return nullptr;
    }
    return pInstance;
}

void CInput_Device::Free()
{
    __super::Free();
	Safe_Delete(m_mouse);
	Safe_Delete(m_keyboard);
	Safe_Delete(m_mouseTracker);
	Safe_Delete(m_kbdTracker);
}
