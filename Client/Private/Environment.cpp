#include "Environment.h"
#include "Shader.h"
#include "Model.h"
#include "GameInstance.h"

CEnvironment::CEnvironment(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}
CEnvironment::CEnvironment(const CEnvironment& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom{ Prototype.m_pShaderCom }
	, m_pModelCom{ Prototype.m_pModelCom }
{
}

HRESULT CEnvironment::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;

}

HRESULT CEnvironment::Initialize(void* pArg)
{
	if (pArg)
	{
		ENVIRONMENT_DESC* pDesc = static_cast<ENVIRONMENT_DESC*>(pArg);

		m_strModelTag = pDesc->strModelTag;
		if (FAILED(__super::Initialize(pDesc)))
			return E_FAIL;
		if (FAILED(__super::Add_Component(TEXT("Com_Model"), m_pGameInstance->GetModel(m_strModelTag, true), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;

	}
	else
	{
		GAMEOBJECT_DESC			Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 40.f;
		if (FAILED(__super::Initialize(&Desc)))
			return E_FAIL;
	}
	
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), m_strModelTag,
	//	TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	//	return E_FAIL;
	m_pTransformCom->Scaling(_float3(0.2f, 0.2f, 0.2f));
	return S_OK;
}

void CEnvironment::Update(_float fTimeDelta)
{
}

void CEnvironment::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CEnvironment::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;
}

json CEnvironment::Serialize()
{
	json j = CGameObject::Serialize();

	j["modelTag"] =WStringToString( m_strModelTag);
	return j;
}

void CEnvironment::Deserialize(const json& j)
{
	CGameObject::Deserialize(j);
	if (j.contains("modelTag"))
		m_strModelTag = StringToWString(j["modelTag"].get<string>());
	if (FAILED(__super::Add_Component(TEXT("Com_Model"), m_pGameInstance->GetModel(m_strModelTag, true), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return;
//	m_pTransformCom->Scaling(_float3(0.2f, 0.2f, 0.2f));*/
}

HRESULT CEnvironment::Ready_Components()
{

	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	return S_OK;
}

CEnvironment* CEnvironment::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvironment* pInstance = new CEnvironment(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvironment");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEnvironment::Clone(void* pArg)
{
	CEnvironment* pInstance = new CEnvironment(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvironment");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CEnvironment::Free()
{
	__super::Free();
	if (!m_bIsCloned)
	{
		Safe_Release(m_pShaderCom);
	}
	Safe_Release(m_pModelCom);
}
