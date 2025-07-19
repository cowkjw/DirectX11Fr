#include "FireParticle.h"
#include "GameInstance.h"
#include "JsonLoader.h"

CFireParticle::CFireParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect(pDevice, pContext)
{
	m_iShaderPass = 5; // 쉐이더 패스 
}

CFireParticle::CFireParticle(const CFireParticle& Prototype)
	: CParticleEffect(Prototype)
{
}

HRESULT CFireParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFireParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	EnableUVAnim(L"Fire",4.f, 4.f, 0.06f); // UV 애니메이션 설정
    return S_OK;
}

void CFireParticle::Priority_Update(_float fTimeDelta)
{

}

void CFireParticle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	
	for (auto& uv : m_ParticleUVs)
	{
		auto& uvProps = uv.second;
		uvProps.fCurrentTime += fTimeDelta;

		// 프레임 전환 체크
		if (uvProps.fCurrentTime >= uvProps.fFrameTime)
		{
			uvProps.fCurrentTime = 0.f;
			uvProps.iCurrentFrame = (uvProps.iCurrentFrame + 1) % uvProps.iTotalFrames;

			// 현재 프레임의 행/열 계산
			_int row = uvProps.iCurrentFrame / static_cast<_int>(uvProps.fCols);
			_int col = uvProps.iCurrentFrame % static_cast<_int>(uvProps.fCols);

			// UV 오프셋 계산
			uvProps.vUVOffset.x = col * uvProps.vUVScale.x;
			uvProps.vUVOffset.y = row * uvProps.vUVScale.y;
		}
	}

	
}

void CFireParticle::Late_Update(_float fTimeDelta)
{
//	__super::Late_Update(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
}

HRESULT CFireParticle::Render()
{
	if (FAILED(m_Textures[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;
	if (FAILED(m_Textures[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;
	if (FAILED(m_Textures[TEX_DISTORT]->Bind_ShaderResource(m_pShaderCom, "g_DistortionTexture", 0)))
		return E_FAIL;
	if (FAILED(m_Textures[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_uvOffset", &m_ParticleUVs[L"Fire"].vUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_uvScale", &m_ParticleUVs[L"Fire"].vUVScale, sizeof(_float2))))
		return E_FAIL;
	__super::Render();
    return S_OK;
}

void CFireParticle::SetStartColor(const _float3& vColor)
{ 
	if (m_ParticleSystems.find(L"Slash") != m_ParticleSystems.end())
	{
		auto Desc = m_ParticleSystems[L"Slash"]->GetParticleDesc();
		Desc.vStartColor = vColor;
		m_ParticleSystems[L"Slash"]->SetParticleDesc(Desc);
	}
}

HRESULT CFireParticle::Ready_Components()
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

	//if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("ParticleMask"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_MASK]))))
	//	return E_FAIL;


	m_Textures[TEX_DIFFUSE] =   m_pGameInstance->LoadTexture(TEXT("FireParticleDiff"),L"../Asset/Resources/Textures/Effect/Particle/Fire/T_e_Plc_P0012_Fire020.dds",true);
	m_Textures[TEX_DISTORT] = m_pGameInstance->LoadTexture(TEXT("FireParticleDist"), L"../Asset/Resources/Textures/Effect/Particle/Fire/T_e_cmn_FireDist001N.dds", true);
	m_Textures[TEX_MASK] = m_pGameInstance->LoadTexture(TEXT("FireParticleMask"), L"../Asset/Resources/Textures/Effect/Particle/Fire/T_e_Plc_P0012_Fire020_Alpha.dds", true);
	m_Textures[TEX_NOISE] = m_pGameInstance->LoadTexture(TEXT("FireParticleNoise"), L"../Asset/Resources/Textures/Effect/Particle/Fire/T_e_Cmn_Noise001.dds", true);

	return S_OK;
}

CFireParticle* CFireParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFireParticle* pInstance = new CFireParticle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFireParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFireParticle::Clone(void* pArg)
{
	CFireParticle* pInstance = new CFireParticle(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CFireParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CFireParticle::Free()
{
	__super::Free();
}
