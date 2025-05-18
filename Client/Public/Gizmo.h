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
    static void Manipulate(
        CTransform* pTransform,
        Operation op,
        _bool isOrtho,
        const _float snapT[3] = nullptr,   // 이동용 스냅 (unit)
        const _float snapR[3] = nullptr,   // 회전용 스냅 (deg)
        const _float snapS[3] = nullptr    // 스케일용 스냅 (factor)
    );
};


END_NAMESPACE