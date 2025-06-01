#include "BodyColliderParts.h"
#include <BaseCharacter.h>
#include "GameInstance.h"

CBodyColliderParts::CBodyColliderParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)

{
}


CBodyColliderParts::CBodyColliderParts(const CBodyColliderParts& Prototype) :
	CGameObject(Prototype)
	, m_pColliderComs{ Prototype.m_pColliderComs }
{
}

HRESULT CBodyColliderParts::Initialize_Prototype()
{
	Ready_Components();
	return S_OK;
}

HRESULT CBodyColliderParts::Initialize(void* pArg)
{
	GAMEOBJECT_DESC			Desc{};

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 0.f;
	Desc.strName = TEXT("Body_Parts");

	if (pArg != nullptr)
	{
		BODYCOLLIDERPARTS_DESC* pDesc = reinterpret_cast<BODYCOLLIDERPARTS_DESC*>(pArg);
		auto vecOffsets =  pDesc->vColliderOffsets;

		m_pColliderComs.reserve(vecOffsets.size());

		for (size_t i = 0;i<vecOffsets.size();i++)
		{
			auto offset = vecOffsets[i];
			auto pCollider = CSphereCollider::Create(m_pDevice, m_pContext, 1.5f);
			pCollider->SetOffset(offset);
			pCollider->Initialize(nullptr);
			pCollider->SetListener(this);
			pCollider->SetColliderType(CCollider::ColliderType::HITBOX);
			pCollider->SetActive(false); // 초기에는 비활성화
			m_pColliderComs.push_back(pCollider);
			Add_Component(TEXT("Com_Collider" + to_wstring(i)), pCollider, reinterpret_cast<CComponent**>(&m_pColliderComs.back()));
		}
	}

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.3f, 0.3f, 0.3f));

	return S_OK;
}

void CBodyColliderParts::Priority_Update(_float fTimeDelta)
{
}

void CBodyColliderParts::Update(_float fTimeDelta)
{
	if (auto pCharacter = static_cast<CBaseCharacter*>(m_pParent))
	{
		_bool bCanAttack = pCharacter->GetState() == CBaseCharacter::CSTATE::ATTACK || pCharacter->GetState() == CBaseCharacter::CSTATE::SKILL;

		for (auto& pCollider : m_pColliderComs)
		{
			if (bCanAttack)
			{
				pCollider->SetActive(true);
				pCollider->SetDrawDebug(true);
			}
			else
			{
				pCollider->SetDrawDebug(false);
				pCollider->SetActive(false);
			}
		}
	}
}

void CBodyColliderParts::Late_Update(_float fTimeDelta)
{
	//CGameObject::Late_Update(fTimeDelta);
	if (m_pBoneSocket)
	{
		_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();
		_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();
		_matrix wepaonLocal = m_pTransformCom->Get_WorldMatrix_Inverse();
		_matrix world = XMMatrixMultiply(XMLoadFloat4x4(&boneLocal), XMLoadFloat4x4(&parentWorld));
		_matrix weaponWorld = XMMatrixMultiply(wepaonLocal, world);
		//_matrix World = XMMatrixMultiply(XMLoadFloat4x4(&m_pTransformCom->Get_WorldMatrix()),XMLoadFloat4x4(m_pBoneSocket->Get_CombinedTransformationMatrix()) );
		_float4x4 WorldMatrix{};
		XMStoreFloat4x4(&WorldMatrix, world);
		m_pTransformCom->Set_WorldMatrix(WorldMatrix);
		//	m_pTransformCom->Set_WorldMatrix();

	}

	//if (m_pBoneSocket)
	//{
	//	// 캐릭터 루트(부모)의 월드 행렬
	//	_float4x4 parentWorld = m_pParent->GetTransform()->Get_WorldMatrix();

	//	// 손 본의 로컬 행렬 (애니메이션에서 나온 CombinedTransformation)
	//	_float4x4 boneLocal = *m_pBoneSocket->Get_CombinedTransformationMatrix();

	//	// 무기의 로컬 오프셋 (초기 배치 조정용, 일반적으로 회전+위치)
	//	//_matrix weaponLocal = m_pTransformCom->Get_LocalMatrix(); // ← 새로 만들어야 함

	//	// 손 본의 월드 행렬 = 손 본 로컬 × 캐릭터 월드
	//	_matrix handWorld = XMLoadFloat4x4(&boneLocal) * XMLoadFloat4x4(&parentWorld);

	//	// 무기 월드 = 무기 로컬 × 손 본의 월드
	////	_matrix weaponWorld = weaponLocal * handWorld;

	//	// 적용
	//	_float4x4 finalMat{};
	//	XMStoreFloat4x4(&finalMat, weaponWorld);
	//	m_pTransformCom->Set_WorldMatrix(finalMat);
	//}

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CBodyColliderParts::Render()
{

	return S_OK;
}



HRESULT CBodyColliderParts::Ready_Components()
{

	return S_OK;
}


CBodyColliderParts* CBodyColliderParts::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBodyColliderParts* pInstance = new CBodyColliderParts(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBodyColliderParts");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBodyColliderParts::Clone(void* pArg)
{
	CBodyColliderParts* pInstance = new CBodyColliderParts(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBodyColliderParts");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBodyColliderParts::Free()
{
	__super::Free();

	for (auto& pCollider : m_pColliderComs)
	{
		Safe_Release(pCollider);
	}
	m_pColliderComs.clear();

}

void CBodyColliderParts::OnCollisionEnter(CCollider* other)
{
}

void CBodyColliderParts::OnCollisionStay(CCollider* other, float fTimeDelta)
{
}

void CBodyColliderParts::OnCollisionExit(CCollider* other)
{
}
