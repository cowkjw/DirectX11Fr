#include "FireSlashEffect.h"
#include "GameInstance.h"

CFireSlashEffect::CFireSlashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect( pDevice, pContext )
{
}
CFireSlashEffect::CFireSlashEffect(const CFireSlashEffect& Prototype)
	: CMeshEffect(Prototype)
{

}

HRESULT CFireSlashEffect::Initialize_Prototype()
{
	if(FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

HRESULT CFireSlashEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iShaderPass = 2; // 일단 2번으로 테스트
	m_pTransformCom->Scaling(_float3(100.f, 100.f, 100.f));
	//m_bUseOffset = true;
	return S_OK;
}

void CFireSlashEffect::Priority_Update(_float fTimeDelta)
{
}

void CFireSlashEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_bUseOffset)
	{
	m_vUVOffset.x += fTimeDelta * 0.5f; // UV 애니메이션 속도 조절
	m_vUVOffset.y += fTimeDelta * 0.5f; // UV 애니메이션 속도 조절
	if (m_vUVOffset.x > 1.f)
		m_vUVOffset.x = 0.f;
	if (m_vUVOffset.y > 1.f)
		m_vUVOffset.y = 0.f;
	}
	
}

void CFireSlashEffect::Late_Update(_float fTimeDelta)
{
	
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLUR_EFFECT, this);
}

HRESULT CFireSlashEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CFireSlashEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_FireSlash2"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CFireSlashEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vUVOffset, sizeof(_float2))))
		return E_FAIL;
	return S_OK;
}

CFireSlashEffect* CFireSlashEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFireSlashEffect* pInstance = new CFireSlashEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFireSlashEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFireSlashEffect::Clone(void* pArg)
{
	CFireSlashEffect* pClone = new CFireSlashEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CFireSlashEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CFireSlashEffect::Free()
{
	__super::Free();
}
