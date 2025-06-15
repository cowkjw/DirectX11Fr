#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLevel_Logo final : public CLevel
{
private:
	CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Logo() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Layer_BackGround(const _wstring strLayerTag);

	void Ready_UI_Setup();

	void StartGamePlay();
private:
	_bool m_bIsEndInk = false;
	_float m_fMaxScale = 430.0f; // 최대 스케일
	_float m_fFinalScale = 400.f;

public:
	static CLevel_Logo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE