#pragma once
#include "UIObject.h"   

BEGIN_NAMESPACE(Engine)
class CUIImage;
class ENGINE_DLL CUIButton : public CUIObject
{
public:
	typedef struct tagUIButtonDesc : public CUIObject::UIOBJECT_DESC
	{
		_wstring strButtonImageHoverKey;
	} UIButtonDesc;
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

	virtual json Serialize() override;

public:
	void Set_OnClick(function<void()> OnClick) { m_OnClick = OnClick; }
	void Set_OnHover(function<void()> OnHover) { m_OnHover = OnHover; }
	void SetButtonImage(CUIImage* pButtonImage, CUIImage* pButtonImageHover = nullptr);
	CUIImage* GetButtonImage() const { return m_pButtonImage; }
	CUIImage* GetButtonImageHover() const { return m_pButtonImageHover; }
	_bool IsHovered() const { return m_bHovered; }
private:
	void CheckMouseOver();

private:
	function<void()> m_OnClick;
	function<void()> m_OnHover;
	_bool m_bHovered = false;
	_bool m_bPressed = false;
	CUIImage* m_pButtonImage{ nullptr };
	CUIImage* m_pButtonImageHover{ nullptr };
	_wstring m_strButtonImageHoverKey;
	_wstring m_strButtonImageKey;

public:
	static CUIButton* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

