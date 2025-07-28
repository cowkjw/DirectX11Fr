#pragma once
#include "Engine_Defines.h"

// 직렬화를 위한 인터페이스
BEGIN_NAMESPACE(Engine)
class ENGINE_DLL ISerializable
{
public:
	ISerializable() = default;
	virtual ~ISerializable() = default;
	
	virtual json Serialize() = 0;
	virtual void Deserialize(const json& j) = 0;
};
END_NAMESPACE

