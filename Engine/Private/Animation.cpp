#include "Animation.h"
#include "Bone.h"
#include "Channel.h"

CAnimation::CAnimation()
	: m_fDuration{ 0.f }
	, m_fTickPerSecond{ 0.f }
	, m_fCurrentTrackPosition{ 0.f }
	, m_iNumChannels{ 0 }
	, m_CurrentKeyFrameIndices{}
	, m_Channels{}
{
}

CAnimation::CAnimation(const CAnimation& Prototype)
	: m_fDuration{ Prototype.m_fDuration }
	, m_fTickPerSecond{ Prototype.m_fTickPerSecond }
	, m_fCurrentTrackPosition{ Prototype.m_fCurrentTrackPosition }
	, m_CurrentKeyFrameIndices{ Prototype.m_CurrentKeyFrameIndices }
	, m_iNumChannels{ Prototype.m_iNumChannels }
	, m_Channels{ Prototype.m_Channels }
	
{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);
	strcpy_s(m_Name, Prototype.m_Name);
}



HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones)
{
	strcpy_s(m_Name, pAIAnimation->mName.data);

	/* 이 애니메이션이 컨트롤해야하는 뼈의 개수 */
	m_iNumChannels = pAIAnimation->mNumChannels;

	m_fTickPerSecond = static_cast<_float>(pAIAnimation->mTicksPerSecond);
	m_fDuration = static_cast<_float>(pAIAnimation->mDuration);


	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	for (size_t i = 0; i < m_iNumChannels; i++)
	{		
		CChannel* pChannel = CChannel::Create(pAIAnimation->mChannels[i], Bones);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

	m_Bones = Bones;
    return S_OK;
}

HRESULT CAnimation::InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	// Animation Name
	uint32_t nameLength;
	ifs.read((char*)&nameLength, sizeof(nameLength));
	if (nameLength > MAX_PATH)
		return E_FAIL;
	ifs.read(m_Name, nameLength);
	m_Name[(_char)nameLength] = '\0'; // Null-terminate the string

	//  Duration
	ifs.read((char*)&m_fDuration, sizeof(m_fDuration));
	// TickPerSecond
	ifs.read((char*)&m_fTickPerSecond, sizeof(m_fTickPerSecond));
	// Channels
	uint32_t channelCount;
	ifs.read((char*)&channelCount, sizeof(channelCount));
	for (uint32_t i = 0; i < channelCount; ++i)
	{
		CChannel* pChannel = CChannel::CreateByBinary(ifs, Bones);
		if (nullptr == pChannel)
			return E_FAIL;
		m_Channels.push_back(pChannel);
	}
	m_iNumChannels = (uint32_t)m_Channels.size();
	// CurrentKeyFrameIndices
	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	return S_OK;
}

_bool CAnimation::Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop, vector<string>* outEvents)
{
	m_isLoop = isLoop;
	_float prevPos = m_fCurrentTrackPosition;
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;


	if (m_fCurrentTrackPosition >= m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		if (false == isLoop)
		{
			m_fCurrentTrackPosition = m_fDuration;
			return true;
		}
	}


	// **이벤트 검출** (prevPos < ev.time <= currentPos)
	if (outEvents)
	{
		for (auto& ev : m_events) 
		{
			if (ev.fTime > prevPos && ev.fTime <= m_fCurrentTrackPosition) 
			{
				outEvents->push_back(ev.name);
			}
		}
	}

	// 기존 본 업데이트 로직
	for (_uint i = 0; i < m_iNumChannels; ++i) 
	{
		m_Channels[i]->Update_TransformationMatrix(
			m_CurrentKeyFrameIndices[i],
			m_fCurrentTrackPosition,
			Bones
		);
	}

	// 논루프 애니 종료 반환
	if (!isLoop && m_fCurrentTrackPosition >= m_fDuration)
		return true;
	return false;
}

void CAnimation::ExportBinary(ofstream& ofs)
{
	//Animation Name
	uint32_t nameLength = (uint32_t)strlen(m_Name);
	ofs.write((char*)&nameLength, sizeof(nameLength));
	ofs.write(m_Name, nameLength);
	
	//Duration
	ofs.write((char*)&m_fDuration, sizeof(m_fDuration));
	// TickPerSecond
	ofs.write((char*)&m_fTickPerSecond, sizeof(m_fTickPerSecond));
	//  Channels
	uint32_t channelCount = (uint32_t)m_Channels.size();
	ofs.write((char*)&channelCount, sizeof(channelCount));
	for (auto& channel : m_Channels)
		channel->ExportBinary(ofs);
}

_matrix CAnimation::GetBoneMatrix(_uint iIndex)
{
	for (auto& pChannel : m_Channels)
	{
		if (pChannel->GetBoneIndex() == iIndex)
		{
			return XMLoadFloat4x4(&pChannel->GetLocalMatrix());
		}
	}
	//채널이 없으면 해당 뼈의 로컬 바인드 포즈를 반환
	return XMLoadFloat4x4(&m_Bones[iIndex]->Get_LocalBindPose());
}

CAnimation* CAnimation::Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(pAIAnimation, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation();
	if (FAILED(pInstance->InitializeByBinary(ifs, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CAnimation* CAnimation::Clone(const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation(*this);
	pInstance->m_Bones = Bones;
	return pInstance;
}

void CAnimation::Free()
{
	__super::Free();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
}
