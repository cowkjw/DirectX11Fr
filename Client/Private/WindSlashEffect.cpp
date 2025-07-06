#include "WindSlashEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CWindSlashEffect::CWindSlashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CWindSlashEffect::CWindSlashEffect(const CWindSlashEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor)
{

}

HRESULT CWindSlashEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

HRESULT CWindSlashEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iShaderPass = 3; // 일단 2번으로 테스트
	m_pTransformCom->Scaling(_float3(1.f, 1.f, 1.f));
	// 테스트용 컬러
	m_vColor = _float4(1.f,1.f,1.f,1.f); 
	m_bUseOffset = true;


	return S_OK;
}

void CWindSlashEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CWindSlashEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_bUseOffset)
	{
		m_vUVOffset.x += fTimeDelta*2.f; // UV 애니메이션 속도 조절
		//m_fUVOffset.y += fTimeDelta * 0.5f; // UV 애니메이션 속도 조절
		if (m_vUVOffset.x >= 1.f)
		{
		/*	m_bRenderMesh = false;
			m_bUseOffset = false;*/
			//SetActive(false);
			m_vUVOffset.x = 0.f;
		}
		if (m_vUVOffset.y > 1.f)
			m_vUVOffset.y = 0.f;
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Update(fTimeDelta);
		}
	}
}

void CWindSlashEffect::Late_Update(_float fTimeDelta)
{
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();

		// 본 매트릭스를 그대로 사용 (정규화하지 않음)
		_matrix matBoneLocal = XMLoadFloat4x4(&boneLocal);
		_matrix matParentWorld = XMLoadFloat4x4(&parentWorld);

		// 올바른 매트릭스 곱셈 순서: ParentWorld * BoneLocal
		_matrix world = XMMatrixMultiply(matBoneLocal, matParentWorld);

		//// 파티클 이펙트의 로컬 오프셋이 있다면 적용
		//_matrix localOffset = XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix());
		//world = XMMatrixMultiply(localOffset, world);

		XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
		//	m_pTransformCom->Set_WorldMatrix(m_CombinedWorldMatrix);
	}
	else
	{
		// 본 소켓이 없으면 그냥 월드에 생성하도록
		m_CombinedWorldMatrix = m_pTransformCom->Get_WorldMatrix();
	}
	if (m_bRenderMesh)
	{
		__super::Late_Update(fTimeDelta);
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CWindSlashEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CWindSlashEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_WindSlash"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}

	
	return S_OK;
}

HRESULT CWindSlashEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	return S_OK;
}

void CWindSlashEffect::OnDisable()
{
	//if (m_pHitParticle && m_pHitParticle->IsActive())
	//{
	//	m_pHitParticle->SetActive(false); // 히트 파티클 비활성화
	//}
	m_vUVOffset = _float2(0.5f, 0.f); // UV 오프셋 초기화
}

void CWindSlashEffect::OnEnable()
{

}

CWindSlashEffect* CWindSlashEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWindSlashEffect* pInstance = new CWindSlashEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWindSlashEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWindSlashEffect::Clone(void* pArg)
{
	CWindSlashEffect* pClone = new CWindSlashEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CWindSlashEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CWindSlashEffect::Free()
{
	__super::Free();
}
