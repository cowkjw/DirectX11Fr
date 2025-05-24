#pragma once
#include "Base.h"

BEGIN_NAMESPACE(Engine)

class CShader;
class CTexture;
class CVIBuffer;
class CModel;

class CResourceMag final : public CBase
{
private:
	CResourceMag(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CResourceMag() = default;

public:

    HRESULT Initialize();

	// 동적 리소스 등록

	void RegisterDynamicShader(const _wstring& key, CShader* pShader);
	void RegisterDynamicTexture(const _wstring& key, CTexture* pTexture);
	void RegisterDynamicBuffer(const _wstring& key, CVIBuffer* pBuffer);
	void RegisterDynamicModel(const _wstring& key, CModel* pModel);


    // 정적 리소스 등록
    void RegisterStaticShader(const _wstring& key, CShader* pShader);
    void RegisterStaticTexture(const _wstring& key, CTexture* pTexture);
    void RegisterStaticBuffer(const _wstring& key, CVIBuffer* pBuffer);
	void RegisterStaticModel(const _wstring& key, CModel* pModel);


    // 정적 리소스 로드 
    CShader* LoadShader(const _wstring& key, const _wstring& vsPath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements);
    CTexture* LoadTexture(const _wstring& key, const _wstring& filePath,_uint iNumTextrues = 1);
    CVIBuffer* LoadBuffer(const _wstring& key, BUFFER_TYPE eType);
	CModel* LoadModel(const _wstring& key, const _wstring& filePath, MODEL eType, _matrix preMatrix);

    // 동적 리소스 로드
	CShader* LoadDynamicShader(const _wstring& key, const _wstring& vsPath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements);
	CTexture* LoadDynamicTexture(const _wstring& key, const _wstring& filePath, _uint iNumTextrues = 1);
	CVIBuffer* LoadDynamicBuffer(const _wstring& key, BUFFER_TYPE eType);
	CModel* LoadDynamicModel(const _wstring& key, const _wstring& filePath, MODEL eType, _matrix preMatrix);

	// 정적 리소스 가져오기
    CShader* GetShader(const _wstring& key);
    CTexture* GetTexture(const _wstring& key);
    CVIBuffer* GetBuffer(const _wstring& key);
	CModel* GetModel(const _wstring& key);

	// 동적 리소스 가져오기
	CShader* GetDynamicShader(const _wstring& key);
	CTexture* GetDynamicTexture(const _wstring& key);
	CVIBuffer* GetDynamicBuffer(const _wstring& key);
	CModel* GetDynamicModel(const _wstring& key);


    // 레벨 언로드 시 동적 리소스만 삭제
    void Clear();

    const vector<_wstring>& GetShaderKeys(_bool bIsStatic = true) const { return  bIsStatic ? m_StaticShaderKeyList  : m_DynamicShaderKeyList; }
    const vector<_wstring>& GetTextureKeys(_bool bIsStatic = true) const { return  bIsStatic ? m_StaticTextureKeyList : m_DynamicTextureKeyList; }
    const vector<_wstring>& GetModelKeys(_bool bIsStatic = true) const { return  bIsStatic ? m_StaticModelKeyList : m_DynamicModelKeyList; }



private:

    unordered_map<_wstring, CShader*>   m_staticShaders;
    unordered_map<_wstring, CTexture*>  m_staticTextures;
    unordered_map<_wstring, CVIBuffer*>   m_staticBuffers;
    unordered_map<_wstring, CModel*>   m_staticModels;


    unordered_map<_wstring, CShader*>   m_dynamicShaders;
    unordered_map<_wstring, CTexture*>  m_dynamicTextures;
    unordered_map<_wstring, CVIBuffer*>   m_dynamicBuffers;
    unordered_map<_wstring, CModel*>   m_dynamicModels;



    
    vector<_wstring> m_StaticShaderKeyList;
    vector<_wstring> m_DynamicShaderKeyList;

    vector<_wstring> m_StaticTextureKeyList;
    vector<_wstring> m_DynamicTextureKeyList;

	vector<_wstring> m_StaticModelKeyList;
	vector<_wstring> m_DynamicModelKeyList;


	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
public:
	static CResourceMag* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free();
};
END_NAMESPACE

