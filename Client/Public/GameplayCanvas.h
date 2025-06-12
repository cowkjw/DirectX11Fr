#pragma once
#include "UICanvas.h"
class CGameplayCanvas : public CUICanvas
{
private:
	CGameplayCanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameplayCanvas(const CGameplayCanvas& Prototype);
	virtual ~CGameplayCanvas() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

private:
	HRESULT Ready_ChildUI();

public:
	static CGameplayCanvas* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

