#include "TanTakEffect.h"
#include "GameInstance.h"
#include "ParticleSystem.h"
#include "WaterParticle.h"
#include "WaterWaveParticle.h"
#include "EffectManager.h"
#include "JsonLoader.h"

CTanTakEffect::CTanTakEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CTanTakEffect::CTanTakEffect(const CTanTakEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor),
	m_fDir(Prototype.m_fDir)
	, m_fRotationSpeed(Prototype.m_fRotationSpeed)
	, m_fDuration(Prototype.m_fDuration)
{
}

HRESULT CTanTakEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_vUVOffset = _float2(0.f, 0.f); // UV 오프셋 초기화
	m_vUVScale = _float2(1.f, 1.f); // UV 스케일 설정
	m_iShaderPass = 10;
	m_fDuration = 1.5f;

	

	return S_OK;
}

HRESULT CTanTakEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(11.f,15.f,11.f));
	m_pTransformCom->Set_SpeedPerSec(30.f);
	CJsonLoader jsonLoader(m_pDevice, m_pContext);
	CParticleSystem* pWaveParticleSystem = nullptr;
	CParticleSystem* pSpreadParticleSystem = nullptr;
	CParticleEffect* pEffect = CWaterWaveParticle::Create(m_pDevice, m_pContext);
	if (pEffect == nullptr)
		return E_FAIL;
	pEffect->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/WaterWave_Particle.json", &pWaveParticleSystem);
	pEffect->AddParticleSystem(L"Wave", pWaveParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("WaterWave"), pEffect);



	CParticleEffect* pEffect2 = CWaterParticle::Create(m_pDevice, m_pContext);
	if (pEffect2 == nullptr)
		return E_FAIL;
	pEffect2->Initialize(nullptr);
	jsonLoader.Load_Particle("../Asset/Json/Particle/Water_Particle.json", &pSpreadParticleSystem);
	(pEffect2)->AddParticleSystem(L"SpreadWater", pSpreadParticleSystem);
	CEffectManager::Get_Instance()->RegisterEffect(TEXT("SpreadWater"), pEffect2);


	jsonLoader.Free();
	return S_OK;
}

void CTanTakEffect::Priority_Update(_float fTimeDelta)
{
	
	for (auto& particle : m_ParticleEffects)
	{
		if (particle.second && particle.second->IsActive())
		{
			particle.second->Priority_Update(fTimeDelta);
		}
	}
}

void CTanTakEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	_float3 vPos{};
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
	m_fElapsed += fTimeDelta;
	m_vUVOffset.x -= fTimeDelta * 0.5f;
	m_vUVOffset.y -= fTimeDelta * 1.f;
	//if (m_vUVOffset.y < 0.f)
	//{
	//	m_vUVOffset.y = 1.f;
	//}
	//
	if (m_fElapsed >= m_fDuration)
	{
		m_fElapsed = 0.f;
		SetActive(false);
	}

	else if (m_fElapsed <= m_fDuration * 0.5f)
	{
		if (m_bSpread == false)
		{
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("SpreadWater"), vPos);
		m_bSpread = true;

		_vector offsetUp = XMVectorSet(0.f, 15.f, 0.f, 0.f);

		_vector pos = XMLoadFloat3(&vPos);
		pos = XMVectorAdd(pos, offsetUp);
		XMStoreFloat3(&vPos, pos);
		CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("SpreadWater"), vPos);
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

void CTanTakEffect::Late_Update(_float fTimeDelta)
{
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

HRESULT CTanTakEffect::Render()
{

	if (m_Textures[TEX_NOISE])
	{
		if (FAILED(m_Textures[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom,"g_NoiseTexture",0)))
			return E_FAIL;
	}

	if (m_Textures[TEX_DISSOLVE])
	{
		if (FAILED(m_Textures[TEX_DISSOLVE]->Bind_ShaderResource(m_pShaderCom, "g_DissolveTexture", 0)))
			return E_FAIL;
	}

	if (m_Textures[TEX_ALPHA])
	{
		if (FAILED(m_Textures[TEX_DISSOLVE]->Bind_ShaderResource(m_pShaderCom, "g_DissolveEdgeTexture", 0)))
			return E_FAIL;
	}
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTanTakEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_Tak"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}

	m_Textures[TEX_NOISE] = m_pGameInstance->GetTexture(TEXT("TakNoise"), true);
	m_Textures[TEX_DISSOLVE] = m_pGameInstance->GetTexture(TEXT("TakDiss"), true);
	m_Textures[TEX_ALPHA] = m_pGameInstance->GetTexture(TEXT("TakDissEdge"), true); // 디졸브 경계에 그리기
	
	return S_OK;
}

HRESULT CTanTakEffect::Bind_Shader()
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

void CTanTakEffect::OnDisable()
{
	m_bSpread = false;
	m_vUVScale = _float2(1.f, 1.f); 
	m_vUVOffset = _float2(0.f, 0.f); 
	m_fElapsed = 0.f; 

}

void CTanTakEffect::OnEnable()
{
	_float3 vPos{};
	XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
	CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("WaterWave"), vPos);
	m_vUVScale = _float2(1.f,1.f); 
	m_vUVOffset = _float2(0.f, 0.f); 
	m_fElapsed = 0.f; 
}

CTanTakEffect* CTanTakEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTanTakEffect* pInstance = new CTanTakEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTanTakEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTanTakEffect::Clone(void* pArg)
{
	CTanTakEffect* pClone = new CTanTakEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTanTakEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CTanTakEffect::Free()
{
	__super::Free();
}
