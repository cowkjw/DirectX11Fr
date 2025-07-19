#include "Effect.h"
#include "ParticleSystem.h"
#include "Shader.h"
#include "Texture.h"
#include "GameInstance.h"

CEffect::CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext),
	m_iShaderPass(0)
{
	m_Textures.fill(nullptr);
	XMStoreFloat4x4(&m_CombinedWorldMatrix, XMMatrixIdentity());
}

CEffect::CEffect(const CEffect& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom(Prototype.m_pShaderCom)
	, m_iShaderPass(Prototype.m_iShaderPass)
	, m_fDuration(Prototype.m_fDuration)
	, m_fElapsed(Prototype.m_fElapsed)
	, m_bLoop(Prototype.m_bLoop)
	, m_CombinedWorldMatrix(Prototype.m_CombinedWorldMatrix)
	, m_iTextureIndex(Prototype.m_iTextureIndex)
{
	for (_uint i = 0; i < TEX_MAX; i++)
	{
		m_Textures[i] = Prototype.m_Textures[i];
		Safe_AddRef(m_Textures[i]);
	}
	Safe_AddRef(m_pShaderCom);
}

HRESULT CEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		GAMEOBJECT_DESC GameObjectDesc = {};
		GameObjectDesc.fSpeedPerSec = 30.f;
		GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.f);
		GameObjectDesc.strName = TEXT("Effect");

		if (FAILED(__super::Initialize(&GameObjectDesc)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

    return S_OK;
}

void CEffect::Priority_Update(_float fTimeDelta)
{
}

void CEffect::Update(_float fTimeDelta)
{
	//m_fElapsed += fTimeDelta;
	//if (!m_bLoop && m_fElapsed >= m_fDuration)
	//	SetActive(false);
}

void CEffect::Late_Update(_float fTimeDelta)
{
	//if (m_pBoneSocket)
	//{
	//	_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
	//	_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();

	//	_matrix matScale = XMMatrixScaling(10.f, 10.f, 10.f);  
	//	_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
	//	//world = XMMatrixMultiply(matScale, world); 

	//	_float4x4 WorldMatrix{};
	//	XMStoreFloat4x4(&WorldMatrix, world);
	//	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	//}
//	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONLIGHT, this);
}

HRESULT CEffect::Render()
{
    return S_OK;
}


void CEffect::OnEnable()
{
}

void CEffect::OnDisable()
{
}

HRESULT CEffect::Bind_Shader()
{
	/*if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;*/
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;


	//m_pShaderCom->Bind_RawValue("g_fToonThreshold", &fToonThreshold, sizeof(float));
	//m_pShaderCom->Bind_RawValue("g_vShadowColor", &vShadowColor, sizeof(float4));


	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;


	return S_OK;
}

void CEffect::Free()
{
	__super::Free();

	for (auto& texture : m_Textures)
	{
		Safe_Release(texture);
	}
	m_Textures.fill(nullptr);
	Safe_Release(m_pShaderCom);
}
