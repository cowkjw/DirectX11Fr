#include "BaseCharacter.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "Weapon.h"	


CBaseCharacter::CBaseCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
	m_pInputBuffer = CInputBuffer::Create();
}

CBaseCharacter::CBaseCharacter(const CBaseCharacter& Prototype)
	: CGameObject(Prototype)
	, m_pShaderCom{ Prototype.m_pShaderCom }
	, m_pModelCom{ Prototype.m_pModelCom }
	, m_pColliderCom{ Prototype.m_pColliderCom }
	, m_pAnimatroCom{ Prototype.m_pAnimatroCom }
	, m_fMaxHP{ Prototype.m_fMaxHP }
	, m_fCurrentHP{ Prototype.m_fCurrentHP }
	, m_fStamina{ Prototype.m_fStamina }
	, m_pWeapon{ Prototype.m_pWeapon }
	, m_pInputBuffer{ Prototype.m_pInputBuffer }
	, m_pTarget{Prototype.m_pTarget }
{
}

HRESULT CBaseCharacter::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CBaseCharacter::Initialize(void* pArg)
{

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));


	return S_OK;
}

void CBaseCharacter::Priority_Update(_float fTimeDelta)
{

	static _uint iAnim = 0;
	if (m_pGameInstance->IsKeyPressed('N'))
	{
		m_pAnimatroCom->Set_Animation(iAnim, 0.15f);
		iAnim++;
	}

	if (m_pGameInstance->IsKeyPressed('M'))
	{
		m_pAnimatroCom->Set_Animation(iAnim, 0.15f);
		iAnim = max(0, iAnim - 1);
	}
}

void CBaseCharacter::Update(_float fTimeDelta)
{

	m_fTotalTime += fTimeDelta;
	m_pInputBuffer->Update(m_fTotalTime);
	HandleInput();


	// 3) 애니메이션 업데이트
	m_pAnimatroCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);

	if (m_pState)
	{
		m_pState->Update(this, fTimeDelta);
	}
	if (m_IsKnockback)
	{
		// (1) 속도에 중력 적용
		XMVECTOR velVec = XMLoadFloat3(&m_Velocity);
		XMVECTOR gravityVec = XMLoadFloat3(&GRAVITY) * fTimeDelta;
		//velVec += gravityVec;

		// (2) 위치 갱신
		XMVECTOR posVec = m_pTransformCom->Get_State(STATE::POSITION);
		XMVECTOR deltaPos = velVec * fTimeDelta;
		posVec += deltaPos;

		// (3) 땅 충돌 및 반사
		float newY = XMVectorGetY(posVec);
		//if (newY <= groundY)
		//{
		//	posVec = XMVectorSetY(posVec, groundY);
		//	velVec = XMVectorSetY(velVec, -XMVectorGetY(velVec) * RESTITUTION);

		//	// y 속도가 충분히 작아지면 넉백 종료
		XMVECTOR horizVel = XMVectorSetY(velVec, 0.0f);
		// 길이(크기) 계산
		float   speedXZ = XMVectorGetX(XMVector3Length(horizVel));

		// 속도가 임계값 이하가 되면 넉백 종료
		const float stopThreshold = 0.1f;
		if (speedXZ < stopThreshold)
		{
			m_IsKnockback = false;
			velVec = XMVectorZero();  // 완전히 멈춤
		}

		// (4) 상태 저장 및 Transform 적용
		XMStoreFloat3(&m_Velocity, velVec);
		m_pTransformCom->Set_State(STATE::POSITION, posVec);
	}
	else
	{
		// 넉백이 아닐 땐 Velocity를 0으로 유지
		m_Velocity = { 0,0,0 };
	}
}

void CBaseCharacter::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);

	//_float3 tmp{};  
	//XMStoreFloat3(&tmp, m_pTransformCom->Get_State(STATE::POSITION));  
	//if (m_pGameInstance->IsAABBInFrustum(tmp, m_pTransformCom->Get_Scaled()))  
	//{  
	//  
	//}
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CBaseCharacter::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;


	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	//for (_uint i = 0; i < iNumMesh; i++)
	//{
	//	if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
	//		return E_FAIL;

	//	if (FAILED(m_pShaderCom->Begin(0)))
	//		return E_FAIL;

	//	if (FAILED(m_pModelCom->Render(i)))
	//		return E_FAIL;
	//}


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

void CBaseCharacter::Set_Weapon(const char* boneName, CWeapon* pWeapon)
{
	m_pWeapon = pWeapon;

	CBone* pBoneRHand = m_pModelCom->Get_Bone(boneName);

	if (pBoneRHand && m_pWeapon)
	{
		m_pWeapon->Set_BoneSocket(pBoneRHand);
	}
}

void CBaseCharacter::ChangeState(IState* pState)
{
	if (m_pState)
	{
		m_pState->Exit(this);

	}
	if (pState)
	{
		pState->Enter(this);
	}
	Safe_Delete(m_pState);
	m_pState = pState;
}

void CBaseCharacter::HandleInput()
{
	if (m_pGameInstance->IsKeyPressed('J')) m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('K')) m_pInputBuffer->AddCommand({ ECommand::Jump, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('L')) m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('O')) m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('I')) m_pInputBuffer->AddCommand({ ECommand::Skill0, m_fTotalTime });
}

void CBaseCharacter::Set_Target(const _wstring& name, LEVEL eLevel)
{
	m_pTarget = dynamic_cast<CBaseCharacter*>(m_pGameInstance->Find_GameObjectByName(ToIndex(eLevel), name));

	if (!m_pTarget)
	{
		MSG_BOX("Target not found");
		return;
	}
}



void CBaseCharacter::OnCollisionEnter(CCollider* other)
{
	//if (auto pOtherChar = dynamic_cast<CBaseCharacter*>(other->GetOwner()))
	//{
	//	if (pOtherChar->IsKnockback()) return;
	//	XMVECTOR myVel = XMLoadFloat3(&m_Velocity);
	//	// 두 캐릭터 중심 차이로 근사한 법선
	//	_vector myPos = this->GetTransform()->Get_State(STATE::POSITION);
	//	_vector otherPos = pOtherChar->GetTransform()->Get_State(STATE::POSITION);
	//	XMVECTOR normal = XMVector3Normalize(
	//		otherPos - myPos);

	//	// 법선과 내 속도의 내적(dot)
	//	float dp = XMVectorGetX(XMVector3Dot(normal, myVel));

	//	// dp > 0 이면 내 속도가 법선 방향과 유사 → 내가 공격자
	//	// dp < 0 이면 내 속도가 법선과 반대 → 내가 피격자
	//	if (dp < 0.0f)
	//	{

	//		//_vector myPos = this->GetTransform()->Get_State(STATE::POSITION);
	//		//_vector otherPos = pOtherChar->GetTransform()->Get_State(STATE::POSITION);


	//	}
	//	// 2) 방향 계산 (상대 → 나 방향을 반대로 해서 밀어냄)
	//	XMVECTOR dirVec = otherPos - myPos;
	//	dirVec = XMVector3Normalize(dirVec);

	//	// 3) 세기 결정 (원하는 넉백 세기)
	//	const float pushStrength = 5.f;  // 예시, 값 조정

	//	// 4) 상대 캐릭터에 넉백 속도 추가
	//	//    CBaseCharacter에 mVelocity 같은 멤버가 있다고 가정
	//	XMFLOAT3 pushVel;
	//	XMStoreFloat3(&pushVel, dirVec * pushStrength);

	//	auto vVel = pOtherChar->GetVelocity(); // 현재 속도 가져오기

	//	vVel.x += pushVel.x;
	//	vVel.y += pushVel.y;
	//	vVel.z += pushVel.z;

	//	pOtherChar->SetKnockback(true); // 넉백 상태로 설정
	//	pOtherChar->SetVelocity(vVel); // 넉백 속도 적용
	//}
}


void CBaseCharacter::OnCollisionStay(CCollider* other, _float fTimeDelta)
{

}


void CBaseCharacter::OnCollisionExit(CCollider* other)
{
}

void CBaseCharacter::Ready_Animation()
{

}

HRESULT CBaseCharacter::Ready_Components()
{

	///* For.Com_Shader */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
	//	TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	//if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	//if (FAILED(__super::Add_Component(TEXT("Com_Shader"), m_pGameInstance->GetShader(TEXT("Shader_VtxAnimMesh"), true), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;


	///* For.Com_Model */
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kyoujuro"),
	//	TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	//	return E_FAIL;

	///* For.Com_AnimController*/
	//if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
	//	TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatroCom), m_pModelCom)))
	//	return E_FAIL;

	return S_OK;
}

CBaseCharacter* CBaseCharacter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBaseCharacter* pInstance = new CBaseCharacter(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBaseCharacter");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBaseCharacter::Clone(void* pArg)
{
	CBaseCharacter* pInstance = new CBaseCharacter(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBaseCharacter");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CBaseCharacter::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	Safe_Release(m_pAnimatroCom);
	Safe_Release(m_pColliderCom);
	Safe_Delete(m_pState);
	Safe_Release(m_pInputBuffer);
}


