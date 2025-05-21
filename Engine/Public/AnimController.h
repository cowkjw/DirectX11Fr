#pragma once
#include "Component.h"
BEGIN_NAMESPACE(Engine)
class CAnimController :  public CComponent
{
public:
	struct AnimState
	{
		string stateName;
		class CAnimation* clip;
	};
private:
	CAnimController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimController(const CAnimController& Prototype);
	virtual ~CAnimController() = default;


public:

	HRESULT Initialize_Prototype();
	HRESULT Initialize(void* pArg);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	HRESULT Render();

private:
	class CAnimator* m_pAnimator = { nullptr };

private:

};
END_NAMESPACE
