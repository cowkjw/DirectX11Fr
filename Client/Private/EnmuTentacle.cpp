#include "EnmuTentacle.h"
#include "GameInstance.h"
#include "Model.h"
#include "BodyColliderParts.h"

CEnmuTentacle::CEnmuTentacle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pModelCom(nullptr)
	, m_pAnimatorCom(nullptr)
	, m_pBodyCollider(nullptr)
	, m_fTimeElapsed(0.f)
{
}
CEnmuTentacle::CEnmuTentacle(const CEnmuTentacle& rhs)
	: CGameObject(rhs)
	, m_fTimeElapsed(rhs.m_fTimeElapsed)
	, m_pShaderCom(rhs.m_pShaderCom)
	, m_pAnimatorCom(nullptr)
	, m_pBodyCollider(nullptr)
{
}


HRESULT CEnmuTentacle::Initialize_Prototype()
{
	if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnmuTentacle::Initialize(void* pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	GAMEOBJECT_DESC			Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("EnmuTentacle");


	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
	m_pTransformCom->Scaling(_float3(0.05f, 0.05f, 0.05f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, -18.f,0.f, 1.f));
	return S_OK;
}

void CEnmuTentacle::Update(_float fTimeDelta)
{


	m_pAnimatorCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);

	for (auto& child : m_vecChildren)
	{	
		if (!child->IsActive())
			continue;
		child->Update(fTimeDelta);
	}
}
 
void CEnmuTentacle::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
	for (auto& child : m_vecChildren)
	{
		if (!child->IsActive())
			continue;
		child->Late_Update(fTimeDelta);
	}

}

HRESULT CEnmuTentacle::Render()
{
	if (FAILED(Bind_Shaders()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

void CEnmuTentacle::PlayAniamtion()
{
	m_bAnimPlaying = true;
	m_pBodyCollider->SetActive(true); // 콜라이더 활성화
	m_pAnimatorCom->PlayClip(m_pModelCom->GetAnimationClipByName("A_P1011_V00_C90_AtkThrow01_WepV33"));
}

void CEnmuTentacle::OnEnable()
{
	m_pModelCom->Play_Animation(0.f); // 애니메이션 초기화
	m_pBodyCollider->SetActive(false); // 콜라이더도 비활성화
}

void CEnmuTentacle::OnDisable()
{
	m_pBodyCollider->SetActive(false); // 콜라이더도 비활성화
	auto anim = m_pModelCom->GetAnimationClipByName("A_P1011_V00_C90_AtkThrow01_WepV33");
	m_bAnimPlaying = false;
	m_pAnimatorCom->StopAnimation();
	anim->ResetTrack();
	anim->SetCurrentTrackPosition(0.f);
}

HRESULT CEnmuTentacle::Ready_Components()
{
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::ENMU_BOSS), TEXT("Prototype_Component_Model_EnmuTentacle"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatorCom), m_pModelCom)))
		return E_FAIL;


	auto Bones = m_pModelCom->Get_Bones();

	CBodyColliderParts::BODYCOLLIDERPARTS_DESC desc{};
	desc.vColliderOffsets.push_back(_float3(0.f, 0.f, 0.f));
	desc.fRadius = 5.f;

	m_pBodyCollider = CBodyColliderParts::Create(m_pDevice, m_pContext);
	m_pBodyCollider->Set_BoneSocket(m_pModelCom->Get_Bone("C_entaclesArmA_End"));
	this->AddChild(m_pBodyCollider);
	m_pBodyCollider->Initialize(&desc);

	return S_OK;
}

HRESULT CEnmuTentacle::Bind_Shaders()
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

void CEnmuTentacle::Ready_Animations()
{
	auto anim = m_pModelCom->GetAnimationClipByName("A_P1011_V00_C90_AtkThrow01_WepV33");
	anim->SetLoop(false);
}

CEnmuTentacle* CEnmuTentacle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnmuTentacle* pInstance = new CEnmuTentacle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnmuTentacle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEnmuTentacle::Clone(void* pArg)
{
	CEnmuTentacle* pInstance = new CEnmuTentacle(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnmuTentacle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEnmuTentacle::Free()
{
	__super::Free();
	if (!m_bIsCloned)
	{
		Safe_Release(m_pShaderCom);
	}
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimatorCom);
	Safe_Release(m_pBodyCollider);
}
