#pragma once

#include "Engine_Defines.h"

BEGIN_NAMESPACE(Engine)

class ENGINE_DLL CBase
{
protected:
	CBase();
	virtual ~CBase() = default;

public:
	unsigned int AddRef();
	unsigned int Release();

protected:
	unsigned int		m_iRefCnt = { };

public:
	virtual void Free();

};
END_NAMESPACE


