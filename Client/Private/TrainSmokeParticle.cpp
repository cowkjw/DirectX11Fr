#include "TrainSmokeParticle.h"
#include "GameInstance.h"
#include "JsonLoader.h"

CTrainSmokeParticle::CTrainSmokeParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect(pDevice, pContext)
{
	m_iShaderPass = 2; // 쉐이더 패스 
}

CTrainSmokeParticle::CTrainSmokeParticle(const CTrainSmokeParticle& Prototype)
	: CParticleEffect(Prototype)
{
}

HRESULT CTrainSmokeParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTrainSmokeParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iTextureIndex = 6;
	m_bUseParentTexture = true;
    return S_OK;
}

void CTrainSmokeParticle::Priority_Update(_float fTimeDelta)
{
}

void CTrainSmokeParticle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CTrainSmokeParticle::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
}

HRESULT CTrainSmokeParticle::Render()
{
	__super::Render();
    return S_OK;
}

void CTrainSmokeParticle::SetStartColor(const _float3& vColor)
{ 
}

HRESULT CTrainSmokeParticle::Ready_Components()
{	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxPointInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Particle */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
	//	TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_ParticleSystems[L"Slash"]))))
	//	return E_FAIL;

	CJsonLoader jsonLoader(m_pDevice,m_pContext);

	jsonLoader.Load_Particle("../Asset/Json/Particle/TrainSmoke_Particle.json", &m_ParticleSystems[L"TrainSmoke"]);
	jsonLoader.Free();

	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("ParticleMask"), true), reinterpret_cast<CComponent**>(&m_ParticleTextures[L"TrainSmoke"]))))
		return E_FAIL;
	m_ParticleTextureIndices[L"TrainSmoke"] = 6; // 텍스쳐 인덱스 설정

	return S_OK;
}

CTrainSmokeParticle* CTrainSmokeParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrainSmokeParticle* pInstance = new CTrainSmokeParticle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTrainSmokeParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTrainSmokeParticle::Clone(void* pArg)
{
	CTrainSmokeParticle* pInstance = new CTrainSmokeParticle(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CTrainSmokeParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTrainSmokeParticle::Free()
{
	__super::Free();
}
