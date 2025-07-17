#include "GuardEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "EffectManager.h"
#include "ParticleSystem.h"
#include "HitParticle.h"

CGuardEffect::CGuardEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CGuardEffect::CGuardEffect(const CGuardEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_vColors(Prototype.m_vColors)
{
}

HRESULT CGuardEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bUseOffset = true;
	m_bRenderMesh = true;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 13;
	m_fDuration = 0.2f;
	m_vColor = _float4(0.1f, 0.4f, 1.f, 1.f);
	return S_OK;
}

HRESULT CGuardEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(15.5f,15.5f,8.f));
	m_pTransformCom->Rotate_EulerAngles(_float3(0.f, 90.f, 0.f)); // Y축으로 90도 회전


	return S_OK;
}

void CGuardEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CGuardEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;
	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f;
		SetActive(false);
	}
	else
	{
		m_vUVOffset.x += fTimeDelta * 2.5f;
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

void CGuardEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
		//__super::Late_Update(fTimeDelta);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
	}

	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CGuardEffect::Render()
{
	Bind_Shader();

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_SpecularTexture", i, aiTextureType_OPACITY, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
			return E_FAIL;
		_int iMeshId = i;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iMeshId", &iMeshId, sizeof(_int))))
			return E_FAIL;
		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CGuardEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Model_Gurad"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CGuardEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVScale", &m_vUVScale, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fElapsed, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fDuration", &m_fDuration, sizeof(_float))))
		return E_FAIL;
	return S_OK;
}

void CGuardEffect::OnDisable()
{

	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
}

void CGuardEffect::OnEnable()
{
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
}

void CGuardEffect::InitColor()
{
	m_vColors[0] = _float4(0.f, 0.3f, 0.8f, 1.f); 
	m_vColors[1] = _float4(0.0f, 0.6f, 0.8f, 1.f);
	m_vColors[2] = _float4(0.3f, 0.6f, 0.8f, 1.f);
	m_vColors[3] = _float4(0.1f, 0.3f, 0.8f, 1.f);
}

CGuardEffect* CGuardEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGuardEffect* pInstance = new CGuardEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGuardEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGuardEffect::Clone(void* pArg)
{
	CGuardEffect* pClone = new CGuardEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CGuardEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CGuardEffect::Free()
{
	__super::Free();
}
