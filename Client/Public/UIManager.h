#pragma once
#include "Base.h"
#include "Client_Defines.h"	
BEGIN(Engine)
class CUIObject;
END
class CUIManager final : public CBase
{
private:
	CUIManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CUIManager() = default;

public:
	void Update(_float fTimeDelta);
	HRESULT Render();

	void AddUI(CUIObject* pUI);
	void RemoveUI(CUIObject* pUI);
	void ClearUI();
	CUIObject* GetUI(const _wstring& uiTag);

public:
	HRESULT Initialize();


private:
	map<_wstring, vector<CUIObject*>> m_mapUI;	
public:
	static CUIManager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};

