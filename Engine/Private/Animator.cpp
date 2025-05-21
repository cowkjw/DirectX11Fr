#include "Animator.h"
#include "Model.h"
#include "Bone.h"

CAnimator::CAnimator()
{
}

HRESULT CAnimator::Initialize(CModel* pModel, const vector<class CBone*>& Bones)
{
	m_pModel = pModel;
	m_Bones = Bones;
	return S_OK;
}

void CAnimator::Update(_float fDeltaTime)
{
	if (!m_Blend.active)
	{
		// 블렌드 중이 아니면 그냥 현재 애니메이션만 업데이트
		if (m_pCurrentAnim == nullptr)
			return;
		m_pCurrentAnim->Update_Bones(fDeltaTime, m_Bones, m_Blend.isLoop);
	}
	else
	{
		UpdateBlend(fDeltaTime);
	}
}

void CAnimator::Play(_uint iAnimIndex, _bool isLoop)
{

}

void CAnimator::CrossFade(_uint iAnimIndex, _float blendDuration, _bool isLoop)
{
}

void CAnimator::StartTransition(CAnimation* from, CAnimation* to, _float duration)
{
	m_Blend.active = true;
	m_Blend.srcAnim = from;
	m_Blend.dstAnim = to;
	m_Blend.elapsed = 0.f;
	m_Blend.duration = duration;
	m_Blend.isLoop = to->Get_isLoop();

	// 애니메이션 트랙 초기화
//	m_Blend.srcAnim->ResetTrack();
	m_Blend.dstAnim->ResetTrack();
}

void CAnimator::UpdateBlend(_float fDeltaTime)
{
	m_Blend.elapsed += fDeltaTime;
	_float t = min(m_Blend.elapsed / m_Blend.duration, 1.f);

	//  두 애니메이션을 각자 업데이트 (루프 모드 유지)
	m_Blend.srcAnim->Update_Bones(fDeltaTime, m_Bones, m_Blend.srcAnim->Get_isLoop());
	m_Blend.dstAnim->Update_Bones(fDeltaTime, m_Bones, m_Blend.dstAnim->Get_isLoop());
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		_matrix srcM = m_Blend.srcAnim->GetBoneMatrix(static_cast<_uint>(i));
		_matrix dstM = m_Blend.dstAnim->GetBoneMatrix(static_cast<_uint>(i));

		_vector sS, sR, sT, dS, dR, dT;
		XMMatrixDecompose(&sS, &sR, &sT, srcM);
		XMMatrixDecompose(&dS, &dR, &dT, dstM);

		_vector bS = XMVectorLerp(sS, dS, t);
		_vector bR = XMQuaternionSlerp(sR, dR, t);
		_vector bT = XMVectorLerp(sT, dT, t);

		_matrix M = XMMatrixScalingFromVector(bS)
			* XMMatrixRotationQuaternion(bR)
			* XMMatrixTranslationFromVector(bT);

		m_Bones[i]->Set_TransformationMatrix(M);
	}

	// 블렌드 완료 시
	if (t >= 1.f)
	{
		m_Blend.active = false;
		m_pCurrentAnim = m_Blend.dstAnim;
		m_Blend.srcAnim->ResetTrack();
	}
}

CAnimator* CAnimator::Create(CModel* pModel, const vector<class CBone*>& Bones)
{
	CAnimator* pInstance = new CAnimator();
	if (FAILED(pInstance->Initialize(pModel, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CAnimator* CAnimator::Clone(CModel* pModel, const vector<class CBone*>& Bones)
{
	CAnimator* pInstance = new CAnimator(*this);
	if (FAILED(pInstance->Initialize(pModel, Bones)))
	{
		MSG_BOX("Failed to Cloned : CAnimator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimator::Free()
{
	__super::Free();

}
