#pragma once
#include "Engine_Defines.h"

struct InputData
{
    XMVECTOR moveDir;   // 이동 방향 (정규화된 벡터 또는 (0,0,0))
    bool     doAttack;  // 기본 평타(1타) 공격
	bool	 doAttack2; // 추가 공격(2타) - 예: 연속 공격, 콤보 등
	bool    doAttack3; // 추가 공격(3타) - 예: 연속 공격, 콤보 등
	bool     doAttack4; // 추가 공격(4타) - 예: 연속 공격, 콤보 등
    bool     doJump;    // 점프
    bool     doGuard;   // 가드(방어)
    bool     doSkill0;  // 스킬0
    bool     doSkill1;  // 스킬1
	bool     doSkill2;  // 스킬2 
    bool     doStep;    // 스텝(회피)

    // 생성자에서 초기값 세팅
    InputData()
        : moveDir(XMVectorZero())
        , doAttack(false)
        , doJump(false)
        , doGuard(false)
        , doSkill0(false)
        , doSkill1(false)
        , doSkill2(false)
        , doStep(false)
		, doAttack2(false)
		, doAttack3(false)
		, doAttack4(false)

    {
    }
};