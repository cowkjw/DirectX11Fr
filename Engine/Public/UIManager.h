#pragma once
#include "Base.h"
#include "UICanvas.h"

BEGIN_NAMESPACE(Engine)
class CUIManager final : public CBase
{
private:
	CUIManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CUIManager() = default;

public:
	HRESULT Initialize();
	void Update_UI(_float fTimeDelta);
private:
	void Priority_Update(_float fTimeDelta) ;
	void Update(_float fTimeDelta) ;
	void Late_Update(_float fTimeDelta) ;


public:
	void AddCanvasUI(CUICanvas* pUI);
	void RemoveCanvasUI(const _wstring& canvasName);
	void RemoveUI(const _wstring& canvasName, const _wstring& uiName);
	void ClearCanvas();

	CUIObject* GetUI(const _wstring& canvasName,const _wstring& uiName);
	CGameObject* CreateUI(CUIObject::UIOBJECT_DESC* pDesc, UI_TYPE eUIType);
	CGameObject* GetCanvasUI(const _wstring& canvasName);


private:
	unordered_map<_wstring, CUICanvas*> m_mapCanvasUI;
	ID3D11Device* m_pDevice = { nullptr };	
	ID3D11DeviceContext* m_pContext = { nullptr };
public:
	static CUIManager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};
END_NAMESPACE