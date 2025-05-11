#pragma once
#include "Client_Defines.h"
#include "Base.h"

BEGIN_NAMESPACE(Engine)
class CGameInstance;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CUIController;
class CIMGUIMag final : public CBase
{
private:
	CIMGUIMag(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CIMGUIMag() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	CGameInstance* m_pGameInstance = {nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CUIController* m_pUIController{ nullptr };

public:
	static CIMGUIMag* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE

