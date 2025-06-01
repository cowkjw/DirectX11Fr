#pragma once
#include "Client_Enum.h"

enum class ECommand { LightAttack, HeavyAttack, Move,Up,Down, Jump, Dash, Guard, Skill0, Skill1, Skill2, Ultemate };
struct Command
{
    ECommand   type;
    _float      timestamp;  // 게임 시작 후 경과 시간(초)
    Command(ECommand t, _float ts) : type(t), timestamp(ts) {}
};