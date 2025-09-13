#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CGameInstance;
class CGameObject;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CPanel : public CBase
{
protected:
	CPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CPanel() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

public:
	static CPanel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

