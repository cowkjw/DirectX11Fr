#include "Level_GamePlay.h"

#include "HitSlashCrossParticle.h"
#include "ThirdPersonCamera.h"
#include "EffectManager.h"
#include "Level_Loading.h"
#include "BaseCharacter.h"
#include "UIProgressBar.h"
#include "GameInstance.h"
#include "HitParticle.h"
#include "JsonLoader.h"
#include "UIImage.h"
#include "Weapon.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	CJsonLoader jsonLoader(m_pDevice,m_pContext);
	jsonLoader.Load_Objects("../Asset/Json/GamePlayCanvas.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});
	jsonLoader.Load_Objects("../Asset/Json/GamePlayBackgroundObj.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});

	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Terrain"),
		ToIndex(LEVEL::GAMEPLAY), TEXT("Layer_BackGround")))
		return E_FAIL;
	if(FAILED(Ready_Lights()))
		return E_FAIL;
	/*if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Sky"),
		ToIndex(LEVEL::GAMEPLAY), TEXT("Sky")))
		return E_FAIL;*/




	//if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Camera_Free"),
	//	ToIndex(LEVEL::GAMEPLAY), TEXT("Layer_Camera")))
	//	return E_FAIL;


	if (FAILED(Ready_Layer_TestCharacter(TEXT("Kyojuro"))))
		return E_FAIL;

	CGameObject* pCharacter = m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Kyojuro"));

	if (pCharacter)
	{
		CThirdPersonCamera::THRIDCAMERA_DESC CameraDesc{};
		CameraDesc.fSmoth = 0.1f;
		CameraDesc.pTarget = pCharacter;
		CameraDesc.fSpeedPerSec = 50.f;
		CameraDesc.fRotationPerSec = XMConvertToRadians(180.f);
		CameraDesc.vEye = _float3(0.f, 20.f, -50.f);
		CameraDesc.vAt = _float3(0.f, 0.f, 0.f);
		CameraDesc.fFov = XMConvertToRadians(60.0f);
		CameraDesc.fNear = 0.1f;
		CameraDesc.fFar = 1000.f;

		if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::STATIC), TEXT("Prototype_GameObject_ThirdPersonCamera"),
			ToIndex(LEVEL::GAMEPLAY), TEXT("Layer_Camera"),&CameraDesc))
			return E_FAIL;
	}


	auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StartImage"));
	if (pUiImage)
	{
		pUiImage->SetActive(true);
		m_bStartGame = true;
	}

	m_pKyojuro = static_cast<CBaseCharacter*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Kyojuro")));
	m_pAkaza = static_cast<CBaseCharacter*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Akaza")));

	CParticleSystem* pParticleSystem = nullptr;
	CEffect* pEffect = CHitParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/KyojuroSlashHit_Particle.json", &pParticleSystem);
	static_cast<CHitParticle*>(pEffect)->AddParticleSystem(L"Slash", pParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("SlashHitParticle"), pEffect);


	pEffect = CHitParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/AkazaHit_Particle.json", &pParticleSystem);

	static_cast<CHitParticle*>(pEffect)->AddParticleSystem(L"BodyHit",pParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("AkazaHitParticle"), pEffect);
	///////////

	// 슬래시용
	pEffect = CHitSlashCrossParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/HitCross_Particle.json", &pParticleSystem);
	static_cast<CHitSlashCrossParticle*>(pEffect)->AddParticleSystem(L"HitCross", pParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("HitCrossParticle"), pEffect);

	jsonLoader.Free();
	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	if (m_pGameInstance->IsKeyPressed(VK_TAB))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BATTLE))))
			return;
	}
	UpdateGameFlow(fTimeDelta);
	CEffectManager::Get_Instance()->Update_ActivedParticle(fTimeDelta);
}

HRESULT CLevel_GamePlay::Render()
{
	//SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));
	CEffectManager::Get_Instance()->ClenUpPendingParticleEffects();
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_TestCharacter(const _wstring strLayerTag)
{

 	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_KoujuroWeapon"),
		ToIndex(LEVEL::GAMEPLAY), TEXT("Weapon")))
		return E_FAIL;
	// Prototype_GameObject_Akaza
	CBaseCharacter* pAkaza = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Akaza"),
		ToIndex(LEVEL::GAMEPLAY), TEXT("Akaza")));

	if (!pAkaza)
		return E_FAIL;

	// Prototype_GameObject_Kyojuro
	CBaseCharacter* pKyojuro = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Kyojuro"),
		ToIndex(LEVEL::GAMEPLAY), strLayerTag));

	if (!pKyojuro)
		return E_FAIL;

	pAkaza->Set_Target(TEXT("Kyojuro"), LEVEL::GAMEPLAY);
	pKyojuro->Set_Target(TEXT("Akaza"), LEVEL::GAMEPLAY);

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{

	LIGHT_DESC			LightDesc{};

	//LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	//LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	//LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	//LightDesc.fAmbient = 0.4f;
	//LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	//if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
	//	return E_FAIL;


	//LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	//LightDesc.vDirection = _float4(1.f, 1.f, 1.f, 0.f);
	//LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	//LightDesc.fAmbient = 1.f;
	//LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);


	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, 1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.3f, 0.3f, 0.45f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

void CLevel_GamePlay::UpdateGameFlow(_float fTimeDelta)
{
	
	if (m_bStartGame)
	{
		m_fStartImageElapsedTime += fTimeDelta;
		if (m_fStartImageElapsedTime >= m_fStartImageTime)
		{
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StartImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(false);
			}
			m_bStartGame = false;
			if (m_pKyojuro)
			{
				m_pKyojuro->SetGameStarted(true);
			}
			if (m_pAkaza)
			{
				m_pAkaza->SetGameStarted(true);
			}
		}
	}

	if (m_pKyojuro)
	{
		if (m_pKyojuro->GetState() == CBaseCharacter::CSTATE::DIE)
		{
			m_bEndGame = true;
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StopImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(true);
			}

		}
	}
	 if (m_pAkaza)
	{
		if (m_pAkaza->GetState() == CBaseCharacter::CSTATE::DIE)
		{
			m_bEndGame = true;
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StopImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(true);
			}
		}
	}

	if (m_bEndGame)
	{
		m_fStopImageElapsedTime += fTimeDelta;
		if (m_fStopImageElapsedTime >= m_fStopImageTime)
		{
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StopImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(false);
				auto pFinImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("FinImage"));
				if (pFinImage)
				{
					pFinImage->SetActive(true);
				}
			}
			m_bIsGameOver = true;
		}
	}

	if (m_bIsGameOver)
	{
		m_fFinalImageElapsedTime += fTimeDelta;
		if (m_fFinalImageElapsedTime >= m_fFinalImageTime)
		{
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("FinImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(false);
			}
			m_bIsGameOver = false;
			if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
				CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BATTLE))))
				return;
		}
	}
}

CLevel_GamePlay* CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_GamePlay* pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_GamePlay");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_GamePlay::Free()
{
	__super::Free();

}
