#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLevel_EnmuBoss final : public CLevel
{
private:
	CLevel_EnmuBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_EnmuBoss() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Layer_Background();
	HRESULT Ready_Layer_Characters();
	HRESULT Ready_Lights();
private:
	void UpdateGameFlow(_float fTimeDelta);

private:
	_bool m_bStartGame = false;
	_float m_fStartImageElapsedTime = 0.f;
	const _float m_fStartImageTime = 2.5f;
	_bool m_bEndGame = false;
	_bool m_bIsGameOver = false;
	_float m_fStopImageElapsedTime = 0.f;
	const _float m_fStopImageTime = 2.f;
	_float m_fFinalImageElapsedTime = 0.f;
	const _float m_fFinalImageTime = 2.5f;

	class CBaseCharacter* m_pTanjiro = nullptr;
	class CEnmuMeat* m_pEnmu = nullptr; // ƒÏ¡÷∑Œ
public:
	static CLevel_EnmuBoss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE