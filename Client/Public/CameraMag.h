#pragma once
#include "Base.h"
#include "Camera.h"
#include "Client_Defines.h"

BEGIN_NAMESPACE(Client)
class CCameraMag : public CBase
{
	DECLARE_SINGLETON(CCameraMag)
private:
	CCameraMag() {}
	virtual ~CCameraMag() = default;
public:
	void RegisterCamera(const _wstring& cameraName, CCamera* pCamera);
	void RegisterCutSceneProperty(const _wstring& key, const json& j);
	void UnregisterCamera(const _wstring& cameraName);

	void ActiveCamera(const _wstring& cameraName);
	void ClearCameras();

	CCamera* GetActiveCamera();
	CCamera* GetCamera(const _wstring& cameraName);

	void SetCutSceneProperty(const _wstring& key);

private:
	unordered_map<_wstring, CCamera*> m_Cameras; // 카메라 목록
	_wstring m_ActiveCameraName; // 현재 활성화된 카메라 이름
	unordered_map<_wstring, json> m_CutSceneProperties; // 키 이름 ,json정보

private:
	virtual void Free() override;
};
END_NAMESPACE
