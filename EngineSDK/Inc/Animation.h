#pragma once

#include "Base.h"

BEGIN_NAMESPACE(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	HRESULT InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones);

	_bool Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop);

	void ExportBinary(ofstream& ofs);

	/* 애니메이션의 이름을 반환. */
	const char* Get_Name() { return m_Name; }

	void SetLoop(_bool isLoop) { m_isLoop = isLoop; }
	_bool Get_isLoop() { return m_isLoop; }
	_matrix GetBoneMatrix(_uint iIndex); // 각 애니메이션 채널의 로컬 본 매트릭스 반환하기

	void SetDurtation(_float fDuration) { m_fDuration = fDuration; }
	void SetTickPerSecond(_float fTickPerSecond) { m_fTickPerSecond = fTickPerSecond; }
	void SetCurrentTrackPosition(_float fCurrentTrackPosition) { m_fCurrentTrackPosition = fCurrentTrackPosition; }
	void SetCurrentKeyFrameIndices(_uint iIndex, _uint iKeyFrameIndex) { m_CurrentKeyFrameIndices[iIndex] = iKeyFrameIndex; }

	_uint GetCurrentKeyFrameIndices(_uint iIndex) { return m_CurrentKeyFrameIndices[iIndex]; }
	_float GetDuration() { return m_fDuration; }
	_float GetTickPerSecond() { return m_fTickPerSecond; }
	_float GetCurrentTrackPosition() { return m_fCurrentTrackPosition; }
	_float GetClipLength() const
	{
		return m_fDuration / m_fTickPerSecond;
	}

public:
	void ResetTrack()
	{
		m_fCurrentTrackPosition = 0.f;
		fill(m_CurrentKeyFrameIndices.begin(),
			m_CurrentKeyFrameIndices.end(), 0u);
	}
private:
	/* 전체 재생 거리. */
	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};
	vector<_uint>			m_CurrentKeyFrameIndices;
	/* 이 애니메이션을 표현하기위해서 사용하는 뼈의 갯수 */
	_uint					m_iNumChannels;
	vector<class CChannel*>	m_Channels;
	char m_Name[MAX_PATH] = {};
	_bool m_isLoop = false;
	vector<class CBone*> m_Bones;
public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	static CAnimation* CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones);
	CAnimation* Clone(const vector<class CBone*>& Bones);
	virtual void Free() override;
};

END_NAMESPACE