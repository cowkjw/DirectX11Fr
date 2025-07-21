#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void SetUpUI();
	void MoveCloud(_float fTimeDelta);
	void UpdateShojiOpen(_float fTimeDelta);

private:
	static _float4  m_vInitShojiOrigin[2];

	LEVEL				m_eNextLevelID = { LEVEL::END };
	_float   m_fShojiDuration = 1.5f;
	_float   m_fShojiTime = 0.f;
	_float   m_fCloudTime = 0.f;               // 클라우드 애니메이션 누적 시간

	_vector  m_vCloudOrigin[4];
	_vector  m_vShojiOrigin[2];
	class CLoader*		m_pLoader = { nullptr };

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END_NAMESPACE