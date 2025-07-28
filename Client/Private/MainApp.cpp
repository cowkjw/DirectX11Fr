#include "MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include <JsonLoader.h>
#include "ThirdPersonCamera.h"
#include "DashSmokeEffect.h"
#include "EffectManager.h"
#include "GuardEffect.h"
#include "SoundMag.h"
CMainApp::CMainApp()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CMainApp::Initialize()
{
	ENGINE_DESC			EngineDesc{};

	EngineDesc.hWnd = g_hWnd;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;
	EngineDesc.isWindowed = true;
	EngineDesc.iNumLevels = ToIndex(LEVEL::END);

	if (FAILED(m_pGameInstance->Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Font(TEXT("Demonslayer"), TEXT("../Asset/Resources/Fonts/Demonslayer.spritefont"))))
		return E_FAIL;

	Ready_Static_Prototype();
	if (FAILED(Start_Level(LEVEL::LOGO)))
		return E_FAIL;
	CSoundMag::Get_Instance()->Initialize();
	CSoundMag::Get_Instance()->LoadBanks("../Asset/SoundFmod/Build/Desktop/Master.bank", "../Asset/SoundFmod/Build/Desktop/Master.strings.bank");
	CSoundMag::Get_Instance()->LoadAllBanks("../Asset/SoundFmod/Build/Desktop/");

	return S_OK;
}

void CMainApp::Update(_float fTimeDelta)
{
	//m_pGameInstance->Fixed_Update(fTimeDelta);
	m_pGameInstance->Update_Engine(fTimeDelta);
	CEffectManager::Get_Instance()->Late_Update(fTimeDelta);
	CSoundMag::Get_Instance()->Update();
}

HRESULT CMainApp::Render()
{
	m_pGameInstance->Begin_Draw();
	m_pGameInstance->Draw();

	m_pGameInstance->End_Draw();

	return S_OK;
}


HRESULT CMainApp::Start_Level(LEVEL eStartLevel)
{
	if (FAILED(m_pGameInstance->Change_Level(ToIndex(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, eStartLevel))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Ready_Static_Prototype()
{	
	
	///* For.Prototype_Component_VIBuffer_Rect*/
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	CJsonLoader jsonLoader;
	jsonLoader.Load_Shaders("../Asset/Json/Shaders.json", [&]() {});

	jsonLoader.Load_Textures("../Asset/Json/Textures.json", [&]() {});

	jsonLoader.Free();

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_BoxCollider"),
		CBoxCollider::Create(m_pDevice, m_pContext, _float3(1.f,1.f,1.f)))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_CapsuleCollider"),
		CCapsuleCollider::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_SphereCollider"),
		CSphereCollider::Create(m_pDevice, m_pContext,0.5f))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		CAnimator::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_ThirdPersonCamera */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_GameObject_ThirdPersonCamera"),
		CThirdPersonCamera::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Model_Dash"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/DashSmoke/DashSmoke.bin"))))
		return E_FAIL;

	CParticleSystem::PARTICLE_DESC ParticleDesc{};
	ParticleDesc.iNumInstance = 5000;
	ParticleDesc.vCenter = _float3(64.f, 30.f, 64.0f);
	ParticleDesc.vRange = _float3(128.f, 3.0f, 128.f);
	ParticleDesc.vSize = _float2(0.1f, 0.4f);
	ParticleDesc.vLifeTime = _float2(5.f, 8.f);
	ParticleDesc.vSpeed = _float2(3.f, 5.f);
	ParticleDesc.isLoop = true;
	ParticleDesc.vVelocity = _float3(1.f, 1.f, 1.f);
	ParticleDesc.fSpreadAngle = 180.f;
	ParticleDesc.fGravity = 0.f;
	ParticleDesc.vStartColor = _float3(1.f, 1.f, 1.f);
	ParticleDesc.vEndColor = _float3(0.5f, 1.f, 0.5f);

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
		CParticleSystem::Create(m_pDevice, m_pContext, ParticleDesc))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Model_Gurad"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Common/Guard/Gurad.bin"))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Effect_Guard"),
		CGuardEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	return S_OK;
}

CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);

	m_pGameInstance->Release_Engine();

	Safe_Release(m_pGameInstance);
}
