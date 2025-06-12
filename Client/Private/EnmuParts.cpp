#include "EnmuParts.h"
#include "GameInstance.h"

CEnmuParts::CEnmuParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pShaderCom(nullptr)
	, m_pModelCom(nullptr)
	, m_pAnimatorCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_fMaxHP(100.f)
	, m_fCurrentHP(100.f)
	, m_iShaderPass(0)
{
}
CEnmuParts::CEnmuParts(const CEnmuParts& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom(Prototype.m_pShaderCom)
	, m_pModelCom(nullptr)
	, m_pAnimatorCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_fMaxHP(Prototype.m_fMaxHP)
	, m_fCurrentHP(Prototype.m_fCurrentHP)
	, m_iShaderPass(Prototype.m_iShaderPass)
{
}

HRESULT CEnmuParts::Initialize(void* pArg)
{
	if (FAILED(CGameObject::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	return S_OK;
}

void CEnmuParts::Priority_Update(_float fTimeDelta)
{
}

void CEnmuParts::Update(_float fTimeDelta)
{
	
	m_pAnimatorCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);
}

void CEnmuParts::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CEnmuParts::Render()
{
	if (FAILED(Bind_Shaders()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CEnmuParts::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnmuParts::Bind_Shaders()
{

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
	//	return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_Light(0);


	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CEnmuParts::Ready_Animation()
{
}

void CEnmuParts::Ready_Collider()
{
}

void CEnmuParts::Free()
{
	__super::Free();
	if (!m_bIsCloned) // 쉐이더는 일단 공통으로 사용할듯 
	{
		Safe_Release(m_pShaderCom);
	}
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimatorCom);
	Safe_Release(m_pColliderCom);
}

void CEnmuParts::OnCollisionEnter(CCollider* other)
{
}

void CEnmuParts::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CEnmuParts::OnCollisionExit(CCollider* other)
{
}
