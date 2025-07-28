#include "HitSlashCrossParticle.h"
#include "TrainSmokeParticle.h"
#include "ThirdPersonCamera.h"
#include "HitShockParticle.h"
#include "Level_EnmuBoss.h"
#include "CutSceneCamera.h"
#include "Level_Loading.h"
#include "BaseCharacter.h"
#include "EffectManager.h"
#include "GameInstance.h"
#include "EnmuTentacle.h"
#include "HitParticle.h"
#include "JsonLoader.h"
#include "CameraMag.h"
#include "EnmuMeat.h"
#include "UIImage.h"
#include "Weapon.h"

CLevel_EnmuBoss::CLevel_EnmuBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_EnmuBoss::Initialize()
{

	CJsonLoader jsonLoader(m_pDevice, m_pContext);
	jsonLoader.Load_Objects("../Asset/Json/EnmuBossCanvas.json", [&]() {}); // UI 로드
	jsonLoader.Load_Objects("../Asset/Json/EnmuBossObj.json", [&]() {}); // 배경 오브젝트 로드

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Characters()))
		return E_FAIL;

	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TrainLight"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("TrainPointLight")))
		return E_FAIL;

	if (FAILED(Ready_Effects(jsonLoader)))
		return E_FAIL;

	jsonLoader.Free();

	// 안개 셰이더 셋팅
	m_pGameInstance->Active_Fog(true);
	m_pGameInstance->SetFogDistance(550.f, 600.f);
	m_pGameInstance->Set_FogColor(_float4(0.5f, 0.5f, 0.5f, 0.4f));


	// 사운드 재생
	CSoundMag::Get_Instance()->PlayBGM("event:/BGM/EnmuBgm");
	CSoundMag::Get_Instance()->PlayEffect("event:/BGM/TrainAmb", "TrainAmb");
	return S_OK;
}

void CLevel_EnmuBoss::Update(_float fTimeDelta)
{
	if (m_pGameInstance->IsKeyPressed(VK_TAB))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BATTLE))))
			return;
	}
	UpdateGameFlow(fTimeDelta);
	CEffectManager::Get_Instance()->Update_ActivedParticle(fTimeDelta);
	CEffectManager::Get_Instance()->ClenUpPendingParticleEffects();
}

HRESULT CLevel_EnmuBoss::Render()
{
	return S_OK;
}

HRESULT CLevel_EnmuBoss::Ready_Layer_Characters()
{
	auto pWeapon = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TanjiroWeapon"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("Weapon"));
	if (!pWeapon)
		return E_FAIL;
	pWeapon->GetTransform()->Scaling(_float3(10.f, 10.f, 10.f));
	//// Prototype_GameObject_EnmuMeat
	CEnmuMeat* pEnmu = static_cast<CEnmuMeat*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_EnmuMeat"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("EnmuMeat")));

	if (!pEnmu)
		return E_FAIL;

	CBaseCharacter* pTanjiro = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Tanjiro"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("Tanjiro")));

	if (!pTanjiro)
		return E_FAIL;
	Ready_Camera(pTanjiro);
	return S_OK;
}

HRESULT CLevel_EnmuBoss::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.7f, 0.9f, 0.7f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(0, 20.f, -150.f, 1.f);
	LightDesc.fRange = 1.f;
	LightDesc.vDiffuse = _float4(1.0f, 0.75f, 0.4f, 1.f);
	LightDesc.fAmbient = 0.5f;
	LightDesc.vSpecular = _float4(0.f, 1.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EnmuBoss::Ready_Effects(CJsonLoader& jsonLoader)
{
	CGameObject* pEnvWind = nullptr;
	if (!(pEnvWind = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_EnvWind"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("EnvWind"))))
		return E_FAIL;

	pEnvWind->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(-126.143f, 41.156f, 0.f, 1.f));


	if (!(pEnvWind = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_EnvWind"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("EnvWind"))))
		return E_FAIL;
	pEnvWind->GetTransform()->Set_State(STATE::POSITION, XMVectorSet(132.143f, 41.156f, 0.f, 1.f));



	m_pTanjiro = static_cast<CBaseCharacter*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("Tanjiro")));
	m_pEnmu = static_cast<CEnmuMeat*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::ENMU_BOSS), TEXT("EnmuMeat")));

	CParticleSystem* pParticleSystem = nullptr;
	CEffect* pEffect = CHitParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/AkazaHit_Particle.json", &pParticleSystem);

	static_cast<CHitParticle*>(pEffect)->AddParticleSystem(L"BodyHit", pParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("TanjiroHitParticle"), pEffect);

	// 타격용
	pEffect = CHitShockParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/HitBodyShock_Particle.json", &pParticleSystem);
	static_cast<CHitShockParticle*>(pEffect)->AddParticleSystem(L"HitShock", pParticleSystem);
	static_cast<CHitShockParticle*>(pEffect)->SetInitParticleUV(4, 4, 0.0001f);
	pEffect->SetTextureIndex(1);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("HitBodyShockParticle"), pEffect);

	pEffect = CHitShockParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/HitGroundShock_Particle.json", &pParticleSystem);
	static_cast<CHitShockParticle*>(pEffect)->AddParticleSystem(L"HitShock", pParticleSystem);
	static_cast<CHitShockParticle*>(pEffect)->SetInitParticleUV(4, 4, 0.05f);
	pEffect->SetTextureIndex(25);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("HitGroundParticle"), pEffect);



	pEffect = CHitSlashCrossParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/HitCross_Particle.json", &pParticleSystem);
	static_cast<CHitSlashCrossParticle*>(pEffect)->AddParticleSystem(L"HitCross", pParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("HitCrossParticle"), pEffect);
	pEffect = CHitShockParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect = CHitShockParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/HitShock_Particle.json", &pParticleSystem);
	static_cast<CHitShockParticle*>(pEffect)->SetInitParticleUV(3, 3, 0.0085f);
	pEffect->SetTextureIndex(8);
	static_cast<CHitShockParticle*>(pEffect)->AddParticleSystem(L"HitShock", pParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("HitShockParticle"), pEffect);

	return S_OK;
}

HRESULT CLevel_EnmuBoss::Ready_Camera(CGameObject* pTarget)
{
	CThirdPersonCamera::THRIDCAMERA_DESC CameraDesc{};
	CameraDesc.fSmoth = 0.1f;
	CameraDesc.pTarget = pTarget;
	CameraDesc.fSpeedPerSec = 50.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(180.f);
	CameraDesc.vEye = _float3(0.f, 20.f, -50.f);
	CameraDesc.vAt = _float3(0.f, 0.f, 0.f);
	CameraDesc.fFov = XMConvertToRadians(60.0f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1200.f;
	CameraDesc.strName = TEXT("ThirdPersonCamera");

	CGameObject* pMainCamera = nullptr;
	if (!(pMainCamera = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_ThirdPersonCamera"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("ThirdPersonCamera"), &CameraDesc)))
		return E_FAIL;

	if (pMainCamera)
	{
		CCameraMag::Get_Instance()->RegisterCamera(TEXT("MainCamera"), static_cast<CCamera*>((pMainCamera)));
		CCameraMag::Get_Instance()->ActiveCamera(TEXT("MainCamera"));
	}
	CGameObject* pCutSceneCamera = nullptr;
	//Prototype_GameObject_CutSceneCam
	if (!(pCutSceneCamera = m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_CutSceneCam"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("CutSceneCamera"), &CameraDesc)))
		return E_FAIL;
	if (pCutSceneCamera)
	{
		CCameraMag::Get_Instance()->RegisterCamera(TEXT("CutSceneCamera"), static_cast<CCamera*>((pCutSceneCamera)));
		CJsonLoader jsonLoader(m_pDevice, m_pContext);
		json j = jsonLoader.Load_CutScene_PropertyAsJson("../Asset/Json/CutScene/EnmuMeat.json");
		CCameraMag::Get_Instance()->RegisterCutSceneProperty(TEXT("EnmuMeatStart"), j);
		j = jsonLoader.Load_CutScene_PropertyAsJson("../Asset/Json/CutScene/EnmuDead.json");
		CCameraMag::Get_Instance()->RegisterCutSceneProperty(TEXT("EnmuMeatDeath"), j);
		jsonLoader.Free();
		CCameraMag::Get_Instance()->ActiveCamera(TEXT("CutSceneCamera"));
		CCameraMag::Get_Instance()->SetCutSceneProperty(TEXT("EnmuMeatStart"));
		auto pCutSceneCam = static_cast<CCutSceneCamera*>(CCameraMag::Get_Instance()->GetActiveCamera());
		pCutSceneCam->SetPlay(true);
	}
	return S_OK;
}

void CLevel_EnmuBoss::UpdateGameFlow(_float fTimeDelta)
{
	UpdateStartState(fTimeDelta);

	CheckCharacterDeath();

	UpdateEndState(fTimeDelta);
}

void CLevel_EnmuBoss::CheckCharacterDeath()
{
	if (m_pTanjiro)
	{
		if (!m_bEndGame && m_pTanjiro->GetState() == CBaseCharacter::CSTATE::DIE)
		{
			m_bEndGame = true;
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StopImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(true);
			}
			CSoundMag::Get_Instance()->StopBGM();
			CSoundMag::Get_Instance()->StopEffect("TrainAmb");
		}
	}

	if (m_pEnmu)
	{
		static _bool shotSound = false;
		if (!m_bEndGame && m_pEnmu->GetState() == EnmuState::DIE)
		{
			m_bEndGame = true;
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StopImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(true);
			}
			CSoundMag::Get_Instance()->StopBGM();
			CSoundMag::Get_Instance()->StopEffect("TrainAmb");
		}
		if (m_pEnmu->GetHp() <= 0.f && !shotSound)
		{
			shotSound = true;
			CSoundMag::Get_Instance()->PlayOneShot("event:/UI/Shobu");
			CSoundMag::Get_Instance()->PlayOneShot("event:/Enmu/Death");
		}
	}
}

void CLevel_EnmuBoss::UpdateStartState(_float fTimeDelta)
{
	if (!m_bEndStartCutScnen)
	{
		auto pCutSceneCam = dynamic_cast<CCutSceneCamera*>(CCameraMag::Get_Instance()->GetActiveCamera());
		if (pCutSceneCam && pCutSceneCam->IsPlaying() == false)
		{
			CCameraMag::Get_Instance()->ActiveCamera(TEXT("MainCamera")); // 메인 카메라로 전환
			auto pUiImage = m_pGameInstance->Get_UI(TEXT("GameplayCanvas"), TEXT("StartImage"));
			if (pUiImage)
			{
				pUiImage->SetActive(true);
				m_bStartGame = true;
			}
			m_bEndStartCutScnen = true;
		}
	}

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
			if (m_pTanjiro)
			{
				m_pTanjiro->SetGameStarted(true);
			}
			if (m_pEnmu)
			{
				m_pEnmu->SetGameStarted(true);
			}
		}
	}
}

void CLevel_EnmuBoss::UpdateEndState(_float fTimeDelta)
{
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

CLevel_EnmuBoss* CLevel_EnmuBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_EnmuBoss* pInstance = new CLevel_EnmuBoss(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_EnmuBoss");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_EnmuBoss::Free()
{
	__super::Free();
	CEffectManager::Get_Instance()->Free();
}
