#include "AkaFistEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "EffectManager.h"
#include "ParticleSystem.h"
#include "HitParticle.h"

CAkaFistEffect::CAkaFistEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CAkaFistEffect::CAkaFistEffect(const CAkaFistEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_vColors(Prototype.m_vColors)
{
}

HRESULT CAkaFistEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_bUseOffset = true;
	m_bRenderMesh = true;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 12;
	m_fDuration = 0.2f;
	InitColor();
	return S_OK;
}

HRESULT CAkaFistEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(11.f,11.f,11.f));


	return S_OK;
}

void CAkaFistEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CAkaFistEffect::Update(_float fTimeDelta)
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
		m_vUVOffset.x += fTimeDelta * 0.5f;
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

void CAkaFistEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
		//__super::Late_Update(fTimeDelta);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLUR_EFFECT, this);
	}

	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CAkaFistEffect::Render()
{
	// 4번에 SmokeMask넣어둠
	// 6번에 distortion 넣어둠
	// 8번에 알파보단 아마 이미시브같음
	// 1번에 디퓨즈
	//m_pModelCom->Bind_Material(m_pShaderCom, "g_EmissiveTexture", 0, aiTextureType_EMISSIVE, 0);
	//if (FAILED(__super::Render()))
	//	return E_FAIL;
	Bind_Shader();

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_SpecularTexture", i, aiTextureType_OPACITY, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColors[i], sizeof(_float4))))
			return E_FAIL;
		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAkaFistEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Fist"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CAkaFistEffect::Bind_Shader()
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

void CAkaFistEffect::OnDisable()
{
//	m_vUVScale = _float2(3.f, 1.f); // UV 스케일 설정
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
}

void CAkaFistEffect::OnEnable()
{
//	m_vUVScale = _float2(3.f, 1.f); // UV 스케일 설정
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_fElapsed = 0.f; // 시간 초기화
	//_float3 vPos{};
	//XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//CEffectManager::Get_Instance()->SpawnParticleEffect(L"Fire", vPos,_float3(1.f,2.f,1.f));
	//CEffectManager::Get_Instance()->SpawnParticleEffect(L"FireSpread", vPos);
}

void CAkaFistEffect::InitColor()
{
	m_vColors[0] = _float4(0.f, 0.3f, 0.8f, 1.f); // Blue
	m_vColors[1] = _float4(0.0f, 0.6f, 0.8f, 1.f); // Light Blue
	m_vColors[2] = _float4(0.3f, 0.6f, 0.8f, 1.f); // Light Purple
	m_vColors[3] = _float4(0.1f, 0.3f, 0.8f, 1.f); // Purple
}

CAkaFistEffect* CAkaFistEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAkaFistEffect* pInstance = new CAkaFistEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAkaFistEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAkaFistEffect::Clone(void* pArg)
{
	CAkaFistEffect* pClone = new CAkaFistEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CAkaFistEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CAkaFistEffect::Free()
{
	__super::Free();
}
