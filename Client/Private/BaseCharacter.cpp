#include "BaseCharacter.h"
#include "DashSmokeEffect.h"
#include "StateBoundHurt.h"
#include "StateHurtBlow.h"
#include "EffectManager.h"
#include "StateHurtDown.h"
#include "StateHurtAir.h"
#include "GameInstance.h"
#include "InputBuffer.h"
#include "GuardEffect.h"
#include "Environment.h"
#include "StateDeath.h"
#include "Navigation.h"
#include "StateHurt.h"
#include "StateIdle.h"
#include "Animation.h"
#include "Weapon.h"	
#include <ThirdPersonCamera.h>



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
	, m_pRangeColliderCom{ Prototype.m_pRangeColliderCom }
	, m_bGameStarted{ Prototype.m_bGameStarted }
	, m_pDashSmokeEffect{ Prototype.m_pDashSmokeEffect }
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


	m_pGuardEffect = static_cast<CGuardEffect*>(m_pGameInstance->Add_GameObject(ToIndex(LEVEL::STATIC), TEXT("Prototype_Effect_Guard"), ToIndex(LEVEL::GAMEPLAY), TEXT("Guard")));

	if (m_pGuardEffect)
	{
		m_pGuardEffect->SetParent(this);
		m_pGuardEffect->SetActive(false);
	}


	m_pAnimatorCom->RegisterEventListener("EndHurt", [&](const string& eventName) {
		//	ChangeState(new StateIdle(TEXT("Idle")));
		m_pAnimatorCom->SetBool("Hurted", false);
		});

	m_pAnimatorCom->RegisterEventListener("HitedAir", [&](const string& eventName) {

		if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pTarget))
		{
			if (m_bCanBlowAttack && pChar->GetState() != CSTATE::GUARD)
				pChar->Blow(this, 35.f);
		}
		});

	m_pAnimatorCom->RegisterEventListener("EndAttack", [&](const string& eventName) {
		if (m_pAnimatorCom)
		{
			m_pAnimatorCom->SetBool("Attacking", false);
		}
		});
	m_pAnimatorCom->RegisterEventListener("Death", [&](const string& eventName) {
		if (m_pAnimatorCom)
		{
			m_pAnimatorCom->SetBool("Death", true);
		}
		});

	m_pAnimatorCom->RegisterEventListener("AttackSound", [&](const string& eventName)
		{
			if (m_strName == TEXT("Kyojuro"))
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Kyojuro/Attack");
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashSwing");

			}
			else if (m_strName == TEXT("Akaza"))
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Akaza/Attack");
			}
			else
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Tanjiro/Attack");
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashSwing");
			}
			
		});

	m_pAnimatorCom->RegisterEventListener("FinalAttackSound", [&](const string& eventName)
		{
			if (m_strName == TEXT("Kyojuro"))
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Kyojuro/Attack4");
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashSwing");
			}
			else if (m_strName == TEXT("Akaza"))
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Akaza/Attack4");
			}
			else
			{
				CSoundMag::Get_Instance()->PlayEffect("event:/Tanjiro/Attack4");
				CSoundMag::Get_Instance()->PlayEffect("event:/Common/SlashSwing");
			}

		});
	return S_OK;
}

void CBaseCharacter::Priority_Update(_float fTimeDelta)
{
	if (m_fCurrentHP <= 0.f && m_eState != CSTATE::DIE&&m_pState->GetStateName()!=L"Death")
	{
		StartHitStop(3.f);
		if (m_pTarget)
		{
			if (auto pChar = dynamic_cast<CBaseCharacter*>(m_pTarget))
			{
				pChar->StartHitStop(3.f);
			}
		}
		ChangeState(new StateDeath());
		return;
	}

}

void CBaseCharacter::Update(_float fTimeDelta)
{
	if (m_eState != CSTATE::DIE && m_bGameStarted)
	{
		/*HandleInput();
		m_fTotalTime += fTimeDelta;
		m_pInputBuffer->Update(m_fTotalTime);
		if(!m_bAirborne&&!m_bIsJumping)
		UpdateState(fTimeDelta);

		if (m_bAirborne)
		{
			UpdateAirborne(fTimeDelta);
		}*/

		if (m_fHitStopTime > 0.f)
		{
			m_fHitStopTime -= fTimeDelta;
			fTimeDelta *= 0.2f; // HitStop 동안 시간 느리게 흐름
		}

		if (!m_bAirborne && !m_bIsBound)
			HandleInput();

		m_fTotalTime += fTimeDelta;
		m_pInputBuffer->Update(m_fTotalTime);




	}
	if (!m_bAirborne && !m_bIsJumping)
		UpdateState(fTimeDelta);
	else
		UpdateAirborne(fTimeDelta);
	// 애니메이션 업데이트
	m_pAnimatorCom->GetAnimController()->Update(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);

}

void CBaseCharacter::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT CBaseCharacter::Render()
{
	Bind_Shaders();

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

HRESULT CBaseCharacter::Render_Shadow()
{

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_WorldMatrix())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Light_ViewMatrix())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Light_ProjMatrix())))
		return E_FAIL;
	_float fCamFar = m_pGameInstance->Get_CameraFar();

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCameraFar", &fCamFar, sizeof _float)))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(4)))
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
	if (m_fCurrentHP <= 0.f)
	{
		// 자꾸 상태 변환돼서 그냥 return
		if (pState && pState->GetStateName() != L"Death")
		{
			pState = new StateDeath();
			Safe_Delete(pState);
			return;
		}
	}
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

		PushBack(pAttacker); // 공격자에게서 뒤로 밀려남

	}
	m_bFirstCollision = true; // 첫 충돌 처리 완료
}

void CBaseCharacter::OnCollisionEnter(CCollider* other, const XMFLOAT3& hitPos)
{
	if (other->GetType() == ColliderType::HITBOX)
	{
		//BodyHitParticle
		if (m_pAnimatorCom->CheckBool("Hurted"))
			return; // 이미 피격 중이면 무시
		//CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("SlashHitParticle"), hitPos);
	//	CEffectManager::Get_Instance()->SpawnParticleEffect(TEXT("HitCrossParticle"), hitPos);
	}
}


void CBaseCharacter::OnCollisionStay(CCollider* other, _float fTimeDelta)
{
	m_bFirstCollision = false; // 첫 충돌 이후에는 계속 충돌 상태로 유지
}


void CBaseCharacter::OnCollisionExit(CCollider* other)
{
	m_bFirstCollision = false; // 충돌 종료 시 첫 충돌 상태 해제
}

void CBaseCharacter::LaunchAirborne(_float fJumpForce, _bool bIsBound)
{
	m_Velocity.x = 0.f;
	m_Velocity.z = 0.f;
	/*if (m_bIsBound)
		return;*/
	if (m_bAirborne)
		m_Velocity.y = min(m_Velocity.y + fJumpForce, 38.f);
	else
		m_Velocity.y = fJumpForce;
	m_bAirborne = true;
	m_bIsJumping = true; // 점프 상태로 설정

	if (!bIsBound)
	{
		ChangeState(new StateHurtAir());
	}
	else
	{
		m_bIsBound = bIsBound; // 바운드 여부 설정
		ChangeState(new StateBoundHurt());
	}
}

void CBaseCharacter::LaunchAirborneFall(_float fJumpForce)
{
	m_Velocity.x = 0.f;
	m_Velocity.z = 0.f;
	if (m_bAirborne)
		m_Velocity.y = std::min(m_Velocity.y + fJumpForce, 100.f);
	else
		m_Velocity.y = fJumpForce;
	m_bAirborne = true;
	m_bIsJumping = true; // 점프 상태로 설정
	ChangeState(new StateHurtBlow());
}

void CBaseCharacter::Blow(CGameObject* pAttacker, _float fBlowForce)
{
	if (m_bAirborne || m_bIsBound)
		return; // 이미 공중에 있거나 바운드 상태면 무시
	if (m_eState != CSTATE::GUARD)
	{
		// 공격자의 위치에서 나를 향하는 방향 벡터 계산
		_vector attackerPos = pAttacker->GetTransform()->Get_State(STATE::POSITION);
		_vector myPos = m_pTransformCom->Get_State(STATE::POSITION);
		attackerPos = XMVectorSetY(attackerPos, XMVectorGetY(myPos)); // xz로 안하니까 y 차이가 너무 크면 이상하게 처리됨
		_float   distSq = XMVectorGetX(XMVector3LengthSq(myPos - attackerPos));
		_vector dirXZ{};
		if (distSq <= 600.f) // 너무 가까우면 그냥 뒤로 밀림
		{
			dirXZ = XMVector3Normalize(pAttacker->GetTransform()->Get_State(STATE::LOOK));
		}
		else
		{
			dirXZ = XMVector3Normalize(myPos - attackerPos);
		}
		XMStoreFloat3(&m_Velocity, XMVectorScale(dirXZ, fBlowForce));
		m_Velocity.y = fBlowForce + 6.f; 
		m_bAirborne = true; // 공중 상태로 전환
		m_bIsJumping = true; // 점프 상태로 설정

		ChangeState(new StateHurtBlow());
	}
	m_bCanBlowAttack = false; // 블로우 공격 후에는 다시 사용할 수 없도록 설정
}

void CBaseCharacter::UpdateAirborne(_float fTimeDelta)
{

	_vector vVel = XMLoadFloat3(&m_Velocity);
	_vector gravity = m_bIsBound ? XMLoadFloat3(&GRAVITY) * fTimeDelta * 10.f : XMLoadFloat3(&GRAVITY) * fTimeDelta * 9.6f;
	vVel += gravity;
	XMStoreFloat3(&m_Velocity, vVel);

	_vector pos = m_pTransformCom->Get_State(STATE::POSITION);
	pos += vVel * fTimeDelta;
	if (m_pNavigationCom)
	{
		if (m_pNavigationCom->isMove(pos))
		{

			m_pTransformCom->Set_State(STATE::POSITION, pos);
		}
		else
		{
			_vector tmpPos = m_pTransformCom->Get_State(STATE::POSITION);
			tmpPos = XMVectorSetY(tmpPos, XMVectorGetY(pos));
			m_pTransformCom->Set_State(STATE::POSITION, tmpPos);
		}
	}

	// 바닥 충돌 처리
	if (pos.m128_f32[1] <= m_fGoroundHeight)
	{
		if (m_bIsBound)
		{
			m_Velocity.y = 40.f;  // 한 번만 튀기고
			m_bIsBound = false;  // 이후엔 낙하로 전환
		}
		else
		{
			m_Velocity.y = 0.f;
			m_bAirborne = false;
			m_bIsJumping = false;
			m_bFalling = false;
		}

		// 바닥에 고정
		_vector curPos = m_pTransformCom->Get_State(STATE::POSITION);
		curPos = XMVectorSetY(curPos, m_fGoroundHeight);
		if (m_pNavigationCom)
		{
			if (m_pNavigationCom->isMove(curPos))
				m_pTransformCom->Set_State(STATE::POSITION, curPos);
		}

		CSoundMag::Get_Instance()->PlayEffect("event:/Common/BodyFall");
	}
}

void CBaseCharacter::UpdateBounding(_float fTimeDelta)
{
	if (m_bIsBound)
	{
		_vector pos = m_pTransformCom->Get_State(STATE::POSITION);

		if (pos.m128_f32[1] < m_fGoroundHeight) // 바닥에 닿으면
		{
			m_bIsBound = false; // 바운드 상태 해제
			m_Velocity.y = 0.f; // 속도 초기화
			m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(pos, 0.f)); // 바닥에 고정
			if (m_eState != CSTATE::HURT && m_eState != CSTATE::GUARD)
			{
				ChangeState(new StateIdle(TEXT("Idle")));
			}
		}
		else
		{
			pos += XMLoadFloat3(&m_Velocity) * fTimeDelta;
			m_pTransformCom->Set_State(STATE::POSITION, pos);
		}
	}
}

void CBaseCharacter::PushBack(CBaseCharacter* pAttacker)
{	// 나와 공격자 사이의 방향 계산 (XZ 평면)
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
		if (m_pNavigationCom->isMove(newPos))
			GetTransform()->Set_State(STATE::POSITION, newPos);
	}

	m_pTransformCom->LookAtXZ(pAttacker->GetTransform()->Get_State(STATE::POSITION));

	//if (m_eState != CSTATE::HURT && m_eState != CSTATE::GUARD)
	//{
	//	//ChangeState(new StateHurt());
	//	//TakeDamage(2.f); // 피해량 조정 가능
	//}
}

void CBaseCharacter::HurtDown()
{
	if (m_bAirborne || m_bIsBound)
		return; // 이미 공중에 있거나 바운드 상태면 무시
	ChangeState(new StateHurtDown());
}

void CBaseCharacter::TakeDamage(_float fDamage)
{
	if (m_fCurrentHP <= 0.f)
		return;

	if (auto pCamera = dynamic_cast<CThirdPersonCamera*>(CGameInstance::Get_Instance()->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("ThirdPersonCamera"))))
	{
	
		auto pKyo =CGameInstance::Get_Instance()
			->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Kyojuro"));
		auto pAka = CGameInstance::Get_Instance()
			->Find_GameObjectByName(ToIndex(LEVEL::GAMEPLAY), TEXT("Akaza"));
		if (!pKyo || !pAka)
			return;

		static _float currentSide = 1.f;    // 방향은 -1~1 사이
		const _float fThreshold = 0.3f;      
		const _float fBaseSide = 50.f;
		const _float fOffsetUp = 15.f;

		const _vector up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		_vector kyoPos = pKyo->GetTransform()->Get_State(STATE::POSITION);
		_vector kyoRight = pKyo->GetTransform()->Get_State(STATE::RIGHT);
		kyoRight = XMVectorSetY(kyoRight, 0.f);  
		kyoRight = XMVector3Normalize(kyoRight);  
		// 적의 상대 위치
		_vector akaPos = pAka->GetTransform()->Get_State(STATE::POSITION);
		akaPos = XMVectorSetY(akaPos, 0.f);
		_vector toAka = XMVector3Normalize(akaPos - kyoPos);

		// 좌우 판단
		_float dotSide = XMVectorGetX(XMVector3Dot(toAka, kyoRight));

		// 현재 오른쪽인지 왼쪽인지 판단
		// -1이면 왼쪽, 1이면 오른쪽
		if (currentSide > 0.f && dotSide < -fThreshold)
			currentSide = -1.f;
		else if (currentSide < 0.f && dotSide > fThreshold)
			currentSide = 1.f;

		// 오프셋 계산
		_vector offset = kyoRight * (currentSide * fBaseSide) + up * fOffsetUp;
		pCamera->TriggerShake(0.13f, 0.23f);
		pCamera->OnHit(pKyo, pAka, offset, 0.4f);
	}
	m_pGameInstance->SetHitStop(true, 0.25f);
	if (m_eState == CSTATE::GUARD)
	{
		SpawnGurad();
	}
	m_fCurrentHP -= fDamage;
	if (m_fCurrentHP <= 0.f)
	{
		m_fCurrentHP = 0.f;
	}
}

void CBaseCharacter::StartHitStop(_float duration)
{
	m_pGameInstance->SetHitStop(true, 0.25f);
	//m_fHitStopTime = duration;
}

void CBaseCharacter::Ready_Animation()
{

}

void CBaseCharacter::SpawnGurad()
{
	if (m_pGuardEffect)
	{
		if (m_pGuardEffect->IsActive())
		{
			m_pGuardEffect->SetActive(false); // 이미 활성화된 경우 비활성화
		}
	

		// 내 앞에 바라보면서 
		_vector vForward = XMVector3Normalize(
			m_pTransformCom->Get_State(STATE::LOOK)
		);
		_vector guardPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector offsetForward = XMVectorScale(vForward, 3.f);
		_vector offsetUp = XMVectorSet(0.f, 10.f, 0.f, 0.f);
		guardPos = XMVectorAdd(guardPos, offsetForward);
		guardPos = XMVectorAdd(guardPos, offsetUp);

		m_pGuardEffect->GetTransform()->RotateToDirection(vForward);
		m_pGuardEffect->GetTransform()->Set_State(STATE::POSITION, guardPos);

		m_pGuardEffect->SetActive(true);
	}
}

HRESULT CBaseCharacter::Ready_Components()
{
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
	_float fCamFar = m_pGameInstance->Get_CameraFar();
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCameraFar", &fCamFar, sizeof(_float))))
		return E_FAIL;

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
	Safe_Release(m_pGuardEffect);
	Safe_Release(m_pColliderCom);
	Safe_Delete(m_pState);
	Safe_Release(m_pInputBuffer);
	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pDashSmokeEffect);
	Safe_Release(m_pRangeColliderCom);
}


