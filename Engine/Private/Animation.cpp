#include "Animation.h"
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
}



HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones)
{
	/* 이 애니메이션이 컨트롤해야하는 뼈의 갯수 */
	m_iNumChannels = pAIAnimation->mNumChannels;

	m_fTickPerSecond = pAIAnimation->mTicksPerSecond;
	m_fDuration = pAIAnimation->mDuration;


	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	/* 각 뼈의 정보를 새엇ㅇ한다. */
	for (size_t i = 0; i < m_iNumChannels; i++)
	{		
		CChannel* pChannel = CChannel::Create(pAIAnimation->mChannels[i], Bones);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

    return S_OK;
}

HRESULT CAnimation::InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	// 1) Duration
	ifs.read((char*)&m_fDuration, sizeof(m_fDuration));
	// 2) TickPerSecond
	ifs.read((char*)&m_fTickPerSecond, sizeof(m_fTickPerSecond));
	// 3) Channels
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
	// 4) CurrentKeyFrameIndices
	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	return S_OK;
}

_bool CAnimation::Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop)
{
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

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix(m_CurrentKeyFrameIndices[i], m_fCurrentTrackPosition, Bones);
	}
	return false;
}

void CAnimation::ExportBinary(ofstream& ofs)
{
	// 1) Duration
	ofs.write((char*)&m_fDuration, sizeof(m_fDuration));
	// 2) TickPerSecond
	ofs.write((char*)&m_fTickPerSecond, sizeof(m_fTickPerSecond));
	// 3) Channels
	uint32_t channelCount = (uint32_t)m_Channels.size();
	ofs.write((char*)&channelCount, sizeof(channelCount));
	for (auto& channel : m_Channels)
		channel->ExportBinary(ofs);
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

CAnimation* CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	__super::Free();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();


}
