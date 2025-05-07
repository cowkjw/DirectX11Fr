#pragma once
#include "Base.h"
#include "UICanvas.h"

BEGIN_NAMESPACE(Engine)
class CUIManager final : public CBase
{
private:
	CUIManager();
	virtual ~CUIManager() = default;

public:
	void Update_UI(_float fTimeDelta);
private:
	void Priority_Update(_float fTimeDelta) ;
	void Update(_float fTimeDelta) ;
	void Late_Update(_float fTimeDelta) ;
	void Render() ;

public:
	void AddCanvasUI(CUICanvas* pUI);
	void RemoveCanvasUI(const _wstring& canvasName);
	void RemoveUI(const _wstring& canvasName, const _wstring& uiName);
	void ClearCanvas();
	CUIObject* GetUI(const _wstring& canvasName,const _wstring& uiName);


public:
	HRESULT Initialize();

private:
	unordered_map<_wstring, CUICanvas*> m_mapCanvasUI;
public:
	static CUIManager* Create();
	virtual void Free() override;

};
END_NAMESPACE