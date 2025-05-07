#pragma once
#include "UIObject.h"

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CUICanvas final : public CUIObject
{
private:
	CUICanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUICanvas(const CUICanvas& Prototype);
	virtual ~CUICanvas() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void AddChildUI(CUIObject* pChildUI);
	void RemoveChildUI(CUIObject* pChildUI);
	
private:
	void SortChildUI();

private:
	vector<CUIObject*> m_vecChildUIObjects;
public:
	static CUICanvas* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

