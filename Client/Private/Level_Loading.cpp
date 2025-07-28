#include "Level_BattleSelect.h"
#include "Level_Loading.h"
#include "Level_EnmuBoss.h"
#include "Level_GamePlay.h"
#include "GameInstance.h"
#include "Level_Editor.h"
#include "Level_Logo.h"
#include "Level_Mode.h"
#include "GameObject.h"
#include "JsonLoader.h"
#include "UIImage.h"
#include "Loader.h"

_float4 CLevel_Loading::m_vInitShojiOrigin[2]{};
CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
	static _bool bIsLoading = true;
	if (bIsLoading)
	{
		bIsLoading = false;
		CJsonLoader jsonLoader;
		jsonLoader.Load_Objects("../Asset/Json/LodingCanvas.json", [&]() {});
		jsonLoader.Free();
		auto pLoadAnim = m_pGameInstance->Get_UI(TEXT("LodingCanvas"), TEXT("LodingAnim"));
		if (pLoadAnim)
		{
			CUIImage* pLoadingAnim = static_cast<CUIImage*>(pLoadAnim);
			pLoadingAnim->EnableUVAnim(2, 3, 0.065f);
		}
	}
	m_eNextLevelID = eNextLevelID;

	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);
	if (nullptr == m_pLoader)
		return E_FAIL;
	m_pGameInstance->ClearColliders();
	CSoundMag::Get_Instance()->StopAll(true);

	static _bool firstLoad = true;
	if (firstLoad)
	{
		firstLoad = false;
	}
	SetUpUI();
	m_fShojiTime = 0.f;
	return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
	static _bool bIsFirst = true;
	m_pGameInstance->ClearUI();

	if (true == m_pLoader->isFinished())
	{
		CLevel* pLevel = { nullptr };
		m_pGameInstance->ClearLights();
		switch (m_eNextLevelID)
		{
		case LEVEL::LOGO:
			pLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::GAMEPLAY:
			pLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::EDITOR:
			pLevel = CLevel_Editor::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::ENMU_BOSS:
			pLevel = CLevel_EnmuBoss::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::MODE:
			pLevel = CLevel_Mode::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::BATTLE:
			pLevel = CLevel_BattleSelect::Create(m_pDevice, m_pContext);
			break;
		}

		if (nullptr == pLevel)
			return;

		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(m_eNextLevelID), pLevel)))
			return;

		if (m_eNextLevelID == LEVEL::EDITOR)
		{
			m_pGameInstance->GetCanvasUI(TEXT("LodingCanvas"))->SetActive(false);
		}
		else
		{
			m_pGameInstance->GetCanvasUI(TEXT("LodingCanvas"))->SetActive(false);
		}
		bIsFirst = true;
	}
	else
	{
		if (bIsFirst)
		{
			auto pLoadingCavnas = m_pGameInstance->GetCanvasUI(TEXT("LodingCanvas"));
			if (pLoadingCavnas)
			{
				CSoundMag::Get_Instance()->PlayOneShot("event:/UI/Loading");
				pLoadingCavnas->SetActive(true);
			}
			bIsFirst = false;
		}
		MoveCloud(fTimeDelta);
		UpdateShojiOpen(fTimeDelta);
	}
}

HRESULT CLevel_Loading::Render()
{
	m_pLoader->Output_LoadingText();

	return S_OK;
}

void CLevel_Loading::MoveCloud(_float fTimeDelta)
{
	m_fCloudTime += fTimeDelta;

	// 진폭과 속도
	const _float ampDark = 5.f;            // 다크 구름 진폭
	const _float ampNormal = 10.f;         // 일반 구름 진폭
	const _float speed = 0.2f;            // 주기 조절 

	// 0~2파이 속도로 시간에 따라 변화
	_float phase = m_fCloudTime * XM_2PI * speed;

	// Dark 구름 offset: sin * ampDark
	_float offsetDark = sinf(phase) * ampDark;
	// 일반 구름은 180도 위상차
	_float offsetNormal = sinf(phase + XM_PI) * ampNormal;

	auto apply = [&](CUIImage* cloud, const _vector& origin, _float offsetX)
		{
			if (!cloud) return;
			_vector pos = origin;
			pos = XMVectorSetX(pos, XMVectorGetX(origin) + offsetX);
			cloud->GetTransform()->Set_State(STATE::POSITION, pos);
		};

	auto uiL = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("LodingCanvas"), TEXT("CloudLeft")));
	auto uiR = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("LodingCanvas"), TEXT("CloudRight")));

	apply(uiL, m_vCloudOrigin[2], offsetNormal);
	apply(uiR, m_vCloudOrigin[3], -offsetNormal);
}

void CLevel_Loading::SetUpUI()
{

	auto uiL = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("LodingCanvas"), TEXT("CloudLeft")));
	auto uiR = static_cast<CUIImage*>(m_pGameInstance->Get_UI(TEXT("LodingCanvas"), TEXT("CloudRight")));


	if (uiL)     m_vCloudOrigin[2] = uiL->GetTransform()->Get_State(STATE::POSITION);
	if (uiR)     m_vCloudOrigin[3] = uiR->GetTransform()->Get_State(STATE::POSITION);


	auto L = static_cast<CUIImage*>(m_pGameInstance->Get_UI(L"LodingCanvas", L"ShojiLeft"));
	auto R = static_cast<CUIImage*>(m_pGameInstance->Get_UI(L"LodingCanvas", L"ShojiRight"));

	// 좌우 반원 마스크 적용
	// 열리는 시간, 오프셋, 스케일 설정
	if (L) 
	{ 
		L->EnableMask(L"Mask_Shoji_L");
		L->SetMaskParams(0.5f, { 0,0 }, { 1.f,1.f });
	}
	if (R)
	{ 
		R->EnableMask(L"Mask_Shoji_R");
		R->SetMaskParams(0.5f, { 0,0 }, { 1.f,1.f });
	}

	static _bool bIsFirst = true;
	if (bIsFirst)
	{
		if (L)
		{
			XMStoreFloat4(&m_vInitShojiOrigin[0], L->GetTransform()->Get_State(STATE::POSITION));
		}
		if (R)
		{
			XMStoreFloat4(&m_vInitShojiOrigin[1], R->GetTransform()->Get_State(STATE::POSITION));
		}
		bIsFirst = false;
	}
	else
	{
		if (L)
		{
			L->GetTransform()->Set_State(STATE::POSITION, XMLoadFloat4(&m_vInitShojiOrigin[0]));
		}
		if (R)
		{
			R->GetTransform()->Set_State(STATE::POSITION, XMLoadFloat4(&m_vInitShojiOrigin[1]));
		}
	}

	if (L)
	{
		m_vShojiOrigin[0] = L->GetTransform()->Get_State(STATE::POSITION);

	}
	if (R)
	{
		m_vShojiOrigin[1] = R->GetTransform()->Get_State(STATE::POSITION);
	}
}

void CLevel_Loading::UpdateShojiOpen(_float fTimeDelta)
{

	const _float slideDist = 340.f;
	m_fShojiTime = min(m_fShojiTime + fTimeDelta, m_fShojiDuration);
	_float progress = m_fShojiTime / m_fShojiDuration;
	_float offsetX = progress;


	//  좌측은 왼쪽으로, 우측은 오른쪽으로
	auto L = static_cast<CUIImage*>(m_pGameInstance->Get_UI(L"LodingCanvas", L"ShojiLeft"));
	auto R = static_cast<CUIImage*>(m_pGameInstance->Get_UI(L"LodingCanvas", L"ShojiRight"));

	if (L)
	{
		L->SetMaskParams(
			0.5f,                    // threshold
			{ _float2(-offsetX, 0) }, // uvOffset.x = 1→0
			{ _float2(1.f, 1.f) }    // uvScale 고정
		);
		_vector posL = m_vShojiOrigin[0];
		posL = XMVectorSetX(posL, XMVectorGetX(m_vShojiOrigin[0]) - slideDist * progress);
		L->GetTransform()->Set_State(STATE::POSITION, posL);
	}
	if (R)
	{
		R->SetMaskParams(
			0.5f,                    // threshold
			{ _float2(offsetX, 0) }, // uvOffset.x = 1→0
			{ _float2(1.f, 1.f) }    // uvScale 고정
		);
		_vector posR = m_vShojiOrigin[1];
		posR = XMVectorSetX(posR, XMVectorGetX(m_vShojiOrigin[1]) + slideDist * progress);
		R->GetTransform()->Set_State(STATE::POSITION, posR);
	}
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : CLevel_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Loading::Free()
{
	__super::Free();

	Safe_Release(m_pLoader);
}
