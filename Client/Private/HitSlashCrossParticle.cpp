#include "HitSlashCrossParticle.h"
#include "GameInstance.h"
#include "JsonLoader.h"

CHitSlashCrossParticle::CHitSlashCrossParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect(pDevice, pContext)
{
	m_iShaderPass = 2; // 쉐이더 패스 
}

CHitSlashCrossParticle::CHitSlashCrossParticle(const CHitSlashCrossParticle& Prototype)
	: CParticleEffect(Prototype)
{
}

HRESULT CHitSlashCrossParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CHitSlashCrossParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	SetTextureIndex(2);

    return S_OK;
}

void CHitSlashCrossParticle::Priority_Update(_float fTimeDelta)
{
}

void CHitSlashCrossParticle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	m_pTransformCom->Turn(XMVectorSet(0.f, 0.f, 1.f, 0.f), fTimeDelta); // Y축으로 90도 회전
}

void CHitSlashCrossParticle::Late_Update(_float fTimeDelta)
{
	//__super::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
}

HRESULT CHitSlashCrossParticle::Render()
{
	if (FAILED(m_Textures[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;
	__super::Render();
    return S_OK;
}

HRESULT CHitSlashCrossParticle::Ready_Components()
{	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxPointInstance"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Particle */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Particle"),
	//	TEXT("Com_Particle"), reinterpret_cast<CComponent**>(&m_ParticleSystems[L"Slash"]))))
	//	return E_FAIL;


	////// 테스트용 
	//CJsonLoader jsonLoader(m_pDevice,m_pContext);

	//jsonLoader.Load_Particle("../Asset/Json/Particle/HitCross_Particle.json", &m_ParticleSystems[L"TEST"]);
	//jsonLoader.Free();

	/* For.Com_Texture */

	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("CrossMask"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_MASK]))))
		return E_FAIL;

	return S_OK;
}

CHitSlashCrossParticle* CHitSlashCrossParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHitSlashCrossParticle* pInstance = new CHitSlashCrossParticle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CHitSlashCrossParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CHitSlashCrossParticle::Clone(void* pArg)
{
	CHitSlashCrossParticle* pInstance = new CHitSlashCrossParticle(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CHitSlashCrossParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CHitSlashCrossParticle::Free()
{
	__super::Free();
}
