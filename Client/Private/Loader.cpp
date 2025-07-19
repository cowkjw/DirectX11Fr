#include "Loader.h"

#include "KyojuroEnkSpiralEffect.h"
#include "HitSlashCrossParticle.h"
#include "EnmuSwingWindEffect.h"
#include "EnmuPunchWindEffect.h"
#include "TanNejVortexEffect.h"
#include "ThirdPersonCamera.h"	
#include "KyojuroKienEffect.h"
#include "AkaKuBulletEffect.h"
#include "TanNejWindEffect.h"
#include "TanMigRingEffect.h"
#include "KyojuroNobEffect.h"
#include "KyojuroEnkEffect.h"
#include "WarningZoneDecal.h"
#include "DashSmokeEffect.h"
#include "TrainPointLight.h"
#include "WindSlashEffect.h"
#include "FireSlashEffect.h"
#include "RingWindEffect.h"
#include "CutSceneCamera.h"
#include "ParticleEffect.h"
#include "AkaFistEffect.h"
#include "EnvWindEffect.h"
#include "GameInstance.h"
#include "EnmuTentacle.h"
#include "TanTakEffect.h"
#include "TanMigEffect.h"
#include "HitParticle.h"
#include "Environment.h"
#include "GuardEffect.h"
#include "SlashEffect.h"
#include "TanjiroMig.h"
#include "KyojuroEnk.h"
#include "FreeCamera.h"
#include "BackGround.h"
#include "Navigation.h"
#include "JsonLoader.h"
#include "UICanvas.h"
#include "EnmuMeat.h"
#include "Tanjiro.h"
#include "Kyojuro.h"
#include "Terrain.h"
#include "Weapon.h"
#include "Akaza.h"
#include "Sky.h"


//#include "player.h"
//#include "Effect.h"
//#include "Sky.h"

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

_uint APIENTRY LoadingMain(void* pArg)
{
	/* 자원로딩한다. */
	CLoader* pLoader = static_cast<CLoader*>(pArg);

	if (FAILED(pLoader->Loading()))
		return 1;

	return 0;

}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_CriticalSection);

	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Loading()
{
	if (FAILED(CoInitializeEx(nullptr, 0)))
		return E_FAIL;

	EnterCriticalSection(&m_CriticalSection);


	HRESULT		hr = {};


	switch (m_eNextLevelID)
	{
	case LEVEL::LOGO:
		hr = Loading_For_Logo();
		break;

	case LEVEL::GAMEPLAY:
		hr = Loading_For_GamePlay();
		break;
	case LEVEL::EDITOR:
		hr = Loading_For_Editor();
		break;
	case LEVEL::ENMU_BOSS:
		hr = Loading_For_EnmuBoss();
	case LEVEL::MODE:
		hr = Loading_For_Mode();
		break;
	case LEVEL::BATTLE:
		hr = Loading_For_Battle();
		break;
	}
	LeaveCriticalSection(&m_CriticalSection);

	CoUninitialize();

	if (FAILED(hr))
		return E_FAIL;
	//static _bool bIsLoading = true;
	//if (bIsLoading)
	//{
	//	bIsLoading = false;
	//	CJsonLoader jsonLoader;
	//	jsonLoader.Load_Objects("../Asset/Json/LodingCanvas.json", [&]() {
	//		// 이곳에 로드 후 처리할 작업을 추가합니다.
	//		});
	//	jsonLoader.Free();
	//	auto pLoadAnim = m_pGameInstance->Get_UI(TEXT("LodingCanvas"), TEXT("LodingAnim"));
	//	if (pLoadAnim)
	//	{
	//		CUIImage* pLoadingAnim = static_cast<CUIImage*>(pLoadAnim);
	//		pLoadingAnim->EnableUVAnim(2, 3, 0.08f);
	//	}
	//}
	//if (m_eNextLevelID == LEVEL::MODE)
	//{
	//	CJsonLoader jsonLoader;

	//	jsonLoader.Load_Objects("../Asset/Json/InkCanvas.json", [&]() {
	//		// 이곳에 로드 후 처리할 작업을 추가합니다.
	//		});

	//	auto pInkAnim = m_pGameInstance->Get_UI(TEXT("InkCanvas"), TEXT("InkMask"));
	//	if (pInkAnim)
	//	{
	//		pInkAnim->SetActive(true);
	//		CUIImage* pInkFade = static_cast<CUIImage*>(pInkAnim);
	//		pInkFade->EnableUVAnim(5, 6, 0.066f);
	//		pInkFade->SetShaderPass(1);
	//	}
	//}

	return S_OK;
}

HRESULT CLoader::Loading_For_Logo()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));
	///* For.Prototype_Component_Texture_BackGround*/
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::LOGO), TEXT("Prototype_Component_Texture_BackGround"),
	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Asset/Resources/Textures/Lang_Title_Char.png")))))
	//	return E_FAIL;

	//m_pGameInstance->LoadTexture(TEXT("TitleBack"), TEXT("../Asset/Resources/Textures/Title/Lang_Title_Char.png"),true);
 //	m_pGameInstance->LoadTexture(TEXT("TitleEffect"), TEXT("../Asset/Resources/Textures/Title/Back_Eff.png"),true);
 //	m_pGameInstance->LoadTexture(TEXT("TitleLogo"), TEXT("../Asset/Resources/Textures/Title/TitleLogo.png"),true);




	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));

	 
	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	///* For.Prototype_GameObject_BackGround */
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::LOGO), TEXT("Prototype_GameObject_BackGround"),
	//	CBackGround::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_LogoCanvas */
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::LOGO), TEXT("Prototype_GameObject_LogoCanvas"),
	//	CUICanvas::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_GamePlay()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));
	/* For.Prototype_Component_Texture_Terrain */
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Terrain"),
	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Asset/Resources/Textures/Terrain/Tile%d.dds"), 2))))
	//	return E_FAIL;
	CJsonLoader jsonLoader;

	jsonLoader.Load_Models("../Asset/Json/Models.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});
	jsonLoader.Free();

	m_pGameInstance->LoadTexture(TEXT("Terrain"), TEXT("../Asset/Resources/Textures/Terrain/Tile%d.dds"), true,3);

	_matrix		PreTransformMatrix = XMMatrixIdentity();

	/* For.Prototype_Component_Model_KoujuroWeapon */
//	PreTransformMatrix = XMMatrixRotationX(XMConvertToRadians(.f));
	PreTransformMatrix = XMMatrixRotationX(XMConvertToRadians(70.f)) * XMMatrixRotationZ(XMConvertToRadians(30.f))* XMMatrixTranslation(0.5f,2.7f,0.f);
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_KoujuroWeapon"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Weapon/KoujuroWeapon.bin", PreTransformMatrix))))
		return E_FAIL;
	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));





	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kyoujuro"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Kyoujuro/Kyoujuro.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Akaza"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Akaza/Akaza.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Sky"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Map/Sky.bin", PreTransformMatrix))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixRotationY(XMConvertToRadians(180.f));// *XMMatrixRotationZ(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_FireSlash2"),
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/SM_e_Plc_P0012_Slash001.fbx", PreTransformMatrix))))
		return E_FAIL;
	PreTransformMatrix = XMMatrixScaling(100.f, 100.f, 100.f) * XMMatrixRotationY(XMConvertToRadians(180.f)) * XMMatrixRotationZ(XMConvertToRadians(180.f));
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_DefaultSlash"),
	//	CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/DefaultSlash/SM_e_Plc_P0012_Slash001.fbx", PreTransformMatrix))))
	//	return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_DefaultSlash"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/DefaultSlash/DefaultSlash.bin", PreTransformMatrix))))
		return E_FAIL;
	//PreTransformMatrix = XMMatrixScaling(200.f, 200.f, 200.f);// *XMMatrixRotationX(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_WindSlash"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Akaza/WindSlash2.bin"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_EnkFire"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Enk/Tiger.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_EnkFireSpiral"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Enk/EnkSpiral.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kien"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Kien.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Nob"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Nob/Nob.bin", PreTransformMatrix))))
		return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_Nob"),
	//	CKyojuroNobEffect::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	CVIBuffer_Terrain::TERRAIN_DESC desc{};
	desc.fX = 512.f;
	desc.fZ = 512.f;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, desc))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Envirnoment"),
		CEnvironment::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	//lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));
	/* For.Prototype_GameObject_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_Slash"),
		CSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_WindSlash"),
		CWindSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_FireSlash"),
		CFireSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	//KyojuroEnkEffect

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_EnkFire"),
		CKyojuroEnkEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Kyojuro */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Kyojuro"),
		CKyojuro::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Akaza */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Akaza"),
		CAkaza::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For.Prototype_GameObject_KoujuroWeapon */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_KoujuroWeapon"),
		CWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;


		/* Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Navigation"),
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../Asset/NavMesh/NavMeshGamePlay.dat")))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_KuuBullet"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Akaza/Kuu/Kuu.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fist"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Akaza/Fist/Fist.bin", PreTransformMatrix))))
		return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_AkaFist"),
	//	CAkaFistEffect::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_RingWind"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Common/RingWind.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_RingWind"),
		CRingWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Editor()
{
	lstrcpy(m_szLoadingText, TEXT("로딩중입니다."));
	CJsonLoader jsonLoader(m_pDevice,m_pContext);
	//jsonLoader.Load_Textures("../Asset/Json/Textures.json", [&]() {
	//	// 이곳에 로드 후 처리할 작업을 추가합니다.
	//	});

	jsonLoader.Load_Models("../Asset/Json/Models.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});


	jsonLoader.Free();


	CVIBuffer_Terrain::TERRAIN_DESC desc{};
	desc.fX = 256.f;
	desc.fZ = 256.f;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, desc))))
		return E_FAIL;

	m_pGameInstance->LoadTexture(TEXT("Terrain"), TEXT("../Asset/Resources/Textures/Terrain/Tile%d.dds"), true, 2);

	/* For.Prototype_GameObject_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For.Prototype_GameObject_Camera_Free */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_Free"),
		CFreeCamera::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	_matrix		PreTransformMatrix = XMMatrixIdentity();

	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kyoujuro"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Kyoujuro/Kyoujuro.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Akaza"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Akaza/Akaza.bin", PreTransformMatrix))))
		return E_FAIL;

	//Asset\Resources\Models\Effect\Kyo
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_FireSlash2"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/SM_e_Plc_P0012_Slash001.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_DefaultSlash"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/DefaultSlash/DefaultSlash.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_WindSlash"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Akaza/WindSlash2.bin"))))
		return E_FAIL;

	/*"Prototype_Component_Model_Tanjiro"*/
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Tanjiro"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Tanjiro/Tanjiro.bin", PreTransformMatrix))))
		return E_FAIL;


	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));

	/* For.Prototype_GameObject_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Particle_HitPoint"),
		CHitParticle::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Particle_SlashCross"),
		CHitSlashCrossParticle::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For.Prototype_GameObject_Akaza */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Akaza"),
		CAkaza::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For.Prototype_GameObject_KoujuroWeapon */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_KoujuroWeapon"),
		CWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Kyojuro"),
		CKyojuro::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Envirnoment"),
		CEnvironment::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Envirnoment"),
		CEnvironment::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Tanjiro"),
		CTanjiro::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Sky"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Map/Sky.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuLeftArm"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/LeftArm/EnmuLeftArm.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuRightArm"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/RightArm/EnmuRightArm.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Body"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/Body/EnmuBossBody.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuHead"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/Head/EnmuBossHead.bin", PreTransformMatrix))))
		return E_FAIL;


	/* For.Prototype_GameObject_EnmuMeat */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_EnmuMeat"),
		CEnmuMeat::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TanjiroWeapon"),
		CWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_FireSlash"),
		CFireSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;



	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_Slash"),
		CSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_WindSlash"),
		CWindSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	///* Prototype_Component_Navigation */
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Navigation"),
	//	CNavigation::Create(m_pDevice, m_pContext, nullptr))))
	//	return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"),
		CWarningZoneDecal::Create(m_pDevice, m_pContext))))
		return E_FAIL;



	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_EnkFire"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Enk/Tiger.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_EnkFireSpiral"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Enk/EnkSpiral.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_EnkFire"),
		CKyojuroEnkEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_EnkSpiral"),
		CKyojuroEnkSpiralEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kien"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Kien.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_Kien"),
		CKyojuroKienEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Nob"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/Nob/Nob.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_Nob"),
		CKyojuroNobEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	//KyojuroEnk
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_KyojuroEnk"),
		CKyojuroEnk::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	// Mig
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_MigCrossRing"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Mig/MigCrossRing.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_MigRing"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Mig/MigRing.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_MigCross"),
		CTanMigRingEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_Mig"),
		CTanMigEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TanjiroMig"),
		CTanjiroMig::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Tak"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Tak/Tak.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TanjiroTak"),
		CTanTakEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_Dash"),
		CDashSmokeEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	PreTransformMatrix = XMMatrixRotationX(XMConvertToRadians(70.f)) * XMMatrixRotationZ(XMConvertToRadians(30.f)) * XMMatrixTranslation(0.5f, 2.7f, 0.f);
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_KoujuroWeapon"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Weapon/KoujuroWeapon.bin", PreTransformMatrix))))
		return E_FAIL;
	PreTransformMatrix = XMMatrixRotationX(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_TanjiroWeapon"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Weapon/TanjiroWeapon.bin", PreTransformMatrix))))
		return E_FAIL;



	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fist"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Akaza/Fist/Fist.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_AkaFist"),
		CAkaFistEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_KuuBullet"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Akaza/Kuu/Kuu.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_KuuBullet"),
		CAkaKuBulletEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Vortex"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Nej/Vortex/Vortex.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_NejWindL"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Nej/TwistWindL.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_NejWindR"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Nej/TwistWindR.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_Vortex"),
		CTanNejVortexEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_NejWind"),
		CTanNejWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TrainLight"),
		CTrainPointLight::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_SwingWindR"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Enmu/Swing/SwingWindR.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_SwingWindL"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Enmu/Swing/SwingWindL.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_SwingWind"),
		CEnmuSwingWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_PunchWind"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Enmu/PunchWind/PunchWind.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_PucnhWind"),
		CEnmuPunchWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnvWind"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Common/EnvWind.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_EnvWind"),
		CEnvWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_RingWind"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Common/RingWind.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Effect_RingWind"),
		CRingWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	m_isFinished = true;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));
	return S_OK;
}

HRESULT CLoader::Loading_For_EnmuBoss()
{

	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));
	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));
	CJsonLoader jsonLoader;

	jsonLoader.Load_Models("../Asset/Json/Models.json", [&]() {
		// 이곳에 로드 후 처리할 작업을 추가합니다.
		});
	jsonLoader.Free();


	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fiona"),
	//	CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Kyoujuro/Kyoujuro.fbx"))))
	//	return E_FAIL;

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuTentacle"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/Tentacle/EnmuTentacle.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Sky"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Map/Sky.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuLeftArm"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/LeftArm/EnmuLeftArm.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuRightArm"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/RightArm/EnmuRightArm.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Body"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/Body/EnmuBossBody.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuHead"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/EnmuBoss/Head/EnmuBossHead.bin", PreTransformMatrix))))
		return E_FAIL;

	/*"Prototype_Component_Model_Tanjiro"*/
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Tanjiro"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::ANIM, "../Asset/Resources/Models/Tanjiro/Tanjiro.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_DefaultSlash"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Kyo/DefaultSlash/DefaultSlash.bin", PreTransformMatrix))))
		return E_FAIL;
	PreTransformMatrix =  XMMatrixRotationX(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_TanjiroWeapon"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Weapon/TanjiroWeapon.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_MigCrossRing"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Mig/MigCrossRing.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_MigRing"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Mig/MigRing.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Tak"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Tak/Tak.bin", PreTransformMatrix))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_Slash"),
		CSlashEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Vortex"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Nej/Vortex/Vortex.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_NejWindL"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Nej/TwistWindL.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_NejWindR"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Tan/Nej/TwistWindR.bin", PreTransformMatrix))))
		return E_FAIL;








	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));

	///* For.Prototype_GameObject_Camera_Free */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_CutSceneCam"),
		CCutSceneCamera::Create(m_pDevice, m_pContext))))
		return E_FAIL;

		/* For.Prototype_GameObject_ThirdPersonCamera */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_ThirdPersonCamera"),
		CThirdPersonCamera::Create(m_pDevice, m_pContext))))
		return E_FAIL;



	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Envirnoment"),
		CEnvironment::Create(m_pDevice, m_pContext))))
		return E_FAIL;



	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Tanjiro"),
		CTanjiro::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	///* For.Prototype_GameObject_Kyojuro */
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_Kyojuro"),
	//	CKyojuro::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	/* For.Prototype_GameObject_KoujuroWeapon */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TanjiroWeapon"),
		CWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	///* For.Prototype_GameObject_ThirdPersonCamera */
	//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_ThirdPersonCamera"),
	//	CThirdPersonCamera::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;


	/* For.Prototype_GameObject_EnmuMeat */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_EnmuMeat"),
		CEnmuMeat::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For.Prototype_GameObject_EnmuTentacle */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_EnmuTentacle"),
		CEnmuTentacle::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Navigation"),
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../Asset/NavMesh/NavMeshEnmuBoss2.dat")))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_WarningZone"),
		CWarningZoneDecal::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_GameObject_TrainLight"),
		CTrainPointLight::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_SwingWindR"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Enmu/Swing/SwingWindR.bin", PreTransformMatrix))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_SwingWindL"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Enmu/Swing/SwingWindL.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_PunchWind"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Enmu/PunchWind/PunchWind.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnvWind"),
		CModel::CreateByBinary(m_pDevice, m_pContext, MODEL::NONANIM, "../Asset/Resources/Models/Effect/Common/EnvWind.bin", PreTransformMatrix))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Effect_EnvWind"),
		CEnvWindEffect::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Battle()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));



	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));

	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Mode()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("원형객체을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLoader* pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : CLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLoader::Free()
{
	__super::Free();

	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);

	DeleteCriticalSection(&m_CriticalSection);
}
