#include "Level_EnmuBoss.h"
#include "GameInstance.h"
#include "JsonLoader.h"
#include "ThirdPersonCamera.h"
#include "BaseCharacter.h"
#include "EnmuMeat.h"
#include "EnmuTentacle.h"
#include "Level_Loading.h"
#include <Weapon.h>

CLevel_EnmuBoss::CLevel_EnmuBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_EnmuBoss::Initialize()
{


	//CBaseCharacter* pTanjiro = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Tanjiro"),
	//	ToIndex(LEVEL::ENMU_BOSS), TEXT("Tanjiro")));

	//if (!pTanjiro)
	//	return E_FAIL;

	CJsonLoader jsonLoader(m_pDevice,m_pContext);
	jsonLoader.Load_Objects("../Asset/Json/EnmuBossCanvas.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});
	jsonLoader.Load_Objects("../Asset/Json/EnmuBossObj.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});
	jsonLoader.Free();
	
	if(FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Characters()))
		return E_FAIL;

	//if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Camera_Free"),
	//	ToIndex(LEVEL::ENMU_BOSS), TEXT("Layer_Camera")))
	//	return E_FAIL;

	

	return S_OK;
}

void CLevel_EnmuBoss::Update(_float fTimeDelta)
{
	if (m_pGameInstance->IsKeyPressed(VK_TAB))
	{
		if (FAILED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
			return;
	}
}

HRESULT CLevel_EnmuBoss::Render()
{
	/*SetWindowText(g_hWnd, TEXT("엔무 보스 레벨"));*/

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

	// Prototype_GameObject_Kyojuro
	CBaseCharacter* pTanjiro = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Tanjiro"),
		ToIndex(LEVEL::GAMEPLAY), TEXT("Tanjiro")));

	if (!pTanjiro)
		return E_FAIL;

	CThirdPersonCamera::THRIDCAMERA_DESC CameraDesc{};
	CameraDesc.fSmoth = 0.1f;
	CameraDesc.pTarget = pTanjiro;
	CameraDesc.fSpeedPerSec = 50.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(180.f);
	CameraDesc.vEye = _float3(0.f, 20.f, -50.f);
	CameraDesc.vAt = _float3(0.f, 0.f, 0.f);
	CameraDesc.fFov = XMConvertToRadians(60.0f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1200.f;

	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_ThirdPersonCamera"),
		ToIndex(LEVEL::GAMEPLAY), TEXT("Layer_Camera"), &CameraDesc))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EnmuBoss::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, 1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vSpecular = _float4(0.4f, 0.4f, 0.4f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
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

}
