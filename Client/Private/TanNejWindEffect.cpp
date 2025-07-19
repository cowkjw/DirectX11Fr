#include "TanNejWindEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CTanNejWindEffect::CTanNejWindEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CTanNejWindEffect::CTanNejWindEffect(const CTanNejWindEffect& Prototype)
	: CMeshEffect(Prototype)
	, m_vColor(Prototype.m_vColor)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_bLeft(Prototype.m_bLeft)

{
}

HRESULT CTanNejWindEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_vUVScale = _float2(2.f, 0.75f); // UV 스케일 설정
	m_iShaderPass = 18;
	m_fDuration = 1.5f;
	// 그레이로
	m_vColor = _float4(0.5f, 0.5f, 0.5f, 1.f); // 초기 색상 설정

	return S_OK;
}

HRESULT CTanNejWindEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(30.f,30.f,30.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CTanNejWindEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CTanNejWindEffect::Update(_float fTimeDelta)
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
		if (m_bLeft)
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), -m_fRotationSpeed * fTimeDelta);
		}
		else
		{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fRotationSpeed * fTimeDelta);
		}
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

void CTanNejWindEffect::Late_Update(_float fTimeDelta)
{
	if (m_bRenderMesh)
	{
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

HRESULT CTanNejWindEffect::Render()
{
	Bind_Textures();
	if (FAILED(__super::Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CTanNejWindEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (m_bLeft)
	{

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_NejWindL"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	}
	else
	{
		/* For.Com_Model */
		if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_NejWindR"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		{
			return E_FAIL;
		}
	}

	if (FAILED(Ready_Textures()))
		return E_FAIL;
	return S_OK;
}

HRESULT CTanNejWindEffect::Bind_Shader()
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
	_float fDistFade = 250.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_DistFade", &fDistFade, sizeof(_float))))
		return E_FAIL;
	return S_OK;
}

void CTanNejWindEffect::OnDisable()
{
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_vUVScale = _float2(2.f, 0.75f); // UV 스케일 설정

	m_pTransformCom->Scaling(_float3(40.f, 15.f, 40.f));
	m_fElapsed = 0.f; 
}

void CTanNejWindEffect::OnEnable()
{
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_vUVScale = _float2(2.f, 0.75f); // UV 스케일 설정
	m_fElapsed = 0.f; 
}

HRESULT CTanNejWindEffect::Bind_Textures()
{
	if (FAILED(!m_Textures[TEX_FRESNEAL]||
			m_Textures[TEX_FRESNEAL]->Bind_ShaderResource(m_pShaderCom, "g_FresnelTexture", 0)))
			return E_FAIL;
	return S_OK;
}

HRESULT CTanNejWindEffect::Ready_Textures()
{
	// 첫번째 메시 디퓨즈에 T_e_Cmn_GrdLine002 컬러로 사용
	m_Textures[TEX_FRESNEAL] = m_pGameInstance->LoadTexture(L"WindFresneal", L"../Asset/Resources/Models/Effect/Tan/Nej/T_e_Skl_Wa_7Siz_Falloff004.dds", true);

	return S_OK;
}

void CTanNejWindEffect::RiseVortex()
{
	_float t = m_fElapsed / m_fDuration;
	if (t > 1.f) t = 1.f;
	const _float riseEnd = 0.3f; 
	_float startY = 15.f;
	_float peakY = 65.f;        // 최고 높이
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
	_float endXZ = 120.f;          // 퍼질 때 최종
	_float newXZ;
	_float p2 = (t - riseEnd) / (1.f - riseEnd);
	newXZ = startXZ + (endXZ - startXZ) * p2;
	m_pTransformCom->Scaling(_float3(newXZ, newY, newXZ));

}

CTanNejWindEffect* CTanNejWindEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanNejWindEffect* pInstance = new CTanNejWindEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanNejWindEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanNejWindEffect::Clone(void* pArg)
{
	CTanNejWindEffect* pClone = new CTanNejWindEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTanNejWindEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CTanNejWindEffect::Free()
{
	__super::Free();

}
