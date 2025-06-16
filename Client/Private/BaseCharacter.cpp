#include "BaseCharacter.h"
#include "GameInstance.h"
#include "Animation.h"
#include "StateIdle.h"
#include "InputBuffer.h"
#include "StateHurt.h"
#include "StateHurtAir.h"
#include "Weapon.h"	
#include "Environment.h"
#include "Navigation.h"


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
	, m_pAnimatorCom{ Prototype.m_pAnimatorCom }
	, m_fMaxHP{ Prototype.m_fMaxHP }
	, m_fCurrentHP{ Prototype.m_fCurrentHP }
	, m_fStamina{ Prototype.m_fStamina }
	, m_pWeapon{ Prototype.m_pWeapon }
	, m_pInputBuffer{ CInputBuffer::Create() }
	, m_pTarget{ Prototype.m_pTarget }
	, m_iShaderPass{ Prototype.m_iShaderPass }
	, m_pState{ nullptr }
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

	if (FAILED(CBaseCharacter::Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	m_fTotalTime = 0.f;

	m_pAnimatorCom->RegisterEventListener("HitedAir", [&](const string& eventName) {
		LaunchAirborne(25.f);
		});

	m_pAnimatorCom->RegisterEventListener("EndHurt", [&](const string& eventName) {
		//	ChangeState(new StateIdle(TEXT("Idle")));
		m_pAnimatorCom->SetBool("Hurted", false);
		});

	return S_OK;
}

void CBaseCharacter::Priority_Update(_float fTimeDelta)
{


}

void CBaseCharacter::Update(_float fTimeDelta)
{

	HandleInput();
	m_fTotalTime += fTimeDelta;
	m_pInputBuffer->Update(m_fTotalTime);
	UpdateState(fTimeDelta);

	if (m_bAirborne)
	{
		UpdateAirborne(fTimeDelta);
	}



	// 3) 애니메이션 업데이트
	m_pAnimatorCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);

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
	//if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
	//	return E_FAIL;

	Bind_Shaders();


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

		if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
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

const _wstring& CBaseCharacter::GetCurrentStateName()
{
	return m_pState->GetStateName();
}

void CBaseCharacter::HandleInput()
{
	if (m_pGameInstance->IsKeyPressed('J')) m_pInputBuffer->AddCommand({ ECommand::LightAttack, m_fTotalTime });
	if (m_pGameInstance->IsKeyPressed('K')) m_pInputBuffer->AddCommand({ ECommand::Jump, m_fTotalTime });
	if (m_pGameInstance->IsKeyDown('L')) m_pInputBuffer->AddCommand({ ECommand::Dash, m_fTotalTime });
	if (m_pGameInstance->IsKeyDown('O'))
	{
		m_pInputBuffer->AddCommand({ ECommand::Guard, m_fTotalTime });
		if (m_pGameInstance->IsKeyPressed('I'))
			m_pInputBuffer->AddCommand({ ECommand::Skill2, m_fTotalTime });
	}
	if (m_pGameInstance->IsKeyPressed('I')) m_pInputBuffer->AddCommand({ ECommand::Skill0, m_fTotalTime });
}

void CBaseCharacter::UpdateState(_float fTimeDelta)
{
	InputData input;
	FillInput(input);
	if (m_pState)
	{
		m_pState->Update(this, input, fTimeDelta);
	}
}

void CBaseCharacter::FillInput(InputData& outInput)
{
	if (m_pAnimatorCom->CheckBool("Hurted"))
		return; // 피격 중이면 입력 무시

	outInput = InputData();  // 기본값
	auto gi = CGameInstance::Get_Instance();
	XMVECTOR dir = XMVectorZero();
	if (gi->IsKeyDown(VK_UP))    dir += XMVectorSet(0, 0, 1, 0);
	if (gi->IsKeyDown(VK_DOWN))  dir += XMVectorSet(0, 0, -1, 0);
	if (gi->IsKeyDown(VK_LEFT))  dir += XMVectorSet(-1, 0, 0, 0);
	if (gi->IsKeyDown(VK_RIGHT)) dir += XMVectorSet(1, 0, 0, 0);
	if (!XMVector3Equal(dir, XMVectorZero()))
		outInput.moveDir = XMVector3Normalize(dir);
	auto commands = m_pInputBuffer->GetCommands();
	// 2) 4→3→2→1 순서로 else if 검사
	if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack, ECommand::LightAttack,
		  ECommand::LightAttack, ECommand::LightAttack },
		2.5f))
	{
		if (CGameInstance::Get_Instance()->IsKeyDown(VK_UP))
		{
			outInput.doAttack3Up = true;
		}
		else if (CGameInstance::Get_Instance()->IsKeyDown(VK_DOWN))
		{
			outInput.doAttack3Down = true;
		}

		outInput.doAttack4 = true;
		outInput.doAttack3 = true;
		outInput.doAttack2 = true;
		outInput.doAttack = true;

	}
	// 3타: LightAttack×3 + 위/아래 판정 (시간 제한 1초)
	else if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack, ECommand::LightAttack,
		  ECommand::LightAttack },
		1.f))
	{
		outInput.doAttack3 = true;
		outInput.doAttack2 = true;
		outInput.doAttack = true;
	}
	else if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack, ECommand::LightAttack },
		0.5f))
	{
		outInput.doAttack2 = true;
		outInput.doAttack = true;
	}
	else if (m_pInputBuffer->CheckCombo(commands,
		{ ECommand::LightAttack },
		0.2f))
	{
		outInput.doAttack = true;
	}

	for (const auto& cmd : commands)
	{
		switch (cmd.type)
		{
		case ECommand::Jump:
			outInput.doJump = true;
			break;
		case ECommand::Dash:
			outInput.doStep = true; // 또는 doDash 로 따로 관리
			break;
		case ECommand::Guard:
			outInput.doGuard = true;
			break;
		case ECommand::Skill2:
			outInput.doSkill2 = true;
			break;
			break;
		case ECommand::Skill0:

			_bool bMoving = !XMVector3Equal(outInput.moveDir, XMVectorZero());
			if (bMoving)
			{
				outInput.doSkill1 = true;
			}
			else
			{
				outInput.doSkill0 = true;
			}
			break;
		}
	}
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
	if (other->GetType() == ColliderType::HITBOX)
	{
		CBaseCharacter* pAttacker = static_cast<CBaseCharacter*>(other->GetOwner()->GetParent()); // 무기인 경우 (사실상 다 히트박스의 부모 기준일거임)
		if (pAttacker == nullptr)
		{
			pAttacker = static_cast<CBaseCharacter*>(other->GetOwner());
			return;
		}
		if (this == pAttacker)
		{
			return; // 자기 자신과 충돌은 무시
		}

		// 나와 공격자 사이의 방향 계산 (XZ 평면)
		XMVECTOR myPos = GetTransform()->Get_State(STATE::POSITION);
		XMVECTOR attackerPos = pAttacker->GetTransform()->Get_State(STATE::POSITION);

		// Y 성분 무시하고 XZ 기준으로 방향 계산
		myPos = XMVectorSetY(myPos, 0.f);
		attackerPos = XMVectorSetY(attackerPos, 0.f);
		XMVECTOR diff = myPos - attackerPos; // 공격자 위치에서 내 위치로 향하는 벡터
		// 길이가 0에 가까우면 기본 뒤로 방향(=attacker가 바라보는 정반대) 사용
		_float length{};
		XMStoreFloat(&length, XMVector3LengthSq(diff));
		XMVECTOR backDir = length > 0.0001f
			? XMVector3Normalize(diff)
			: XMVectorNegate(pAttacker->GetTransform()->Get_State(STATE::LOOK));

		// 뒤로 밀려나는 거리
		const _float knockbackDistance = 2.f;
		XMVECTOR offset = XMVectorScale(backDir, knockbackDistance);

		// 4) 현재 위치에 offset을 더해 새로운 위치로 설정
		XMVECTOR newPos = XMVectorAdd(myPos, offset);
		// Y 성분은 원래대로 유지
		_float origY = GetTransform()->Get_State(STATE::POSITION).m128_f32[1];
		newPos = XMVectorSetY(newPos, origY);
		if (m_pNavigationCom)
		{
			if(m_pNavigationCom->isMove(newPos))
				GetTransform()->Set_State(STATE::POSITION, newPos);
		}
		

		if (m_eState != CSTATE::HURT && m_eState != CSTATE::GUARD)
		{
			ChangeState(new StateHurt());
			TakeDamage(2.f); // 피해량 조정 가능
		}

		_float distanceZ = XMVectorGetZ(newPos - attackerPos);


	}
	m_bFirstCollision = true; // 첫 충돌 처리 완료
}


void CBaseCharacter::OnCollisionStay(CCollider* other, _float fTimeDelta)
{
	m_bFirstCollision = false; // 첫 충돌 이후에는 계속 충돌 상태로 유지
}


void CBaseCharacter::OnCollisionExit(CCollider* other)
{
	m_bFirstCollision = false; // 충돌 종료 시 첫 충돌 상태 해제
}

void CBaseCharacter::LaunchAirborne(_float fJumpForce)
{
	m_bAirborne = true;
	m_Velocity.y = fJumpForce; // 점프 힘 추가
	m_bIsJumping = true; // 점프 상태로 설정
	ChangeState(new StateHurtAir());
}

void CBaseCharacter::UpdateAirborne(_float fTimeDelta)
{
	_vector vVel = XMLoadFloat3(&m_Velocity);
	_vector gravity = XMLoadFloat3(&GRAVITY) * fTimeDelta * 6.5f;
	vVel += gravity;
	XMStoreFloat3(&m_Velocity, vVel);
	_vector pos = m_pTransformCom->Get_State(STATE::POSITION);
	pos += vVel * fTimeDelta;
	m_pTransformCom->Set_State(STATE::POSITION, pos);

	if (m_pTransformCom->Get_State(STATE::POSITION).m128_f32[1] < 0.f) // 바닥에 닿으면
	{
		m_bAirborne = false; // 공중 상태 해제
		m_Velocity.y = 0.f; // 속도 초기화
		m_bIsJumping = false; // 점프 상태 해제
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), 0.f)); // 바닥에 고정
	}
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
	//	TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatorCom), m_pModelCom)))
	//	return E_FAIL;


	/* For.Com_AnimController*/
	if (FAILED(__super::Add_Component(ToIndex(LEVEL::STATIC), TEXT("Prototype_Component_Animator"),
		TEXT("Com_Animator"), reinterpret_cast<CComponent**>(&m_pAnimatorCom), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBaseCharacter::Bind_Shaders()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
	//	return E_FAIL;


	//m_pShaderCom->Bind_RawValue("g_fToonThreshold", &fToonThreshold, sizeof(float));
	//m_pShaderCom->Bind_RawValue("g_vShadowColor", &vShadowColor, sizeof(float4));

	//// 툰 쉐이딩 패스로 렌더링
	//m_pShader->Begin(2); // ToonShading 패스
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
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
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

	Safe_Release(m_pAnimatorCom);
	Safe_Release(m_pColliderCom);
	Safe_Delete(m_pState);
	Safe_Release(m_pInputBuffer);
	Safe_Release(m_pNavigationCom);
}


