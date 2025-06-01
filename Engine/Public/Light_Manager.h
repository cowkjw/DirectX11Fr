#pragma once

#include "Base.h"

BEGIN_NAMESPACE(Engine)

class CLight_Manager final : public CBase
{
private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHT_DESC* Get_Light(_uint iIndex);

public:
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);

private:
	vector<class CLight*>				m_Lights;
	

public:
	static CLight_Manager* Create();
	virtual void Free() override;
};
END_NAMESPACE