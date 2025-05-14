#pragma once
#include "Engine_Defines.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL IInspector 
{
public:
    virtual ~IInspector() = default;
    virtual bool TreeNode(const char* label) = 0;
    virtual void TreePop() = 0;
    virtual bool Checkbox(const char* label, bool* v) = 0;
    virtual bool DragFloat(const char* label, float* v, float speed) = 0;
    virtual bool DragFloat3(const char* label, float v[3], float speed) = 0;

    virtual bool InputFloat(const char* label, float* v) = 0;
    virtual bool InputFloat3(const char* label, float v[3]) = 0;
};

END_NAMESPACE

