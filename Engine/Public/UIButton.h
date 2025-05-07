#pragma once
#include "UIImage.h"   
BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CUIButton : public CUIImage
{
public:
	typedef struct tButtonDesc : public UIOBJECT_DESC
	{
		function<void()> OnClick;
	}BUTTON_DESC;

private:
	CUIButton(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	CUIButton(const CUIButton& Prototype);
	virtual ~CUIButton() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void CheckMouseOver();

private:
	function<void()> m_OnClick;
	_bool m_bHovered = false;
	_bool m_bPressed = false;
	CTexture* m_pHoverTexture{ nullptr };

public:
	static CUIButton* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,const _float4& vColor, function<void()> onClick);
	CUIObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

