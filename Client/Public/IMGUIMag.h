#pragma once
#include "Client_Defines.h"
#include "Base.h"

BEGIN_NAMESPACE(Engine)
class CGameInstance;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CEditorManager;
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
	void Docking();
private:
	CGameInstance* m_pGameInstance = {nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CEditorManager* m_pEditorMag{ nullptr };

public:
	static CIMGUIMag* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE

