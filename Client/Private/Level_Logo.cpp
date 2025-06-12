#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "BackGround.h"
#include "JsonLoader.h"
#include "GameplayCanvas.h"
#include <UIButton.h>
CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Logo::Initialize()
{
	//if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
	//	return E_FAIL;


 //auto pStartButton = dynamic_cast<CUIButton*>(m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("StartButton")));

 //if (pStartButton)
 //{
	// pStartButton->Set_OnClick([this]() {
	//	 StartGamePlay();
	//	 });
 //}

	CJsonLoader jsonLoader;
	jsonLoader.Load_Objects("../Asset/Json/LogoObjects.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});


	//jsonLoader.Load_Objects("../Asset/Json/StaticCanvas.json", [&]() {
	//	// 이곳에 로드 후 처리할 작업을 추가합니다.
	//	});
	//
	Ready_UI_Setup();
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

	if (m_pGameInstance->IsKeyPressed('F'))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::ENMU_BOSS))))
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


}

HRESULT CLevel_Logo::Render()
{
//	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));
	auto pButton = m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("StartButton"));
	if (pButton)
	{
		auto pStartBt = dynamic_cast<CUIButton*>(pButton);
		if (pStartBt && pStartBt->IsHovered())
		{
			m_pGameInstance->Draw_Font(TEXT("Demonslayer"), TEXT("게임 시작"), _float2(970.f, 440.f), XMVectorSet(0.f,0.f,0.f, 1.f));
		}
		else
		{
			m_pGameInstance->Draw_Font(TEXT("Demonslayer"), TEXT("게임 시작"), _float2(970.f, 440.f), XMVectorSet(1.f, 1.f, 1.f, 1.f));
		}
	}
	
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

    auto pUICanvas = CGameplayCanvas::Create(m_pDevice, m_pContext);
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

void CLevel_Logo::Ready_UI_Setup()
{
	auto pButton = m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("StartButton"));
	if (pButton)
	{
		auto pStartBt = dynamic_cast<CUIButton*>(pButton);
		if (pStartBt)
		{
			pStartBt->Set_OnClick([this]() {
				StartGamePlay();
				});
		}
	}
	auto uiWind = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("WindDeco")));
	if (uiWind)
	{
		uiWind->Set_Color(_float4(0.5f, 0.5f, 0.5f, 1.f)); 

	}
	auto uiEffect = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("Effect")));
	if (uiEffect)
	{
		uiEffect->Set_Color(_float4(1.f,1.f,1.f, 0.5f)); // 반투명하게 설정
	}

	/*auto pInkImage = dynamic_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("StaticCanvas"), TEXT("Ink")));
	if (pInkImage)
	{
		pInkImage->EnableUVAnim(5, 6, 1.f);
	}*/

}

void CLevel_Logo::StartGamePlay()
{
	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::MODE))))
		return;
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
