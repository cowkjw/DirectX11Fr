#pragma once
#include "UICanvas.h"
class CTitleCanvas : public CUICanvas
{
private:
	CTitleCanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTitleCanvas(const CTitleCanvas& Prototype);
	virtual ~CTitleCanvas() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

private:
	HRESULT Ready_ChildUI();

public:
	static CTitleCanvas* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

