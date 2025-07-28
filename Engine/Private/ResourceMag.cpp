#include "ResourceMag.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer.h"
#include "Model.h"
#include <VIBuffer_Rect.h>

CResourceMag::CResourceMag(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CResourceMag::RegisterDynamicShader(const _wstring& key, CShader* pShader)
{
	auto iter = m_dynamicShaders.find(key);
	if (iter == m_dynamicShaders.end())
	{
		m_dynamicShaders.emplace(key, pShader);
		m_DynamicShaderKeyList.push_back(key);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pShader;
	}
}

void CResourceMag::RegisterDynamicTexture(const _wstring& key, CTexture* pTexture)
{
	auto iter = m_dynamicTextures.find(key);
	if (iter == m_dynamicTextures.end())
	{
		m_dynamicTextures.emplace(key, pTexture);
		m_DynamicTextureKeyList.push_back(key);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pTexture;
	}
}

void CResourceMag::RegisterDynamicModel(const _wstring& key, CModel* pModel)
{
	auto iter = m_dynamicModels.find(key);
	if (iter == m_dynamicModels.end())
	{
		m_dynamicModels.emplace(key, pModel);
		m_DynamicModelKeyList.push_back(key);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pModel;
	}

}


void CResourceMag::RegisterStaticShader(const _wstring& key, CShader* pShader)
{
	RegisterResource(key, pShader, m_staticShaders, m_StaticShaderKeyList);
	//auto iter = m_staticShaders.find(key);
	//if (iter == m_staticShaders.end())
	//{
	//	m_staticShaders.emplace(key, pShader);
	//	m_StaticShaderKeyList.push_back(key);
	//}
	//else
	//{
	//	Safe_Release(iter->second);
	//	iter->second = pShader;
	//}
}

void CResourceMag::RegisterStaticTexture(const _wstring& key, CTexture* pTexture)
{
	RegisterResource(key, pTexture, m_staticTextures, m_StaticTextureKeyList);
	//auto iter = m_staticTextures.find(key);
	//if (iter == m_staticTextures.end())
	//{
	//	m_staticTextures.emplace(key, pTexture);
	//	m_StaticTextureKeyList.push_back(key);
	//}
	//else
	//{
	//	Safe_Release(iter->second);
	//	iter->second = pTexture;
	//}
}


void CResourceMag::RegisterStaticModel(const _wstring& key, CModel* pModel)
{
	RegisterResource(key, pModel, m_staticModels, m_StaticModelKeyList);
	//auto iter = m_staticModels.find(key);
	//if (iter == m_staticModels.end())
	//{
	//	m_staticModels.emplace(key, pModel);
	//	m_StaticModelKeyList.push_back(key);
	//}
	//else
	//{
	//	Safe_Release(iter->second);
	//	iter->second = pModel;
	//}
}

CShader* CResourceMag::LoadShader(const _wstring& key, const _wstring& vsPath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	auto iter = m_staticShaders.find(key);
	if (iter != m_staticShaders.end())
	{
		return iter->second;
	}
	CShader* pShader = CShader::Create(m_pDevice, m_pContext, vsPath.c_str(), pElements, iNumElements);
	if (nullptr == pShader)
		return nullptr;
	RegisterStaticShader(key, pShader);
	return pShader;

}

CTexture* CResourceMag::LoadTexture(const _wstring& key, const _wstring& filePath, _uint iNumTextrues)
{
	auto iter = m_staticTextures.find(key);
	if (iter != m_staticTextures.end())
	{
		return iter->second;
	}
	CTexture* pTexture = CTexture::Create(m_pDevice, m_pContext, filePath.c_str(), iNumTextrues);
	if (nullptr == pTexture)
		return nullptr;
	RegisterStaticTexture(key, pTexture);
	return pTexture;
}

CModel* CResourceMag::LoadModel(const _wstring& key, const _wstring& filePath,MODEL eType, _matrix preMatrix)
{
	auto iter = m_staticModels.find(key);
	if (iter != m_staticModels.end())
	{
		return iter->second;
	}
	CModel* pModel = CModel::CreateByBinary(m_pDevice, m_pContext, eType, WStringToString(filePath).c_str(), preMatrix);
	if (nullptr == pModel)
		return nullptr;
	RegisterStaticModel(key, pModel);
	return pModel;
}

CShader* CResourceMag::LoadDynamicShader(const _wstring& key, const _wstring& vsPath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	auto iter = m_dynamicShaders.find(key);
	if (iter != m_dynamicShaders.end())
	{
		return iter->second;
	}
	CShader* pShader = CShader::Create(m_pDevice, m_pContext, vsPath.c_str(), pElements, iNumElements);
	if (nullptr == pShader)
		return nullptr;
	RegisterDynamicShader(key, pShader);
	return pShader;
}

CTexture* CResourceMag::LoadDynamicTexture(const _wstring& key, const _wstring& filePath, _uint iNumTextrues)
{
	auto iter = m_dynamicTextures.find(key);
	if (iter != m_dynamicTextures.end())
	{
		return iter->second;
	}
	CTexture* pTexture = CTexture::Create(m_pDevice, m_pContext, filePath.c_str(),iNumTextrues);
	if (nullptr == pTexture)
		return nullptr;
	RegisterDynamicTexture(key, pTexture);
	return pTexture;
}


CModel* CResourceMag::LoadDynamicModel(const _wstring& key, const _wstring& filePath, MODEL eType, _matrix preMatrix)
{
	auto iter = m_dynamicModels.find(key);
	if (iter != m_dynamicModels.end())
	{
		return iter->second;
	}
	CModel* pModel = CModel::CreateByBinary(m_pDevice, m_pContext, eType, WStringToString(filePath).c_str(), preMatrix);
	if (nullptr == pModel)
		return nullptr;
	RegisterDynamicModel(key, pModel);
	return pModel;
}

CShader* CResourceMag::GetShader(const _wstring& key)
{
	auto iter = m_staticShaders.find(key);
	if (iter != m_staticShaders.end())
	{
		Safe_AddRef(iter->second);
		return iter->second;
	}
    return nullptr;
}

CTexture* CResourceMag::GetTexture(const _wstring& key)
{
	auto iter = m_staticTextures.find(key);
	if (iter != m_staticTextures.end())
	{
		Safe_AddRef(iter->second);
		return iter->second;
	}
	return nullptr;
}


CModel* CResourceMag::GetModel(const _wstring& key)
{
	auto iter = m_staticModels.find(key);
	if (iter != m_staticModels.end())
	{
		Safe_AddRef(iter->second);
		return iter->second;
	}
	return nullptr;
}

CShader* CResourceMag::GetDynamicShader(const _wstring& key)
{
	auto iter = m_dynamicShaders.find(key);
	if (iter != m_dynamicShaders.end())
	{
		Safe_AddRef(iter->second);
		return iter->second;
	}
	return nullptr;
}

CTexture* CResourceMag::GetDynamicTexture(const _wstring& key)
{
	auto iter = m_dynamicTextures.find(key);
	if (iter != m_dynamicTextures.end())
	{
		Safe_AddRef(iter->second);
		return iter->second;
	}
	return nullptr;
}


CModel* CResourceMag::GetDynamicModel(const _wstring& key)
{
	auto iter = m_dynamicModels.find(key);
	if (iter != m_dynamicModels.end())
	{
		Safe_AddRef(iter->second);
		return iter->second;
	}
	return nullptr;
}

void CResourceMag::Clear()
{
	for (auto& Pair : m_dynamicShaders)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicShaders.clear();
	for (auto& Pair : m_dynamicTextures)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicTextures.clear();
}

HRESULT CResourceMag::Initialize()
{
	return S_OK;
}

CResourceMag* CResourceMag::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CResourceMag* pInstance = new CResourceMag(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created CResourceMag");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CResourceMag::Free()
{
	__super::Free();
	for (auto& Pair : m_staticShaders)
	{
		Safe_Release(Pair.second);
	}
	m_staticShaders.clear();
	for (auto& Pair : m_staticTextures)
	{
		Safe_Release(Pair.second);
	}
	m_staticTextures.clear();

	for (auto& Pair : m_dynamicShaders)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicShaders.clear();
	for (auto& Pair : m_dynamicTextures)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicTextures.clear();

	for (auto& Pair : m_staticModels)
	{
		Safe_Release(Pair.second);
	}
	m_staticModels.clear();
	for (auto& Pair : m_dynamicModels)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicModels.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
