#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "BackGround.h"
#include "TitleCanvas.h"
CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Logo::Initialize()
{
	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{

	if (m_pGameInstance->IsKeyPressed(VK_SPACE))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
			return;
	}

	if (m_pGameInstance->IsKeyPressed('E'))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::EDITOR))))
			return;
	}

	POINT pt = m_pGameInstance->GetMousePos();

	{
		char buf[64];
		// 포맷팅: 변수 pt.x, pt.y를 문자열에 삽입
		sprintf_s(buf, "MousePos: %d, %d", pt.x, pt.y);
		// 윈도우 타이틀(또는 컨트롤)에 출력
		SetWindowTextA(g_hWnd, buf);
	}

	int a = 0;

}

HRESULT CLevel_Logo::Render()
{
//	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));

	return S_OK;
}

HRESULT CLevel_Logo::Ready_Layer_BackGround(const _wstring strLayerTag)
{

    CUICanvas::UIOBJECT_DESC CanvasDesc{};
    CanvasDesc.fX = g_iWinSizeX * 0.5f;
    CanvasDesc.fY = g_iWinSizeY * 0.5f;
    CanvasDesc.fSizeX = g_iWinSizeX;
    CanvasDesc.fSizeY = g_iWinSizeY;
    CanvasDesc.strName = L"TitleCanvas";

    auto pUICanvas = CTitleCanvas::Create(m_pDevice, m_pContext);
    if (!pUICanvas)
        return E_FAIL;

    if (FAILED(pUICanvas->Initialize(&CanvasDesc)))
    {
        Safe_Release(pUICanvas);
        return E_FAIL;
    }
    m_pGameInstance->AddCanvasUI(pUICanvas);

    return S_OK;
}


CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Logo");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Logo::Free()
{
	__super::Free();

}
