#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)
class CJsonLoader;
class CLevel_GamePlay final : public CLevel
{
private:
	CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GamePlay() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Layer_Character(const _wstring& strLayerTag);
	HRESULT Ready_Lights();
	HRESULT Ready_Effects(CJsonLoader& jsonLoader);
	HRESULT Ready_Camera();
private:
	void UpdateGameFlow(_float fTimeDelta);
	void CheckCharacterDeath(class CBaseCharacter* pChar);
	void UpdateStartState(_float fTimeDelta);
	void UpdateEndState(_float fTimeDelta);
private:
	_bool m_bStartGame = false;
	_bool m_bEndGame = false;
	_bool m_bIsGameOver = false;
	_float m_fStartImageElapsedTime = 0.f;
	_float m_fStopImageElapsedTime = 0.f;
	_float m_fFinalImageElapsedTime = 0.f;
	const _float m_fStopImageTime = 2.f;
	const _float m_fStartImageTime = 2.5f;
	const _float m_fFinalImageTime = 2.5f;
	class CBaseCharacter* m_pAkaza = nullptr;
	class CBaseCharacter* m_pKyojuro = nullptr; // ƒÏ¡÷∑Œ

public:
	static CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE