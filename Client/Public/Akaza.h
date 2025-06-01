#pragma once
#include "BaseCharacter.h"

BEGIN_NAMESPACE(Client)
class CAkaza :public CBaseCharacter
{
public:
	enum class COM_STATE
	{
		IDLE,
		MOVE,
		ATTACK,
		GUARD,
		STEP,
		SKILL,
		JUMP,
		APPRACH,
		HURT,
		DIE,
	};
private:
	CAkaza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAkaza(const CAkaza& Prototype);
	virtual ~CAkaza() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SetComState(COM_STATE eComState) { m_eComState = eComState; }
	COM_STATE GetComState() const { return m_eComState; }
private:
	virtual HRESULT Ready_Components() override;
	virtual void Ready_Animation() override;

	void PredictPlayerState();

private:
	
	COM_STATE m_eComState = COM_STATE::IDLE;
	_float m_fFollowTime = 5.f; // 플레이어를 따라가는 시간

public:
	static CAkaza* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

