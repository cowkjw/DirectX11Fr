#pragma once
#include "Camera.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Engine)
class CGameObject;
END_NAMESPACE

BEGIN_NAMESPACE(Client)
class CCutSceneCamera : public CCamera
{
public:
	struct CamKeyFrame
	{
		_float3 vPosition;
		_float3 vOffset; // 카메라 오프셋
	};
private:
	CCutSceneCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCutSceneCamera(const CCutSceneCamera& Prototype);
	virtual ~CCutSceneCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg = nullptr) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

	void ResetCamera();
	void ClearKeyFrames() { m_KeyFrames.clear(); m_iCurrentFrame = 0; m_fElapsedTime = 0.f; m_bIsPlaying = false; }

	void SetKeyFrames(const vector<CamKeyFrame>& keyFrames, _float fDuration);
	void SetCurrentFrame(_uint iFrameIndex);
	void SetLoop(_bool bLoop) { m_bLoop = bLoop; }
	void SetTargetObject(CGameObject* pObj) { m_pTargetObject = pObj; }
	void SetDuration(_float fDuration) { m_fDuration = fDuration; }
	void SetMainpulate(_bool bMainpulate) { m_bMainpulate = bMainpulate; }
	void SetPlay(_bool bPlay) { m_bIsPlaying = bPlay; }

	_bool IsPlaying() const { return m_bIsPlaying; }
	_float GetDuration() const { return m_fDuration; }
	_float GetElapsedTime() const { return m_fElapsedTime; }
	_uint GetCurrentFrame() const { return m_iCurrentFrame; }
	CGameObject* GetTargetObject() const { return m_pTargetObject; }
	_bool IsMainpulate() const { return m_bMainpulate; }
	_bool IsLoop() const { return m_bLoop; }
	auto& GetKeyFrames() { return m_KeyFrames; }
	virtual json Serialize() override;


private:
	void UpdateCameraTransform(_float fTimeDelata);

private:
	_bool m_bLoop = false;
	_bool m_bIsPlaying = false;
	_float m_fDuration = 1.f; 
	_float m_fElapsedTime = 0.f;
	_float m_fMouseSensor = { };
	_uint m_iCurrentFrame = 0; 
	vector<CamKeyFrame> m_KeyFrames; // 카메라 키프레임들
	CGameObject* m_pTargetObject = nullptr;
	_bool m_bMainpulate = false; // 카메라 조작 중인지 여부

public:
	static CCutSceneCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};
END_NAMESPACE

