#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLevel_BattleSelect final : public CLevel
{
private:
	CLevel_BattleSelect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_BattleSelect() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void SetUpUI();
	void StartAkaza();
	void StartEnmu();

public:
	static CLevel_BattleSelect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE