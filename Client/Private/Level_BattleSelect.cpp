#include "Level_BattleSelect.h"
#include "ThirdPersonCamera.h"
#include "Level_Loading.h"
#include "BaseCharacter.h"
#include "GameInstance.h"
#include "JsonLoader.h"
#include "UIButton.h"
#include "UIImage.h"

CLevel_BattleSelect::CLevel_BattleSelect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_BattleSelect::Initialize()
{
	CJsonLoader jsonLoader(m_pDevice,m_pContext);
	jsonLoader.Load_Objects("../Asset/Json/BattleSelectCanvas.json", [&]() {});
	jsonLoader.Free();
	SetUpUI();

	CSoundMag::Get_Instance()->PlayBGM("event:/BGM/SelectLevel");
	return S_OK;
}

void CLevel_BattleSelect::Update(_float fTimeDelta)
{
	auto pButton = m_pGameInstance->Get_UI(TEXT("BattleSelectCanvas"), TEXT("AkazaButton"));
	auto pCursorImage = m_pGameInstance->Get_UI(TEXT("BattleSelectCanvas"), TEXT("AkazaHover"));
	if (auto pAkazaButton = dynamic_cast<CUIButton*>(pButton))
	{
		if (pAkazaButton->IsHovered())
		{
			if (pCursorImage&&!pCursorImage->IsActive())
			{
				CSoundMag::Get_Instance()->StopEffect("EnmuSelect");
				CSoundMag::Get_Instance()->PlayEffect("event:/UI/AkazaSelect","AkazaSelect");

				pCursorImage->SetActive(true);
			}
			pAkazaButton->GetButtonImage()->SetActive(false);
		}
		else
		{
			if (pCursorImage)
			{
				pCursorImage->SetActive(false);
			}
			pAkazaButton->GetButtonImage()->SetActive(true);
		}
	}

	auto pButton2 = m_pGameInstance->Get_UI(TEXT("BattleSelectCanvas"), TEXT("EnmuButton"));
	auto pCursorImage2 = m_pGameInstance->Get_UI(TEXT("BattleSelectCanvas"), TEXT("EnmuHover"));
	if (auto pEnmuButton = dynamic_cast<CUIButton*>(pButton2))
	{
		if (pEnmuButton->IsHovered())
		{
			if (pCursorImage2&&!pCursorImage2->IsActive())
			{
				CSoundMag::Get_Instance()->StopEffect("AkazaSelect");
				CSoundMag::Get_Instance()->PlayEffect("event:/UI/EnmuSelect","EnmuSelect");
				pCursorImage2->SetActive(true);
			}
			pEnmuButton->GetButtonImage()->SetActive(false);
		}
		else
		{
			if (pCursorImage2)
			{
				pCursorImage2->SetActive(false);
			}
			pEnmuButton->GetButtonImage()->SetActive(true);
		}
	}
}

HRESULT CLevel_BattleSelect::Render()
{
	SetWindowText(g_hWnd, TEXT("배틀 셀렉 레벨"));

	return S_OK;
}

void CLevel_BattleSelect::SetUpUI()
{
	auto pEnmuBt = m_pGameInstance->Get_UI(TEXT("BattleSelectCanvas"), TEXT("EnmuButton"));
	if (pEnmuBt)
	{
		auto pButton = dynamic_cast<CUIButton*>(pEnmuBt);
		if (pButton)
		{
			pButton->Set_OnClick([this]() {
				StartEnmu();
				CSoundMag::Get_Instance()->PlayEffect("event:/UI/BattleStart");
				});
		}
	}

	auto pAkazaBt = m_pGameInstance->Get_UI(TEXT("BattleSelectCanvas"), TEXT("AkazaButton"));
	if (pAkazaBt)
	{
		auto pButton = dynamic_cast<CUIButton*>(pAkazaBt);
		if (pButton)
		{
			pButton->Set_OnClick([this]() {
				StartAkaza();
				CSoundMag::Get_Instance()->PlayEffect("event:/UI/BattleStart");
				});
		}
	}
}

void CLevel_BattleSelect::StartAkaza()
{
	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
		return;
}

void CLevel_BattleSelect::StartEnmu()
{
	if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
		CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::ENMU_BOSS))))
		return;
}


CLevel_BattleSelect* CLevel_BattleSelect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_BattleSelect* pInstance = new CLevel_BattleSelect(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_BattleSelect");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_BattleSelect::Free()
{
	__super::Free();
}
