#pragma once
#include "Animator.h"
#include "EnmuMeat.h"
#include "EnmuParts.h"
#include "GameInstance.h"
#include "BaseCharacter.h"
#include "AnimController.h"
#include "WarningZoneDecal.h"

BEGIN_NAMESPACE(Client)
class BossState
{
public:
    BossState() = default;
    BossState(const _wstring& stateName)
        : m_stateName(stateName) {
    }
    virtual ~BossState() = default;
    virtual void Enter(CEnmuMeat* pChar) = 0;
    virtual void Update(CEnmuMeat* pChar,_float fTimeDelta) = 0;
    virtual void Exit(CEnmuMeat* pChar) = 0;
    const _wstring& GetStateName() const { return m_stateName; }
protected:
    _wstring m_stateName; // 상태 이름
};
END_NAMESPACE
