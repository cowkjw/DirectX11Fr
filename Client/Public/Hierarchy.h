#pragma once
#include "Panel.h"

BEGIN_NAMESPACE(Engine)
class CGameObject;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CHierarchy :  public CPanel
{
private:
	CHierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CHierarchy() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	void DrawHierarchy();      // ¾À ±×·¡ÇÁ
	void DrawChildHierarchy(CGameObject* parent);
	void EraseFromVector(CGameObject* pObj);
	void RemoveAndDelete(CGameObject* obj);

public:
	static CHierarchy* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};
END_NAMESPACE

