#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLogo final : public CLevel
{
private:
	CLogo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLogo() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Layer_BackGround(const _wstring strLayerTag);



public:
	static CLogo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE