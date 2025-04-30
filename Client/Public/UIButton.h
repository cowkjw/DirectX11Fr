#pragma once
#include "UIObject.h"   
#include "Client_Defines.h"

BEGIN(Client)

typedef struct tButtonDesc
{
	function<void()> OnClick;  
}BUTTON_DESC;

class CUIButton final : public CUIObject
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

private:
	function<void()> m_OnClick;
	_bool m_bHovered = false;
	_bool m_bPressed = false;
public:
	static CUIButton* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float4& vColor, function<void()> onClick);
	CUIObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END

