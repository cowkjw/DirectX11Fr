#pragma once
#include "UIImage.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Client)
class CUIAnimImage : public CUIImage
{
private:
	CUIAnimImage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIAnimImage(const CUIAnimImage& rhs);
	virtual ~CUIAnimImage() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void OnEnable() override;
	virtual void OnDisable() override;
private:
	_float m_fTimeElapsed = 0.f; // 애니메이션 시간 경과
	_int m_iCurrentFrame = 0; // 현재 프레임 인덱스
	_int m_iNumFrames = 0; // 총 프레임 수
	_float m_fFrameDuration = 0.1f; // 프레임당 지속 시간

public:
	static CUIAnimImage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

