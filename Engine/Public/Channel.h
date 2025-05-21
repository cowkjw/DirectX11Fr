#pragma once

#include "Base.h"

/* 특정 애니메이션을 구동하기위해서 필요한, 뼈의 시간당 상태정보*/

BEGIN_NAMESPACE(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	HRESULT InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones);
	void Update_TransformationMatrix(_uint& currentKeyFrameIndex, _float fCurrentTrackPosition, const vector<class CBone*>& Bones);

	void ExportBinary(ofstream& ofs);

private:
	_char				m_szName[MAX_PATH] = {};
	_uint				m_iNumKeyFrames;
	vector<KEYFRAME>	m_KeyFrames;
	_uint				m_iBoneIndex = {};
	//_uint				m_iCurrentKeyFrameIndex = {};
	

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	static CChannel* CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones);
	virtual void Free() override;

};

END_NAMESPACE