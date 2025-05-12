#pragma once
#include "Client_Defines.h"
#include "Engine_Defines.h"
BEGIN_NAMESPACE(Engine)
class CTransform;
END_NAMESPACE

BEGIN_NAMESPACE(Client)


class CGizmo final
{
public:

    enum class Operation { TRANSLATE, ROTATE, SCALE };

    static void Manipulate(CTransform* pTransform, Operation op, _bool isOrtho, const _float* snap = nullptr);
};


END_NAMESPACE