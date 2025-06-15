#pragma once


namespace Client
{
	enum class LEVEL { STATIC, LOADING, LOGO, GAMEPLAY, ENMU_BOSS,EDITOR,MODE,ENMU, END };
	enum class CSTATE { IDLE, MOVE, JUMP, ATTACK, SKILL, HURT, DIE }; // 캐릭터 기본 상태 더 추가할 수 있음
	enum class EDirection
	{
		Forward,
		Backward,
		Left,
		Left2,
		Right,
		Right2,
		NONE
	};

}


