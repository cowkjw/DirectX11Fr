#pragma once

#include "Client_Defines.h"
#include "Base.h"

BEGIN_NAMESPACE(Engine)
class CGameInstance;
END_NAMESPACE

BEGIN_NAMESPACE(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	CGameInstance*				m_pGameInstance = { };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
public:
	HRESULT Start_Level(LEVEL eStartLevel);
	HRESULT Ready_Prototype_Component();
public:
	static CMainApp* Create();
	virtual void Free() override;

};

END_NAMESPACE