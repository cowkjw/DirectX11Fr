#pragma once
#include "UIObject.h"   


BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CUIProgressBar : public CUIObject
{
public:
	typedef struct tBarDesc : public UIOBJECT_DESC
	{
	}BAR_DESC;
private:
	CUIProgressBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIProgressBar(const CUIProgressBar& Prototype);
	virtual ~CUIProgressBar() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float      m_fMaxValue{ 1.f };
	_float      m_fCurValue{ 1.f };
	vector<class CTexture*> m_vecTextures;
public:
	static CUIProgressBar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

