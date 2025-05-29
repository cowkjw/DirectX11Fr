#include "Channel.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones)
{
	m_iNumKeyFrames = max(pAIChannel->mNumPositionKeys, pAIChannel->mNumScalingKeys);
	m_iNumKeyFrames = max(pAIChannel->mNumRotationKeys, m_iNumKeyFrames);

	_float3		vScale;
	_float4		vRotation;
	_float3		vTranslation;

	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME		KeyFrame{};

		if(i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTrackPosition = pAIChannel->mScalingKeys[i].mTime;
		}

		if (i < pAIChannel->mNumRotationKeys)
		{
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;			
			KeyFrame.fTrackPosition = pAIChannel->mRotationKeys[i].mTime;
		}

		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTrackPosition = pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;

		m_KeyFrames.push_back(KeyFrame);
	}

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
		{
			if (true == pBone->Compare_Name(pAIChannel->mNodeName.data))
				return true;

			++m_iBoneIndex;

			return false;
		});


	return S_OK;
}

HRESULT CChannel::InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	// 1) Name
	ifs.read((char*)&m_szName, sizeof(m_szName));
	// 2) KeyFrame Count
	ifs.read((char*)&m_iNumKeyFrames, sizeof(m_iNumKeyFrames));
	// 3) KeyFrames
	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME		KeyFrame{};
		ifs.read((char*)&KeyFrame, sizeof(KEYFRAME));
		m_KeyFrames.push_back(KeyFrame);
	}
	// 4) BoneIndex
	ifs.read((char*)&m_iBoneIndex, sizeof(m_iBoneIndex));
	return S_OK;
}

void CChannel::Update_TransformationMatrix(_uint& currentKeyFrameIndex, _float fCurrentTrackPosition, const vector<class CBone*>& Bones)
{
	if (0.0f == fCurrentTrackPosition)
		currentKeyFrameIndex = 0;

	_matrix			TransformationMatrix{};

	KEYFRAME		LastKeyFrame = m_KeyFrames.back();

	_vector			vScale, vRotation, vPosition;

	if (fCurrentTrackPosition >= LastKeyFrame.fTrackPosition)
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vPosition  = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
	}
	else
	{
		if(fCurrentTrackPosition >= m_KeyFrames[currentKeyFrameIndex + 1].fTrackPosition)
			++currentKeyFrameIndex;

		_float			fRatio = (fCurrentTrackPosition - m_KeyFrames[currentKeyFrameIndex].fTrackPosition) /
			(m_KeyFrames[currentKeyFrameIndex + 1].fTrackPosition - m_KeyFrames[currentKeyFrameIndex].fTrackPosition);

		_vector			vSourScale, vDestScale;
		_vector			vSourRotation, vDestRotation;
		_vector			vSourTranslation, vDestTranslation;

		vSourScale = XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex].vScale);
		vDestScale = XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex + 1].vScale);

		vSourRotation = XMLoadFloat4(&m_KeyFrames[currentKeyFrameIndex].vRotation);
		vDestRotation = XMLoadFloat4(&m_KeyFrames[currentKeyFrameIndex + 1].vRotation);

		vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex].vTranslation), 1.f);
		vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex + 1].vTranslation), 1.f);
		
		vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);
		vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);
		vPosition = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);
	}

	// TransformationMatrix = XMMatrixScaling() * XMMatrixRotationQuaternion() * XMMatrixTranslation();
	TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vPosition);

	if (m_iBoneIndex >= Bones.size())
	{
		//MSG_BOX("CChannel::Update_TransformationMatrix - m_iBoneIndex is out of range");
		return;
	}
	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
	XMStoreFloat4x4(&m_LocalTransformationMatrix, TransformationMatrix);
}

void CChannel::ExportBinary(ofstream& ofs)
{
	// 1) Name
	ofs.write((char*)&m_szName, sizeof(m_szName));
	// 2) KeyFrame Count
	ofs.write((char*)&m_iNumKeyFrames, sizeof(m_iNumKeyFrames));
	// 3) KeyFrames
	for (auto& keyFrame : m_KeyFrames)
	{
		ofs.write((char*)&keyFrame, sizeof(KEYFRAME));
	}
	// 4) BoneIndex
	ofs.write((char*)&m_iBoneIndex, sizeof(m_iBoneIndex));
}

CChannel* CChannel::Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CChannel* CChannel::CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	CChannel* pInstance = new CChannel();
	if (FAILED(pInstance->InitializeByBinary(ifs, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CChannel::Free()
{
	__super::Free();

	m_KeyFrames.clear();
}
