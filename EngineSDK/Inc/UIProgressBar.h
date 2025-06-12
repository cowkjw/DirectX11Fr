#pragma once
#include "UIObject.h"   

BEGIN_NAMESPACE(Engine)
class CUIImage;
class ENGINE_DLL CUIProgressBar : public CUIObject
{
public:
	typedef struct tBarDesc : public UIOBJECT_DESC
	{
		_wstring strFillTextureKey; // 채워지는 이미지 키
		_wstring strDamageTextureKey; // 피해 이미지 키
		_wstring strBarShaderKey; // 바 렌더링 셰이더 키
	}BAR_DESC;
private:
	CUIProgressBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIProgressBar(const CUIProgressBar& Prototype);
	virtual ~CUIProgressBar() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void SetBarImage(CUIImage* pBackgroundImage, CUIImage* pFillImage= nullptr, CUIImage* pDamageImage = nullptr);
	virtual json Serialize() override;


public:
	void SetMaxValue(_float max) {
		m_fMaxValue = max;
		m_fCurValue = max;
		m_fPrevValue = max;
	}

	// 데미지 이벤트가 들어올 때 호출
	void ApplyDamage(_float amount);

	// 방향 플립: false = 왼→오, true = 오→왼
	void SetReverse(_bool bReverse) { m_bReverse = bReverse; }
private:
	// 최대값, 현재값, 피해 이펙트용 이전값
	_float  m_fMaxValue{ 100.f };
	_float  m_fCurValue{100.f};
	_float  m_fPrevValue{ 1.f };

	// 피해 바가 사라지기 시작하는 딜레이와 줄어드는 속도
	_float  m_fDelayTime{ 0.03f };
	_float  m_fDelayTimer{ 0.f };
	_float  m_fRecoverSpeed{ 0.25f }; // 초당 비율
	CUIImage* m_pBGImage = nullptr;
	CUIImage* m_pFillImage = nullptr; // 빨간 피해 표시
	CUIImage* m_pDamageImage = nullptr; // 녹색(혹은 호흡색)

	_bool m_bReverse{ false }; // true면 오른쪽에서 왼쪽으로 채워짐

public:
	static CUIProgressBar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
END_NAMESPACE

