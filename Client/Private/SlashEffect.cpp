#include "SlashEffect.h"
#include "GameInstance.h"

CSlashEffect::CSlashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect(pDevice, pContext)
{
}
CSlashEffect::CSlashEffect(const CSlashEffect& Prototype)
	: CMeshEffect(Prototype),
	m_vColor(Prototype.m_vColor)
{

}

HRESULT CSlashEffect::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;
}

HRESULT CSlashEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_iShaderPass = 3; // 일단 2번으로 테스트
	m_pTransformCom->Scaling(_float3(1.f, 1.f, 1.f));
	// 테스트용 컬러
	m_vColor = _float4(1.f, 0.7f,0.0f, 0.85f); // 주황 느낌
	m_bUseOffset = true;
	return S_OK;
}

void CSlashEffect::Priority_Update(_float fTimeDelta)
{
}

void CSlashEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_bUseOffset)
	{
		m_fUVOffset.x += fTimeDelta*2.f; // UV 애니메이션 속도 조절
		//m_fUVOffset.y += fTimeDelta * 0.5f; // UV 애니메이션 속도 조절
		if (m_fUVOffset.x >= 1.f)
		{
			m_bUseOffset = false;
			SetActive(false);
			m_fUVOffset.x = 0.f;
		}
		if (m_fUVOffset.y > 1.f)
			m_fUVOffset.y = 0.f;
	}

}

void CSlashEffect::Late_Update(_float fTimeDelta)
{
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();

		_matrix matScale = XMMatrixScaling(30.f, 30.f, 30.f);
		_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
		//world = XMMatrixMultiply(matScale, world); 

		_float4x4 WorldMatrix{};
		XMStoreFloat4x4(&WorldMatrix, world);
		m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	}
	__super::Late_Update(fTimeDelta);
}

HRESULT CSlashEffect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CSlashEffect::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_DefaultSlash"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CSlashEffect::Bind_Shader()
{
	__super::Bind_Shader();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fUVOffset", &m_fUVOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	return S_OK;
}

void CSlashEffect::OnDisable()
{
	m_fUVOffset = _float2(0.5f, 0.f); // UV 오프셋 초기화
}

void CSlashEffect::OnEnable()
{
	m_fUVOffset = _float2(0.5f, 0.f); // UV 오프셋 초기화
	m_bUseOffset = true; // UV 애니메이션 활성화
}

CSlashEffect* CSlashEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlashEffect* pInstance = new CSlashEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CSlashEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CSlashEffect::Clone(void* pArg)
{
	CSlashEffect* pClone = new CSlashEffect(*this);
	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CSlashEffect");
		Safe_Release(pClone);
	}
	return pClone;
}

void CSlashEffect::Free()
{
	__super::Free();
}
