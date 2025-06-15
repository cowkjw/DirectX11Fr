#include "Level_Enmu.h"
#include "GameInstance.h"
#include "JsonLoader.h"
#include "ThirdPersonCamera.h"
#include "BaseCharacter.h"
#include "EnmuMeat.h"
#include "EnmuTentacle.h"
#include <Weapon.h>

CLevel_Enmu::CLevel_Enmu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Enmu::Initialize()
{



	CJsonLoader jsonLoader(m_pDevice,m_pContext);
	jsonLoader.Load_Objects("../Asset/Json/EnmuBossObj.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});
	jsonLoader.Free();
	
	if(FAILED(Ready_Lights()))
		return E_FAIL;

	//if (FAILED(Ready_Layer_Characters()))
	//	return E_FAIL;

	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Camera_Free"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("Layer_Camera")))
		return E_FAIL;


	if (!m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_KoujuroWeapon"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("Weapon")))
		return E_FAIL;

	CBaseCharacter* pKyojuro = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Kyojuro"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("Kyojuro")));

	if (!pKyojuro)
		return E_FAIL;



	return S_OK;
}

void CLevel_Enmu::Update(_float fTimeDelta)
{
	int a = 10;
}

HRESULT CLevel_Enmu::Render()
{
	SetWindowText(g_hWnd, TEXT("엔무 보스 레벨"));

	return S_OK;
}

HRESULT CLevel_Enmu::Ready_Layer_Characters()
{

	//Prototype_GameObject_EnmuMeat
	
	// Prototype_GameObject_EnmuMeat
	CEnmuMeat* pEnmu = static_cast<CEnmuMeat*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_EnmuMeat"),
		ToIndex(LEVEL::ENMU_BOSS), TEXT("EnmuMeat")));

	if (!pEnmu)
		return E_FAIL;

	//// Prototype_GameObject_Kyojuro
	//CBaseCharacter* pKyojuro = static_cast<CBaseCharacter*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Kyojuro"),
	//	ToIndex(LEVEL::GAMEPLAY), strLayerTag));

	//if (!pKyojuro)
	//	return E_FAIL;

	//pAkaza->Set_Target(TEXT("Kyojuro"), LEVEL::GAMEPLAY);
	//pKyojuro->Set_Target(TEXT("Akaza"), LEVEL::GAMEPLAY);

	return S_OK;
}

HRESULT CLevel_Enmu::Ready_Lights()
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
	LightDesc.vDiffuse = _float4(0.5f, 0.7f, 0.5f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

CLevel_Enmu* CLevel_Enmu::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Enmu* pInstance = new CLevel_Enmu(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Enmu");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Enmu::Free()
{
	__super::Free();

}
