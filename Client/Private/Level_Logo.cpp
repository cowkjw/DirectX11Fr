#include "Level_Logo.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "BackGround.h"
#include "JsonLoader.h"
#include <UIButton.h>
CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Logo::Initialize()
{

	CJsonLoader jsonLoader;
	jsonLoader.Load_Objects("../Asset/Json/LogoObjects.json", [&]() {});

	Ready_UI_Setup();
	CSoundMag::Get_Instance()->PlayBGM("event:/BGM/TitleBGM");
	CSoundMag::Get_Instance()->PlayEffect("event:/UI/TtitleLogo");
	return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{

	//if (m_pGameInstance->IsKeyPressed(VK_SPACE))
	//{
	//	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
	//		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
	//		return;
	//}

	// 에디터로 넘기기
	if (m_pGameInstance->IsKeyPressed('E'))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::EDITOR))))
			return;
	}

	//if (m_pGameInstance->IsKeyPressed('F'))
	//{
	//	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
	//		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::ENMU_BOSS))))
	//		return;
	//}
	UpdateLogoImage(fTimeDelta);

}

HRESULT CLevel_Logo::Render()
{
	UpdateLogoButtonFont();
	
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

	auto logoImage = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("Logo")));
	if (logoImage)
	{
		logoImage->GetTransform()->Scaling(_float3(300.f, 300.f, 1.f)); // 크기 조정
	}

}

void CLevel_Logo::StartGamePlay()
{
	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::MODE))))
		return;
}

void CLevel_Logo::UpdateLogoImage(_float fTimeDelta)
{
	auto logoImage = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("Logo")));
	if (logoImage)
	{
		_float3 scale = logoImage->GetTransform()->Get_Scaled();
		static _bool bScaleUp = true;
		if (m_fMaxScale > scale.x && bScaleUp)
		{
			scale.x += 500.f * fTimeDelta;
			scale.y += 500.f * fTimeDelta;
			logoImage->GetTransform()->Scaling(scale);
		}
		else
		{
			bScaleUp = false;
		}
		if (!bScaleUp)
		{
			if (m_fFinalScale < scale.x)
			{
				scale.x -= 300.f * fTimeDelta;
				scale.y -= 300.f * fTimeDelta;
				logoImage->GetTransform()->Scaling(scale);
			}
		}
	}
}

void CLevel_Logo::UpdateLogoButtonFont()
{
	auto pButton = m_pGameInstance->Get_UI(TEXT("TitleCanvas"), TEXT("StartButton"));
	if (pButton)
	{
		auto pStartBt = dynamic_cast<CUIButton*>(pButton);
		if (pStartBt && pStartBt->IsHovered())
		{
			m_pGameInstance->Draw_Font(TEXT("Demonslayer"), TEXT("게임 시작"), _float2(990.f, 440.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));
		}
		else
		{
			m_pGameInstance->Draw_Font(TEXT("Demonslayer"), TEXT("게임 시작"), _float2(990.f, 440.f), XMVectorSet(1.f, 1.f, 1.f, 1.f));
		}
	}
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
