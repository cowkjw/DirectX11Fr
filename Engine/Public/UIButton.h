#pragma once
#include "UIObject.h"   

BEGIN_NAMESPACE(Engine)
class CUIImage;
class ENGINE_DLL CUIButton : public CUIObject
{
private:
	CUIButton(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIButton(const CUIButton& Prototype);
	virtual ~CUIButton() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_OnClick(function<void()> OnClick) { m_OnClick = OnClick; }
private:
	void CheckMouseOver();

private:
	function<void()> m_OnClick;
	_bool m_bHovered = false;
	_bool m_bPressed = false;
	CUIImage* m_pButtonImage{ nullptr };
	CUIImage* m_pButtonImageHover{ nullptr };

public:
	static CUIButton* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

