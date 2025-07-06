#include "SlashEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CSlashEffect::CSlashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CSlashEffect::CSlashEffect(const CSlashEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor)
{

}

HRESULT CSlashEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

HRESULT CSlashEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iShaderPass = 3; // 일단 2번으로 테스트
	m_pTransformCom->Scaling(_float3(1.f, 1.f, 1.f));
	// 테스트용 컬러
	m_vColor = _float4(0.705f, 0.317f,0.168f, 1.f); // 주황 느낌
	m_bUseOffset = true;
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일
	m_fDuration = 3.f;

	return S_OK;
}

void CSlashEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CSlashEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_bUseOffset)
	{
		m_vUVOffset.x += fTimeDelta*2.f; // UV 애니메이션 속도 조절
		//m_fUVOffset.y += fTimeDelta * 0.5f; // UV 애니메이션 속도 조절
		if (m_vUVOffset.x >= 1.f)
		{
			m_bRenderMesh = false;
			m_bUseOffset = false;
			SetActive(false);
		}
	}
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Update(fTimeDelta);
		}
	}
}

void CSlashEffect::Late_Update(_float fTimeDelta)
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

HRESULT CSlashEffect::Render()
{	// 4번에 Emissive 라인텍스쳐 빛나는 영역인듯
	// 6번에 Normal distortion 넣어둠
	// 8번에 Specular 노이즈 
	// 1번에 Diffuse 마스크
	m_pModelCom->Bind_Material(m_pShaderCom, "g_EmissiveTexture", 0, aiTextureType_EMISSIVE, 0);
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlashEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_DefaultSlash"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_DefaultSlash"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;
	}

	
	return S_OK;
}

HRESULT CSlashEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVScale", &m_vUVScale, sizeof(_float2))))
		return E_FAIL;
	return S_OK;
}

void CSlashEffect::OnDisable()
{
	//if (m_pHitParticle && m_pHitParticle->IsActive())
	//{
	//	m_pHitParticle->SetActive(false); // 히트 파티클 비활성화
	//}
	m_vUVOffset = _float2(0.5f, 0.f); // UV 오프셋 초기화
}

void CSlashEffect::OnEnable()
{

}

CSlashEffect* CSlashEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlashEffect* pInstance = new CSlashEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CSlashEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CSlashEffect::Clone(void* pArg)
{
	CSlashEffect* pClone = new CSlashEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CSlashEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CSlashEffect::Free()
{
	__super::Free();
}
