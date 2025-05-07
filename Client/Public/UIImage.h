#pragma once
#include "UIObject.h"   
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END_NAMESPACE

BEGIN_NAMESPACE(Client)

class CUIImage : public CUIObject
{

protected:
	CUIImage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	CUIImage(const CUIImage& Prototype);
	virtual ~CUIImage()  = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	HWND m_hWnd{ nullptr };

private:
	virtual HRESULT Ready_Components();
public:
	static CUIImage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject* Clone(void* pArg) override;
	virtual void Free() override;
};
	END_NAMESPACE

