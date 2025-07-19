#include "EnmuPunchWindEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CEnmuPunchWindEffect::CEnmuPunchWindEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CEnmuPunchWindEffect::CEnmuPunchWindEffect(const CEnmuPunchWindEffect& Prototype)
	: CMeshEffect(Prototype)
	, m_vColor(Prototype.m_vColor)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_bLeft(Prototype.m_bLeft)

{
}

HRESULT CEnmuPunchWindEffect::Initialize_Prototype()
{
	
	m_vUVOffset = _float2(0.f, 0.0f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 19;
	m_fDuration = 0.4f;
	// 그레이로
	m_vColor = _float4(0.5f, 0.5f, 0.5f, 1.f); // 초기 색상 설정

	return S_OK;
}

HRESULT CEnmuPunchWindEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_pTransformCom->Scaling(_float3(110.f, 110.f, 110.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CEnmuPunchWindEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CEnmuPunchWindEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;


	if(m_bLeft)
		m_vUVOffset.y += fTimeDelta * 1.5f; // UV 애니메이션 속도 조절
	else
		m_vUVOffset.y -= fTimeDelta * 1.5f; // UV 애니메이션 속도 조절

	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f;
		SetActive(false);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Update(fTimeDelta);
		}
	}
}

void CEnmuPunchWindEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
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

HRESULT CEnmuPunchWindEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CEnmuPunchWindEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_PunchWind"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CEnmuPunchWindEffect::Bind_Shader()
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

void CEnmuPunchWindEffect::OnDisable()
{
	m_vUVOffset = _float2(0.f, 0.0f); // UV 오프셋 초기화
	m_fElapsed = 0.f; 
}

void CEnmuPunchWindEffect::OnEnable()
{
	m_fElapsed = 0.f; 
}


void CEnmuPunchWindEffect::UpdateTransform()
{
	if (m_pBoneSocket)
	{
		_matrix matParent = XMLoadFloat4x4(&m_pParent->GetTransform()->Get_WorldMatrix());
		_matrix matBone = XMLoadFloat4x4(m_pBoneSocket->Get_CombinedTransformationMatrix());

		_matrix boneWorld = XMMatrixMultiply(matBone, matParent);
		_vector scale, rotQuat, trans;
		XMMatrixDecompose(&scale, &rotQuat, &trans, boneWorld);
		_float3 myScale = m_pTransformCom->Get_Scaled();
		_vector myScaleVec = XMLoadFloat3(&myScale);
		_matrix world = XMMatrixScalingFromVector(myScaleVec) * XMMatrixTranslationFromVector(trans);

		XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
	}
}


CEnmuPunchWindEffect* CEnmuPunchWindEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnmuPunchWindEffect* pInstance = new CEnmuPunchWindEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnmuPunchWindEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEnmuPunchWindEffect::Clone(void* pArg)
{
	CEnmuPunchWindEffect* pClone = new CEnmuPunchWindEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CEnmuPunchWindEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CEnmuPunchWindEffect::Free()
{
	__super::Free();

}
