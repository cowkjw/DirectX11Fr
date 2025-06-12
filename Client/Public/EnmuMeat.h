#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "BehaviorTree.h"
enum class EnmuState
{
	IDLE, // 기본
	PUNCH, // 펀치 
	FOLLOWPUNCH, // 따라다니고 펀치
	HANDATTACK, // 손바닥 내려칙
	TENTACLEATTACK, // 땅에 박고 촉수
	FREEZEATTACK, // 못움직이게 공격
	ANGRYFREEZEATTACK, // 화난 상태에서 공격
	SWINGATTACK, // 크게 휘두르는 공격
	HURT, // 맞았을 때
	OPENING, // 열고 있을 때
	OPEN, // 열린 상태
	CLOSING, // 닫는 상태
	DIE // 죽음
};

BEGIN_NAMESPACE(Client)
class CEnmuMeat : public CGameObject
{
	friend class BossIdle;
public:
	enum Parts
	{
		LEFTARM,
		RIGHTARM,
		BODY,
		HEAD,
		PARTS_END
	};


private:
	CEnmuMeat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnmuMeat(const CEnmuMeat& Prototype);
	virtual ~CEnmuMeat() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

public:
	void SetState(EnmuState eState);
	void ChangeState(class BossState* pState);
	class CEnmuParts* GetPart(Parts ePart);
	class CBaseCharacter* GetTarget() const { return m_pTarget; }
	void SpawnTentacle(_int iIndex, _vector vPos, _vector vDir);
	void DestroyTentacle(_int iIndex);
	void ActiveTentacle(_int iIndex, _bool bActive);
	void ResetTentacles();

	_float GetDistanceToTarget() const;
	_float GetHp() { return m_fHp; }
	_float Hit(_float fDamage)
	{
		m_fHp -= fDamage;
		if (m_fHp <= 0.f)
		{
			m_fHp = 0.f;
			SetState(EnmuState::DIE);
		}
		return m_fHp;
	}



private:
	HRESULT Ready_Parts();
	HRESULT Ready_BehaviorTree();


private:
	_float m_fHp = 100.f;
	_float m_fMaxHp = 100.f;
	EnmuState m_eState = EnmuState::IDLE; // 현재 상태
	class BossState* m_pBossState{ nullptr };
	class CBaseCharacter* m_pTarget{ nullptr }; // 타겟 캐릭터
	vector<class CEnmuTentacle*> m_vecTentacles; // 팔에 붙은 촉수들

private:
	_float m_CD_Punch;
	_float m_CD_Swing;
	_float m_CD_Hand;
	_float m_CD_Freeze;
	_float m_CD_FollowPunch;
	_float m_CD_Open;
	_float m_CD_Tentacle;
	_uint m_LastPatternIdx; // 마지막으로 사용한 패턴 인덱스


public:
	static CEnmuMeat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

