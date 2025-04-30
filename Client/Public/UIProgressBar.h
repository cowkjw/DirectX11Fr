#pragma once
#include "UIObject.h"   
#include "Observer.h"
#include "Texture.h"
#include "Client_Defines.h"

BEGIN(Engine)
class CTexture;
END
BEGIN(Client)

typedef struct CUIBarDesc
{
}BAR_DESC;

class CUIProgressBar final : public CUIObject, public IObserver<_float>
{
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
	vector<CTexture*> m_vecTextures;
public:
	static CUIProgressBar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject* Clone(void* pArg) override;
	virtual void Free() override;

	// IObserver을(를) 통해 상속됨
	void Notify(_float const& msg) override;
};
END

