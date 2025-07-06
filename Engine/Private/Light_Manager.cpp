#include "Light_Manager.h"
#include "Light.h"

CLight_Manager::CLight_Manager()
{

}

const LIGHT_DESC* CLight_Manager::Get_Light(_uint iIndex)
{
	if (iIndex >= m_Lights.size())
		return nullptr;
	return m_Lights[iIndex]->Get_LightDesc();
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
	CLight* pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Render_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (m_Lights.empty())
		return S_OK;
	if (nullptr == pShader || nullptr == pVIBuffer)
		return E_FAIL;
	for (auto& pLight : m_Lights)
	{
		if (nullptr == pLight)
			continue;
		if (FAILED(pLight->Render(pShader, pVIBuffer)))
			return E_FAIL;
	}

	return S_OK;
}

void CLight_Manager::Clear_Lights()
{
	for (auto& pLight : m_Lights)
	{
		Safe_Release(pLight);
	}
	m_Lights.clear();
}


CLight_Manager* CLight_Manager::Create()
{
	return new CLight_Manager;
}

void CLight_Manager::Free()
{
	__super::Free();

	for (auto& pLight : m_Lights)	
		Safe_Release(pLight);

	m_Lights.clear();
}
