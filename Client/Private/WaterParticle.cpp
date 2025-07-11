#include "WaterParticle.h"
#include "GameInstance.h"
#include "JsonLoader.h"

CWaterParticle::CWaterParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect(pDevice, pContext)
{
	m_iShaderPass = 1; // 쉐이더 패스 
}

CWaterParticle::CWaterParticle(const CWaterParticle& Prototype)
	: CParticleEffect(Prototype)
{
}

HRESULT CWaterParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWaterParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iTextureIndex = 9;
    return S_OK;
}

void CWaterParticle::Priority_Update(_float fTimeDelta)
{
}

void CWaterParticle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWaterParticle::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
}

HRESULT CWaterParticle::Render()
{
	if (FAILED(m_Textures[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;
	__super::Render();
    return S_OK;
}

void CWaterParticle::SetStartColor(const _float3& vColor)
{ 
	if (m_ParticleSystems.find(L"Slash") != m_ParticleSystems.end())
	{
		auto Desc = m_ParticleSystems[L"Slash"]->GetParticleDesc();
		Desc.vStartColor = vColor;
		m_ParticleSystems[L"Slash"]->SetParticleDesc(Desc);
	}
}

HRESULT CWaterParticle::Ready_Components()
{	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxPointInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Particle */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
	//	TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_ParticleSystems[L"Slash"]))))
	//	return E_FAIL;


	// 테스트용 
	//CJsonLoader jsonLoader(m_pDevice,m_pContext);

	//jsonLoader.Load_Particle("../Asset/Json/Particle/HitCross_Particle.json", &m_ParticleSystems[L"TEST"]);
	//jsonLoader.Free();

	/* For.Com_Texture */

	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("ParticleMask"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_MASK]))))
		return E_FAIL;

	return S_OK;
}

CWaterParticle* CWaterParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWaterParticle* pInstance = new CWaterParticle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWaterParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWaterParticle::Clone(void* pArg)
{
	CWaterParticle* pInstance = new CWaterParticle(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CWaterParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWaterParticle::Free()
{
	__super::Free();
}
