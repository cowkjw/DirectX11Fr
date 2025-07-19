#include "HitShockParticle.h"
#include "GameInstance.h"
#include "JsonLoader.h"

CHitShockParticle::CHitShockParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect(pDevice, pContext)
{
	m_iShaderPass = 3; // 쉐이더 패스 
}

CHitShockParticle::CHitShockParticle(const CHitShockParticle& Prototype)
	: CParticleEffect(Prototype)
	, m_InitParticleUV(Prototype.m_InitParticleUV)
	, m_UseParticleUV(Prototype.m_UseParticleUV)
{
}

HRESULT CHitShockParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CHitShockParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	//m_InitParticleUV.fCols = static_cast<_float>(3);
	//m_InitParticleUV.fRows = static_cast<_float>(3);
	//m_InitParticleUV.iTotalFrames = 9;
	//m_InitParticleUV.fFrameTime = 0.005f;

	//// 한 프레임 크기
	//m_InitParticleUV.vUVScale.x = 1.f / m_InitParticleUV.fCols;
	//m_InitParticleUV.vUVScale.y = 1.f / m_InitParticleUV.fRows;

	//// 초기화
	//m_InitParticleUV.fCurrentTime = 0.f;
	//m_InitParticleUV.iCurrentFrame = 0;
	//m_InitParticleUV.vUVOffset.x = 0.f;
	//m_InitParticleUV.vUVOffset.y = 0.f;

	//m_UseParticleUV = m_InitParticleUV;
    return S_OK;
}

void CHitShockParticle::Priority_Update(_float fTimeDelta)
{
}

void CHitShockParticle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	m_pTransformCom->Turn(XMVectorSet(0.f, 0.f, 1.f, 0.f), fTimeDelta); // Y축으로 90도 회전

	m_UseParticleUV.fCurrentTime += fTimeDelta;
	if (m_UseParticleUV.fCurrentTime >= m_UseParticleUV.fFrameTime)
	{
		m_UseParticleUV.fCurrentTime = 0.f;
		m_UseParticleUV.iCurrentFrame = (m_UseParticleUV.iCurrentFrame + 1) % m_UseParticleUV.iTotalFrames;

		// 현재 프레임의 행/열 계산
		_int row = m_UseParticleUV.iCurrentFrame / static_cast<_int>(m_UseParticleUV.fCols);
		_int col = m_UseParticleUV.iCurrentFrame % static_cast<_int>(m_UseParticleUV.fCols);

		// UV 오프셋 계산
		m_UseParticleUV.vUVOffset.x = col * m_UseParticleUV.vUVScale.x;
		m_UseParticleUV.vUVOffset.y = row * m_UseParticleUV.vUVScale.y;
	}

	if (m_UseParticleUV.iTotalFrames <= m_UseParticleUV.iCurrentFrame)
	{

			SetActive(false); // 비활성화
	}
}

void CHitShockParticle::Late_Update(_float fTimeDelta)
{
	//__super::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLOOM_EFFECT, this);
}

HRESULT CHitShockParticle::Render()
{
	m_pShaderCom->Bind_RawValue("g_uvOffset", &m_UseParticleUV.vUVOffset, sizeof(_float2));
	m_pShaderCom->Bind_RawValue("g_uvScale", &m_UseParticleUV.vUVScale, sizeof(_float2));
	if (FAILED(m_Textures[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
		return E_FAIL;
	__super::Render();
    return S_OK;
}

void CHitShockParticle::SpwanParticle()
{
	m_UseParticleUV = m_InitParticleUV; // 파티클 초기화
	__super::SpwanParticle();
}

void CHitShockParticle::DespwanParticle()
{
	__super::DespwanParticle();
}

void CHitShockParticle::SetInitParticleUV(_int col,_int row,_float fFramTime)
{
	m_InitParticleUV.fCols = static_cast<_float>(col);
	m_InitParticleUV.fRows = static_cast<_float>(row);
	m_InitParticleUV.iTotalFrames = m_InitParticleUV.fCols * m_InitParticleUV.fRows;
	m_InitParticleUV.fFrameTime = fFramTime;

	// 한 프레임 크기
	m_InitParticleUV.vUVScale.x = 1.f / m_InitParticleUV.fCols;
	m_InitParticleUV.vUVScale.y = 1.f / m_InitParticleUV.fRows;

	// 초기화
	m_InitParticleUV.fCurrentTime = 0.f;
	m_InitParticleUV.iCurrentFrame = 0;
	m_InitParticleUV.vUVOffset.x = 0.f;
	m_InitParticleUV.vUVOffset.y = 0.f;
	m_UseParticleUV = m_InitParticleUV; // 사용 UV도 초기화

}

HRESULT CHitShockParticle::Ready_Components()
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

	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(TEXT("Shock"), true), reinterpret_cast<CComponent**>(&m_Textures[TEX_MASK]))))
		return E_FAIL;

	return S_OK;
}

CHitShockParticle* CHitShockParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHitShockParticle* pInstance = new CHitShockParticle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CHitShockParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CHitShockParticle::Clone(void* pArg)
{
	CHitShockParticle* pInstance = new CHitShockParticle(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CHitShockParticle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CHitShockParticle::Free()
{
	__super::Free();
}
