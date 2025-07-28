#pragma once
#include "Client_Defines.h"
#include "Animator.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "InputBuffer.h"
#include "AnimController.h"

BEGIN_NAMESPACE(Client)
class IState 
{
public:
	IState() = default;
	IState(const _wstring& stateName)
		: m_stateName(stateName) {
	}
    virtual ~IState() = default;
    // 상태 진입 시 
    virtual void Enter(CBaseCharacter* pChar) = 0;
    // 매 프레임
    virtual void Update(CBaseCharacter* pChar, const InputData& input, float fTimeDelta) = 0;
    // 상태 종료 시
    virtual void Exit(CBaseCharacter* pChar) = 0;
	const _wstring& GetStateName() const { return m_stateName; }
protected:
	_wstring m_stateName; // 상태 이름
};
END_NAMESPACE
