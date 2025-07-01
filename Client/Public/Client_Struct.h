#pragma once
#include "Engine_Defines.h"

struct InputData
{
    XMVECTOR moveDir;   // 이동 방향 (정규화된 벡터 또는 (0,0,0))
    bool     doAttack;  // 기본 평타(1타) 공격
	bool	 doAttack2; // 추가 공격(2타) - 예: 연속 공격, 콤보 등
	bool    doAttack3; // 추가 공격(3타) - 예: 연속 공격, 콤보 등
	bool     doAttack4; // 추가 공격(4타) - 예: 연속 공격, 콤보 등
	bool     doAttack3Down; // 추가 공격(3타) - 예: 연속 공격, 콤보 등
    bool     doAttack3Up;
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
		, doAttack3Down(false)
		, doAttack3Up(false)

    {
    }
};

typedef struct tagParticleUV
{
	_float2 vUVOffset = { 0.f, 0.f }; // UV 오프셋
	_float2 vUVScale = { 1.f, 1.f }; // UV 스케일
	_float fCurrentTime = 0.f; // 현재 시간 (애니메이션 시간)
	_float fFrameTime = 0.f; // 프레임 시간 (애니메이션 속도)
	_int iTotalFrames = 0; // 총 프레임 수
	_int iCurrentFrame = 0; // 현재 프레임
	_int iCols = 1; // 열 수
	_int iRows = 1; // 행 수

} PARTICLE_UV;