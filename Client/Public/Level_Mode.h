#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN_NAMESPACE(Client)

class CLevel_Mode final : public CLevel
{
private:
	CLevel_Mode(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Mode() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Layer_BackGround(const _wstring strLayerTag);

	void Ready_UI_Setup();

	void StartGamePlay();
private:
	void MoveCloud(_float fTimeDelta);

private:
	_float   m_fCloudTime = 0.f;               // 클라우드 애니메이션 누적 시간
	_vector  m_vCloudOrigin[4];

public:
	static CLevel_Mode* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END_NAMESPACE