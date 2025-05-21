#pragma once
#include "Base.h"
#include <Animation.h>

BEGIN_NAMESPACE(Engine)
class ENGINE_DLL CAnimator final  :  public CBase
{
public:
    struct BlendState
    {
        CAnimation* srcAnim = nullptr;   // 이전 애니메이션
        CAnimation* dstAnim = nullptr;   // 다음 애니메이션
        _float        elapsed = 0.f;       // 경과 시간
        _float        duration = 0.2f;      // 블렌드 총 시간
        _bool         isLoop = true;
        _bool         active = false;
    };
private:
	CAnimator();
	virtual ~CAnimator() = default;

public:
	HRESULT Initialize(class CModel* pModel, const vector<class CBone*>& Bones);
    void Update(_float fDeltaTime);

    // 즉시 전환: 현재 애니메이션을 멈추고 바로 새 애니메이션 재생
    void Play(_uint iAnimIndex, _bool isLoop = true);

    // 크로스페이드 전환: duration(초) 동안 페이드
    void CrossFade(_uint iAnimIndex, _float blendDuration, _bool isLoop = true);

    void StartTransition(CAnimation* from, CAnimation* to, _float duration = 0.2f);

	void Set_CurrentAnim(CAnimation* pAnim) { m_pCurrentAnim = pAnim; }

	void Set_BlendState(BlendState blend) { m_Blend = blend; }
	void Set_Model(class CModel* pModel) { m_pModel = pModel; }
	void SetBlendDuration(_float duration) { m_Blend.duration = duration; }
private:
    void UpdateBlend(_float fDeltaTime);

private:
    class CModel* m_pModel{ nullptr };          // 본과 메시 데이터 참조
    CAnimation* m_pCurrentAnim = nullptr;
    BlendState  m_Blend{};
	vector<class CBone*> m_Bones; // 전체 본의 개수

public:
	static CAnimator* Create(class CModel* pModel, const vector<class CBone*>& Bones);
	CAnimator* Clone(class CModel* pModel, const vector<class CBone*>& Bones);
    virtual void Free();
};
END_NAMESPACE
