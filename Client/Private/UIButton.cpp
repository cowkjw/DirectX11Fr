#include "UIButton.h"

void CUIButton::CheckMouseOver()
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);

    _float left = m_fX - m_fSizeX * 0.5f;
    _float right = m_fX + m_fSizeX * 0.5f;
    _float top = m_fY - m_fSizeY * 0.5f;
    _float bottom = m_fY + m_fSizeY * 0.5f;

    m_bHovered =  (pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom);
}
