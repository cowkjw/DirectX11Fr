#include "ParticleEffect.h"

#include "GameInstance.h"

CParticleEffect::CParticleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect{ pDevice, pContext }

{

}

CParticleEffect::CParticleEffect(const CParticleEffect& Prototype)
	: CEffect(Prototype)
	, m_ParticleSystems(Prototype.m_ParticleSystems)
{
	for (auto& Pair : m_ParticleSystems)
	{
		Safe_AddRef(Pair.second);
	}

}

HRESULT CParticleEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParticleEffect::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CParticleEffect::Priority_Update(_float fTimeDelta)
{

}

void CParticleEffect::Update(_float fTimeDelta)
{
	//if (m_pBoneSocket)
	//{
	//	_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
	//	_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();
	//	_matrix matBoneLocal = XMLoadFloat4x4(&boneLocal);
	///*	for (size_t i = 0; i < 3; i++)
	//		matBoneLocal.r[i] = XMVector3Normalize(matBoneLocal.r[i]);*/

	//		_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
	//	_matrix world = /*XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix())**/matBoneLocal * XMLoadFloat4x4(&parentWorld);
	//	world = XMMatrixMultiply(matScale, world); 

	//	_float4x4 WorldMatrix{};
	//	XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
	//	m_pTransformCom->Set_WorldMatrix(m_CombinedWorldMatrix);
	//}
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();

		// 본 매트릭스를 그대로 사용 (정규화하지 않음)
		_matrix matBoneLocal = XMLoadFloat4x4(&boneLocal);
		_matrix matParentWorld = XMLoadFloat4x4(&parentWorld);

		// 올바른 매트릭스 곱셈 순서: ParentWorld * BoneLocal
		_matrix world = XMMatrixMultiply(matBoneLocal, matParentWorld);

		// 파티클 이펙트의 로컬 오프셋이 있다면 적용
		 _matrix localOffset = XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix());
		 world = XMMatrixMultiply(localOffset, world);

		XMStoreFloat4x4(&m_CombinedWorldMatrix, world);
	//	m_pTransformCom->Set_WorldMatrix(m_CombinedWorldMatrix);
	}
	else
	{
		// 본 소켓이 없으면 그냥 월드에 생성하도록
		m_CombinedWorldMatrix = GetTransform()->Get_WorldMatrix();
	}
	_int iActiveParticleCount = 0;
	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second && Pair.second->IsPlaying())
		{
			iActiveParticleCount++;
			Pair.second->UpdateVertexInstances(fTimeDelta);
		}
	}
	if (iActiveParticleCount == 0)
	{
		SetActive(false);
	}
}

void CParticleEffect::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CParticleEffect::Render()
{

	//if (m_pTransformCom)
	//{
	//	m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	//}

	/*for (auto& Pair : m_ParticleShaders)
	{
		if (Pair.second == nullptr)
			continue;
		if (m_pTransformCom)
		{
			m_pTransformCom->Bind_ShaderResource(Pair.second, "g_WorldMatrix");
		}


	}*/
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second && Pair.second->IsActive())
		{
			if (FAILED(Pair.second->Bind_Buffers()))
				return E_FAIL;
			if (FAILED(Pair.second->Render()))
				return E_FAIL;
		}
	}

	//if (FAILED(m_ParticleSystems[L"Snow"]->Bind_Buffers()))
	//	return E_FAIL;

	//if (FAILED(m_ParticleSystems[L"Snow"]->Render()))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CParticleEffect::Ready_Components()
{
	///* For.Com_Shader */
	//if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxRectInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	///* For.Com_Particle */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
	//	TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_ParticleSystems[L"Snow"]))))
	//	return E_FAIL;
	///* For.Com_Texture */

	//if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("TitleLogo"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_DIFFUSE]))))
	//	return E_FAIL;
	return S_OK;
}

CParticleEffect* CParticleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParticleEffect* pInstance = new CParticleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticleEffect::AddParticleSystem(const _wstring& particleName, CParticleSystem* pParticleSystem)
{
	if (nullptr == pParticleSystem)
		return;
	auto it = m_ParticleSystems.find(particleName);
	if (it != m_ParticleSystems.end())
	{
		Safe_Release(it->second);
	}
	m_ParticleSystems[particleName] = pParticleSystem;
	Safe_AddRef(pParticleSystem);
}

void CParticleEffect::RemoveParticleSystem(CParticleSystem* pParticleSystem)
{
	if (nullptr == pParticleSystem)
		return;
	for (auto it = m_ParticleSystems.begin(); it != m_ParticleSystems.end();)
	{
		if (it->second == pParticleSystem)
		{
			Safe_Release(it->second);
			it = m_ParticleSystems.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void CParticleEffect::RemoveParticleSystem(const _wstring& particleName)
{
	auto it = m_ParticleSystems.find(particleName);
	if (it != m_ParticleSystems.end())
	{
		Safe_Release(it->second);
		m_ParticleSystems.erase(it);
	}
}

void CParticleEffect::ClearParticleSystems()
{
	for (auto& pair : m_ParticleSystems)
	{
		Safe_Release(pair.second);
	}
	m_ParticleSystems.clear();
}

void CParticleEffect::SpwanParticle()
{
	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second)
		{
			Pair.second->ResetParticle();
		}
	}
}

void CParticleEffect::DespwanParticle()
{
	for (const auto& Pair : m_ParticleSystems)
	{
		if (Pair.second)
		{
			Pair.second->SetActive(false);
		}
	}
}

void CParticleEffect::OnEnable()
{
	SpwanParticle();
}


CGameObject* CParticleEffect::Clone(void* pArg)
{
	CParticleEffect* pInstance = new CParticleEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CParticleEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticleEffect::Free()
{
	__super::Free();
	for (auto& pair : m_ParticleSystems)
	{
		Safe_Release(pair.second);
	}
	m_ParticleSystems.clear();
}
