#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLevel_Enmu final : public CLevel
{
private:
	CLevel_Enmu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Enmu() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Layer_Background();
	HRESULT Ready_Layer_Characters();
	HRESULT Ready_Lights();

public:
	static CLevel_Enmu* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE