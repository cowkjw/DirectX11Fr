#include "CameraMag.h"
#include "GameInstance.h"
#include "CutSceneCamera.h"

IMPLEMENT_SINGLETON(CCameraMag);

void CCameraMag::RegisterCamera(const _wstring& cameraName, CCamera* pCamera)
{
	if (pCamera == nullptr)
		return;
	auto iter = m_Cameras.find(cameraName);
	if (iter != m_Cameras.end())
	{
		Safe_Release(iter->second);
	}
	m_Cameras[cameraName] = pCamera;
	Safe_AddRef(pCamera);
}

void CCameraMag::RegisterCutSceneProperty(const _wstring& key, const json& j)
{
	if (key.empty())
		return;
	auto iter = m_CutSceneProperties.find(key);
	if (iter != m_CutSceneProperties.end())
	{
		iter->second = j; // 이미 존재하는 경우 업데이트
	}
	else
	{
		m_CutSceneProperties[key] = j; // 새로운 키 추가
	}
}

void CCameraMag::UnregisterCamera(const _wstring& cameraName)
{
	auto iter = m_Cameras.find(cameraName);
	if (iter != m_Cameras.end())
	{
		Safe_Release(iter->second);
		m_Cameras.erase(iter);
	}
}

void CCameraMag::ClearCameras()
{
	for (auto& pair : m_Cameras)
	{
		Safe_Release(pair.second);
	}
	m_Cameras.clear();
	m_ActiveCameraName.clear();
}

void CCameraMag::ActiveCamera(const _wstring& cameraName)
{
	auto iter = m_Cameras.find(cameraName);
	if (iter != m_Cameras.end())
	{
		CCamera* pCamera = iter->second;
		// 기존 카메라 비활성화
		if (!m_ActiveCameraName.empty())
		{
			CCamera* pActiveCamera = GetCamera(m_ActiveCameraName);
			if (pActiveCamera)
			{
				pActiveCamera->SetActive(false);
			}
		}
		m_ActiveCameraName = cameraName;
		pCamera = GetCamera(m_ActiveCameraName);
		if (pCamera)
		{
			if (pCamera->IsActive())
				return; 
			pCamera->SetActive(true);
		}
	}
	else
	{
		m_ActiveCameraName.clear();
	}
}

CCamera* CCameraMag::GetActiveCamera()
{
	auto iter = m_Cameras.find(m_ActiveCameraName);
	if (iter != m_Cameras.end())
	{
		return iter->second;
	}
	return nullptr;
}

CCamera* CCameraMag::GetCamera(const _wstring& cameraName)
{
	auto iter = m_Cameras.find(cameraName);
	if (iter != m_Cameras.end())
	{
		return iter->second;
	}
	return nullptr;
}

void CCameraMag::SetCutSceneProperty(const _wstring& key)
{
	auto iter = m_CutSceneProperties.find(key);

	if (iter != m_CutSceneProperties.end())
	{
		if (m_ActiveCameraName.empty())
			return;
		CCamera* pCamera = GetCamera(m_ActiveCameraName);
		if (auto pCutSceneCam = dynamic_cast<CCutSceneCamera*>(pCamera))
		{
			pCutSceneCam->Deserialize(iter->second);
		}
	}
}

void CCameraMag::Free()
{
	__super::Free();
	for (auto& Pair : m_Cameras)
	{
		Safe_Release(Pair.second);
	}
	m_Cameras.clear();
	m_CutSceneProperties.clear();
}
