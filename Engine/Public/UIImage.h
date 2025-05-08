#pragma once
#include "UIObject.h"   

BEGIN_NAMESPACE(Engine)
class CShader;
class CTexture;

class ENGINE_DLL CUIImage : public CUIObject
{
protected:
	CUIImage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIImage(const CUIImage& Prototype);
	virtual ~CUIImage()  = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
protected:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	_wstring m_strTextureKey{};
	_wstring m_strShaderKey{};

protected:
	HRESULT Ready_Components();
public:
	static CUIImage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

