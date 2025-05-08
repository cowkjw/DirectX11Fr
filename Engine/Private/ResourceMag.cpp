#include "ResourceMag.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer.h"
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
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pTexture;
	}
}

void CResourceMag::RegisterDynamicBuffer(const _wstring& key, CVIBuffer* pBuffer)
{
	auto iter = m_dynamicBuffers.find(key);
	if (iter == m_dynamicBuffers.end())
	{
		m_dynamicBuffers.emplace(key, pBuffer);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pBuffer;
	}
}


void CResourceMag::RegisterStaticShader(const _wstring& key, CShader* pShader)
{
	auto iter = m_staticShaders.find(key);
	if (iter == m_staticShaders.end())
	{
		m_staticShaders.emplace(key, pShader);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pShader;
	}
}

void CResourceMag::RegisterStaticTexture(const _wstring& key, CTexture* pTexture)
{
	auto iter = m_staticTextures.find(key);
	if (iter == m_staticTextures.end())
	{
		m_staticTextures.emplace(key, pTexture);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pTexture;
	}
}

void CResourceMag::RegisterStaticBuffer(const _wstring& key, CVIBuffer* pBuffer)
{
	auto iter = m_staticBuffers.find(key);
	if (iter == m_staticBuffers.end())
	{
		m_staticBuffers.emplace(key, pBuffer);
	}
	else
	{
		Safe_Release(iter->second);
		iter->second = pBuffer;
	}
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

CVIBuffer* CResourceMag::LoadBuffer(const _wstring& key, BUFFER_TYPE eType)
{
	auto iter = m_staticBuffers.find(key);
	if (iter != m_staticBuffers.end())
	{
		return iter->second;
	}
	CVIBuffer* pBuffer = nullptr;
	switch (eType)
	{
	case BUFFER_TYPE::RECT:
		pBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
		if (nullptr == pBuffer)
			return nullptr;
		RegisterStaticBuffer(key, pBuffer);
		return pBuffer;
		break;
	//case BUFFER_TYPE::CUBE:
	//	pBuffer = CVIBuffer_Cube::Create(m_pDevice, m_pContext);
	//	break;
		
	}
   
	return nullptr;
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

CVIBuffer* CResourceMag::LoadDynamicBuffer(const _wstring& key, BUFFER_TYPE eType)
{
	auto iter = m_dynamicBuffers.find(key);
	if (iter != m_dynamicBuffers.end())
	{
		return iter->second;
	}
	CVIBuffer* pBuffer = nullptr;
	switch (eType)
	{
	case BUFFER_TYPE::RECT:
		pBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
		if (nullptr == pBuffer)
			return nullptr;
		RegisterDynamicBuffer(key, pBuffer);
		return pBuffer;
		//case BUFFER_TYPE::CUBE:
		//	pBuffer = CVIBuffer_Cube::Create(m_pDevice, m_pContext);
		//	break;
	}
	return nullptr;
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

CVIBuffer* CResourceMag::GetBuffer(const _wstring& key)
{
	auto iter = m_staticBuffers.find(key);
	if (iter != m_staticBuffers.end())
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

CVIBuffer* CResourceMag::GetDynamicBuffer(const _wstring& key)
{
	auto iter = m_dynamicBuffers.find(key);
	if (iter != m_dynamicBuffers.end())
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
	for (auto& Pair : m_dynamicBuffers)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicBuffers.clear();
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
	for (auto& Pair : m_staticBuffers)
	{
		Safe_Release(Pair.second);
	}
	m_staticBuffers.clear();

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
	for (auto& Pair : m_dynamicBuffers)
	{
		Safe_Release(Pair.second);
	}
	m_dynamicBuffers.clear();


	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
