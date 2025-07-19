#include "RingWindEffect.h"
#include "GameInstance.h"
#include <JsonLoader.h>
#include "ParticleSystem.h"
#include "HitParticle.h"

CRingWindEffect::CRingWindEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CRingWindEffect::CRingWindEffect(const CRingWindEffect& Prototype)
	: CMeshEffect(Prototype)
	, m_vColor(Prototype.m_vColor)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
	, m_bLeft(Prototype.m_bLeft)

{
}

HRESULT CRingWindEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 18;
	m_fDuration = 0.3f;
	// 그레이로
	m_vColor = _float4(0.5f, 0.5f, 0.5f, 1.f); // 초기 색상 설정

	return S_OK;
}

HRESULT CRingWindEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(40.f,40.f,40.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);

	return S_OK;
}

void CRingWindEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CRingWindEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_fElapsed += fTimeDelta;
	m_vUVOffset.y += fTimeDelta * 2.f; // UV 애니메이션 속도 조절
	if (m_vUVOffset.y >= 1.f)
	{
		m_vUVOffset.y = 0.f; // UV 오프셋 초기화
	}
	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f; // 시간 초기화
		SetActive(false); // 이펙트 비활성화
	}
	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	m_CombinedWorldMatrix = matWorld;
}

void CRingWindEffect::Late_Update(_float fTimeDelta)
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

HRESULT CRingWindEffect::Render()
{
	Bind_Textures();
	if (FAILED(__super::Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CRingWindEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_RingWind"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}

	if (FAILED(Ready_Textures()))
		return E_FAIL;
	return S_OK;
}

HRESULT CRingWindEffect::Bind_Shader()
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
	_float fDistFade = 500.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_DistFade", &fDistFade, sizeof(_float))))
		return E_FAIL;
	return S_OK;
}

void CRingWindEffect::OnDisable()
{
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_fElapsed = 0.f; 
}

void CRingWindEffect::OnEnable()
{
	m_vUVOffset = _float2(0.f, 0.1f); // UV 오프셋 초기화
	m_fElapsed = 0.f; 
}

HRESULT CRingWindEffect::Bind_Textures()
{
	if (FAILED(!m_Textures[TEX_FRESNEAL]||
			m_Textures[TEX_FRESNEAL]->Bind_ShaderResource(m_pShaderCom, "g_FresnelTexture", 0)))
			return E_FAIL;
	return S_OK;
}

HRESULT CRingWindEffect::Ready_Textures()
{
	// 첫번째 메시 디퓨즈에 T_e_Cmn_GrdLine002 컬러로 사용
	m_Textures[TEX_FRESNEAL] = m_pGameInstance->LoadTexture(L"RingWindFresneal", L"../Asset/Resources/Models/Effect/Tan/Nej/T_e_Skl_Wa_7Siz_Falloff004.dds", true);

	return S_OK;
}

CRingWindEffect* CRingWindEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRingWindEffect* pInstance = new CRingWindEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CRingWindEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CRingWindEffect::Clone(void* pArg)
{
	CRingWindEffect* pClone = new CRingWindEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CRingWindEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CRingWindEffect::Free()
{
	__super::Free();

}
