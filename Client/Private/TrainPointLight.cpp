#include "TrainPointLight.h"
#include "GameInstance.h"

CTrainPointLight::CTrainPointLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{

}
CTrainPointLight::CTrainPointLight(const CTrainPointLight& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom(Prototype.m_pShaderCom)
	, m_pTextureCom(Prototype.m_pTextureCom)
	, m_pVIBufferCom(Prototype.m_pVIBufferCom)
	, m_vColor(Prototype.m_vColor)
{
	Safe_AddRef(m_pShaderCom);
	Safe_AddRef(m_pTextureCom);
	Safe_AddRef(m_pVIBufferCom);
}

HRESULT CTrainPointLight::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrainPointLight::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC GameObjectDesc = {};
	GameObjectDesc.fSpeedPerSec = 0.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.f);
	GameObjectDesc.strName = TEXT("TrainPointLight");
	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;
	m_pTransformCom->Scaling(_float3(300.f, 300.f, 1.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(3.87, 20.f, 250.f, 1.f));
	m_vColor = _float4(1.0f, 0.95f, 0.4f, 1.f);
	return S_OK;
}

void CTrainPointLight::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::EFFECT, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::BLUR_EFFECT, this);
}

HRESULT CTrainPointLight::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Begin(1)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CTrainPointLight::Ready_Components()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxPosTex"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	if (FAILED(__super::Add_Component(TEXT("Com_Texture"), m_pGameInstance->GetTexture(L"TrainPointLight", true), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;
	return S_OK;
}

HRESULT CTrainPointLight::Bind_Shader()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_WorldMatrix())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	
	_float fCameraFar = m_pGameInstance->Get_CameraFar();

		if (FAILED(m_pShaderCom->Bind_RawValue("g_fCameraFar", &fCameraFar, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
			return E_FAIL;

	return S_OK;
}

CTrainPointLight* CTrainPointLight::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrainPointLight* pInstance = new CTrainPointLight(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTrainPointLight");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTrainPointLight::Clone(void* pArg)
{
	CTrainPointLight* pInstance = new CTrainPointLight(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTrainPointLight");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTrainPointLight::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}


