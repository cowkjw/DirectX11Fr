#include "EnmuSwingWindEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CEnmuSwingWindEffect::CEnmuSwingWindEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CEnmuSwingWindEffect::CEnmuSwingWindEffect(const CEnmuSwingWindEffect& Prototype)
	: CMeshEffect(Prototype)
	, m_vColor(Prototype.m_vColor)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_bLeft(Prototype.m_bLeft)

{
}

HRESULT CEnmuSwingWindEffect::Initialize_Prototype()
{
	
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 19;
	m_fDuration = 0.7f;
	// 그레이로
	m_vColor = _float4(0.5f, 0.5f, 0.5f, 1.f); // 초기 색상 설정

	return S_OK;
}

HRESULT CEnmuSwingWindEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_pTransformCom->Scaling(_float3(220.f,220.f,220.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);
	//m_pTransformCom->Rotate_EulerAngles(_float3(90.f, 0.f, 0.f));
	// 23  23  -117
	if (!m_bLeft)
	{
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(23.f, 15.f, -117.f,1.f));
	m_pTransformCom->Rotate_EulerAngles(_float3(90.f, -65.f, 0.f));
	}
	else
	{
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(23.f, 15.f, -117.f, 1.f));
		m_pTransformCom->Rotate_EulerAngles(_float3(90.f, 65.f, 0.f));
	}
	return S_OK;
}

void CEnmuSwingWindEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CEnmuSwingWindEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;


	if(m_bLeft)
		m_vUVOffset.y += fTimeDelta * 0.5f; // UV 애니메이션 속도 조절
	else
		m_vUVOffset.y -= fTimeDelta * 0.5f; // UV 애니메이션 속도 조절

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
	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	m_CombinedWorldMatrix = matWorld;
}

void CEnmuSwingWindEffect::Late_Update(_float fTimeDelta)
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

HRESULT CEnmuSwingWindEffect::Render()
{
	Bind_Textures();
	if (FAILED(__super::Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CEnmuSwingWindEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (!m_bLeft)
	{

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_SwingWindL"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	}
	else
	{
		/* For.Com_Model */
		if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_SwingWindR"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		{
			return E_FAIL;
		}
	}

	if (FAILED(Ready_Textures()))
		return E_FAIL;
	return S_OK;
}

HRESULT CEnmuSwingWindEffect::Bind_Shader()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof _float4)))
		return E_FAIL;
		
	return S_OK;
}

void CEnmuSwingWindEffect::OnDisable()
{
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_fElapsed = 0.f; 
}

void CEnmuSwingWindEffect::OnEnable()
{
	m_fElapsed = 0.f; 
}

HRESULT CEnmuSwingWindEffect::Bind_Textures()
{
	if (FAILED(!m_Textures[TEX_DISTORT]||
			m_Textures[TEX_DISTORT]->Bind_ShaderResource(m_pShaderCom, "g_DistortionTexture", 0)))
			return E_FAIL;
	return S_OK;
}

HRESULT CEnmuSwingWindEffect::Ready_Textures()
{
	// 디퓨즈에 
	m_Textures[TEX_DISTORT] = m_pGameInstance->GetTexture(L"TrainPointLightDist",true);

	return S_OK;
}


CEnmuSwingWindEffect* CEnmuSwingWindEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnmuSwingWindEffect* pInstance = new CEnmuSwingWindEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnmuSwingWindEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEnmuSwingWindEffect::Clone(void* pArg)
{
	CEnmuSwingWindEffect* pClone = new CEnmuSwingWindEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CEnmuSwingWindEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CEnmuSwingWindEffect::Free()
{
	__super::Free();

}
