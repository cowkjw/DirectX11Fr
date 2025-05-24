#include "Sky.h"
#include "Shader.h"
#include "Model.h"
#include "GameInstance.h"

CSky::CSky(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}
CSky::CSky(const CSky& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom{ Prototype.m_pShaderCom }
	, m_pModelCom{ Prototype.m_pModelCom }
{
}

HRESULT CSky::Initialize_Prototype()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;
	return S_OK;

}

HRESULT CSky::Initialize(void* pArg)
{
	GAMEOBJECT_DESC Desc{};
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("Sky");
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
	m_pTransformCom->Scaling(_float3(0.2f, 0.2f, 0.2f));
	return S_OK;
}

void CSky::Update(_float fTimeDelta)
{
}

void CSky::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CSky::Render()
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

HRESULT CSky::Ready_Components()
{

	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Sky"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;
	return S_OK;
}

CSky* CSky::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSky* pInstance = new CSky(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CSky");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CSky::Clone(void* pArg)
{
	CSky* pInstance = new CSky(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CSky");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CSky::Free()
{
	__super::Free();
	if (!m_bIsCloned)
	{
		Safe_Release(m_pShaderCom);
		Safe_Release(m_pModelCom);

	}
}
