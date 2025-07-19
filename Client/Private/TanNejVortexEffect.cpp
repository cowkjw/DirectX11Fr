#include "TanNejVortexEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CTanNejVortexEffect::CTanNejVortexEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CTanNejVortexEffect::CTanNejVortexEffect(const CTanNejVortexEffect& Prototype)
	: CMeshEffect(Prototype)
	, m_vColor(Prototype.m_vColor)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_Vortex3Textures(Prototype.m_Vortex3Textures)
	, m_Vortex4Textures(Prototype.m_Vortex4Textures)
	, m_iShaerderPasses(Prototype.m_iShaerderPasses)
{
	for (auto& texture : m_Vortex3Textures)
	{
		Safe_AddRef(texture);
	}
	for (auto& texture : m_Vortex4Textures)
	{
		Safe_AddRef(texture);
	}
}

HRESULT CTanNejVortexEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 8;
	m_fDuration = 1.5f;
	// 그레이로
	m_vColor = _float4(0.5f, 0.5f, 0.5f, 1.f); // 초기 색상 설정

	return S_OK;
}

HRESULT CTanNejVortexEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(30.f,30.f,30.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CTanNejVortexEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CTanNejVortexEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;

	RiseVortex();
	


	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f;
		SetActive(false);
	}
	else
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fRotationSpeed * fTimeDelta);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Update(fTimeDelta);
		}
	}
	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	m_CombinedWorldMatrix = matWorld;
}

void CTanNejVortexEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
		//m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CTanNejVortexEffect::Render()
{
	Bind_Shader();

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_SpecularTexture", i, aiTextureType_OPACITY, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		Bind_Textures(i);
		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(m_iShaerderPasses[i])))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CTanNejVortexEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Vortex"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}

	if (FAILED(Ready_Textures()))
		return E_FAIL;
	return S_OK;
}

HRESULT CTanNejVortexEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVScale", &m_vUVScale, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fElapsed, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fDuration", &m_fDuration, sizeof(_float))))
		return E_FAIL;
	return S_OK;
}

void CTanNejVortexEffect::OnDisable()
{
	m_vUVScale = _float2(1.f, 1.f); 
	m_vUVOffset = _float2(0.f, 0.f); 

	m_pTransformCom->Scaling(_float3(30.f, 15.f, 30.f));
	m_fElapsed = 0.f; 
}

void CTanNejVortexEffect::OnEnable()
{
	m_vUVScale = _float2(1.f,1.f); 
	m_vUVOffset = _float2(0.f, 0.f); 
	m_fElapsed = 0.f; 
}

HRESULT CTanNejVortexEffect::Bind_Textures(_uint iMeshIndex)
{
	if (iMeshIndex == 0)
	{
		if (FAILED(!m_Textures[TEX_ALPHA]||
			m_Textures[TEX_ALPHA]->Bind_ShaderResource(m_pShaderCom, "g_AlphaTexture", 0)))
			return E_FAIL;
		if (FAILED(!m_Textures[TEX_ALPHA2]||
			m_Textures[TEX_ALPHA2]->Bind_ShaderResource(m_pShaderCom, "g_Alpha2Texture", 0)))
			return E_FAIL;
		if (FAILED(!m_Textures[TEX_MASK]||
			m_Textures[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_LineTexture", 0)))
			return E_FAIL;
		if (FAILED(!m_Textures[TEX_WPOSITION]||
			m_Textures[TEX_WPOSITION]->Bind_ShaderResource(m_pShaderCom, "g_WPOTexture", 0)))
			return E_FAIL;
	}
	else if (iMeshIndex == 1)
	{
		if (FAILED(!m_Vortex3Textures[TEX_ALPHA2]||
			m_Vortex3Textures[TEX_ALPHA2]->Bind_ShaderResource(m_pShaderCom, "g_Alpha2Texture", 0)))
			return E_FAIL;
		if (FAILED(!m_Vortex3Textures[TEX_DISTORT]||
			m_Vortex3Textures[TEX_DISTORT]->Bind_ShaderResource(m_pShaderCom, "g_DistortionTexture", 0)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(!m_Vortex4Textures[TEX_ALPHA]||
			m_Vortex4Textures[TEX_ALPHA]->Bind_ShaderResource(m_pShaderCom, "g_AlphaTexture", 0)))
			return E_FAIL;
		if (FAILED(!m_Vortex4Textures[TEX_ALPHA2]||
			m_Vortex4Textures[TEX_ALPHA2]->Bind_ShaderResource(m_pShaderCom, "g_Alpha2Texture", 0)))
			return E_FAIL;
		if (FAILED(!m_Vortex4Textures[TEX_MASK]||
			m_Vortex4Textures[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_LineTexture", 0)))
			return E_FAIL;
		if (FAILED(!m_Vortex4Textures[TEX_WPOSITION]||
			m_Vortex4Textures[TEX_WPOSITION]->Bind_ShaderResource(m_pShaderCom, "g_WPOTexture", 0)))
			return E_FAIL;
		if (FAILED(!m_Vortex4Textures[TEX_DISTORT]||
			m_Vortex4Textures[TEX_DISTORT]->Bind_ShaderResource(m_pShaderCom, "g_DistortionTexture", 0)))
			return E_FAIL;
		if (FAILED(!m_Vortex4Textures[TEX_FRESNEAL]||
			m_Vortex4Textures[TEX_FRESNEAL]->Bind_ShaderResource(m_pShaderCom, "g_FresnelTexture", 0)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CTanNejVortexEffect::Ready_Textures()
{
	// 첫번째 메시 디퓨즈에 T_e_Cmn_GrdLine002 컬러로 사용
	m_Textures[TEX_ALPHA] = m_pGameInstance->LoadTexture(L"Vortex1Alpha", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/1/T_e_Cmn_Noise003.dds", true);
	m_Textures[TEX_ALPHA2] = m_pGameInstance->LoadTexture(L"Vortex1Alpha2", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/1/T_e_Skl_Wa_Scmn_Trail002.dds", true);
	m_Textures[TEX_MASK] = m_pGameInstance->LoadTexture(L"Vortex1Line", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/1/T_e_cmn_Line006.dds", true);
	m_Textures[TEX_WPOSITION] = m_pGameInstance->LoadTexture(L"Vortex1WPO", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/1/T_e_cmn_WaveNormal002.dds", true);

	// 두 번째 메시 텍스처 (디퓨즈에는 T_e_Skl_Wa_6Nej_Shadow002 넣어둠 첫번째 알파로 사용)
	m_Vortex3Textures[TEX_ALPHA2] = m_pGameInstance->LoadTexture(L"Vortex3Alpha2", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/3/T_e_cmn_Grd_Radial001.dds", true);
	m_Vortex3Textures[TEX_DISTORT] = m_pGameInstance->LoadTexture(L"Vortex3Dist", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/3/T_e_cmn_Energy001.dds", true);

	// 세 번째 메시 텍스처 
	// 디퓨즈에 Skl_Wa_6Nej_Vortex001C 컬러로 사용하기
	m_Vortex4Textures[TEX_ALPHA] = m_pGameInstance->LoadTexture(L"Vortex4Alpha", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/4/T_e_Skl_Wa_6Nej_Vortex001.dds", true);
	m_Vortex4Textures[TEX_ALPHA2] = m_pGameInstance->GetTexture(L"Vortex1Alpha2",true); // 1번이랑 같은 텍스처 사용
	m_Vortex4Textures[TEX_MASK] = m_pGameInstance->GetTexture(L"Vortex1Line",true); // 1번이랑 같은 텍스처 사용
	m_Vortex4Textures[TEX_WPOSITION] = m_pGameInstance->GetTexture(L"Vortex1WPO",true); // 1번이랑 같은 텍스처 사용
	m_Vortex4Textures[TEX_DISTORT] = m_pGameInstance->LoadTexture(L"Vortex4Dist", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/4/T_e_Cmn_Wave007.dds", true);
	m_Vortex4Textures[TEX_FRESNEAL] = m_pGameInstance->LoadTexture(L"Vortex4Fresneal", L"../Asset/Resources/Models/Effect/Tan/Nej/Vortex/4/T_e_cmn_Falloff003.dds", true);
	return S_OK;
}

void CTanNejVortexEffect::RiseVortex()
{
	_float t = m_fElapsed / m_fDuration;
	if (t > 1.f) t = 1.f;
	const _float riseEnd = 0.3f;    // 30 시점까지 솟구친다
	_float startY = 15.f;
	_float peakY = 60.f;
	_float newY;
	if (t < riseEnd)
	{
		_float p = t / riseEnd;
		newY = startY + (peakY - startY) * p;
	}
	else
	{
		// riseEnd 이후
		newY = peakY;
	}
	_float startXZ = 50.f;
	_float endXZ = 80.f;          // 퍼질 때 최종
	_float newXZ;
	_float p2 = (t - riseEnd) / (1.f - riseEnd);
	newXZ = startXZ + (endXZ - startXZ) * p2;
	m_pTransformCom->Scaling(_float3(newXZ, newY, newXZ));

}

CTanNejVortexEffect* CTanNejVortexEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanNejVortexEffect* pInstance = new CTanNejVortexEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanNejVortexEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanNejVortexEffect::Clone(void* pArg)
{
	CTanNejVortexEffect* pClone = new CTanNejVortexEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTanNejVortexEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CTanNejVortexEffect::Free()
{
	__super::Free();
	for (auto& texture : m_Vortex3Textures)
	{
		Safe_Release(texture);
	}
	for (auto& texture : m_Vortex4Textures)
	{
		Safe_Release(texture);
	}
}
