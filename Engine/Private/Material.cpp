#include "Material.h"
#include "Shader.h"

CMaterial::CMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }    
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CMaterial::Initialize(const _char* pModelFilePath, const aiMaterial* pAIMaterial)
{
	m_TexturePaths.resize(AI_TEXTURE_TYPE_MAX);
    for (_uint i = 1; i < AI_TEXTURE_TYPE_MAX; i++)
    {
        _uint       iNumSRVs = pAIMaterial->GetTextureCount(static_cast<aiTextureType>(i));

        m_SRVs[i].reserve(iNumSRVs);
        m_TexturePaths[i].reserve(iNumSRVs);

        for (_uint j = 0; j < iNumSRVs; j++)
        {
            aiString     strTexturePath;

            if (FAILED(pAIMaterial->GetTexture(static_cast<aiTextureType>(i), j, &strTexturePath)))
                return E_FAIL;

            // 텍스쳐 경로 저장
          

            _char       szFullPath[MAX_PATH] = {};
            _char       szDrive[MAX_PATH] = {};
            _char       szDir[MAX_PATH] = {};
            _char       szFileName[MAX_PATH] = {};
            _char       szExt[MAX_PATH] = {};

            _splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDir, MAX_PATH, nullptr, 0, nullptr, 0);
            _splitpath_s(strTexturePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

            strcpy_s(szFullPath, szDrive);
            strcat_s(szFullPath, szDir);
            strcat_s(szFullPath, szFileName);
            strcat_s(szFullPath, szExt);

            _tchar      szTextureFilePath[MAX_PATH] = {};

            MultiByteToWideChar(CP_ACP, 0, szFullPath, (_int)strlen(szFullPath), szTextureFilePath, MAX_PATH);

            HRESULT         hr = { };
            ID3D11ShaderResourceView* pSRV = { nullptr };

            if (false == strcmp(szExt, ".dds"))
                hr = DirectX::CreateDDSTextureFromFile(m_pDevice, szTextureFilePath, nullptr, &pSRV);
            else           
                hr = DirectX::CreateWICTextureFromFile(m_pDevice, szTextureFilePath, nullptr, &pSRV);

            if (FAILED(hr))
                return E_FAIL;

            m_SRVs[i].push_back(pSRV);
            m_TexturePaths[i].push_back(WStringToString(_wstring(szTextureFilePath)));
        }
    }

    return S_OK;
}

HRESULT CMaterial::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, aiTextureType eType, _uint iTextureIndex)
{
    if (iTextureIndex >= m_SRVs[eType].size())
        return E_FAIL;

    return pShader->Bind_SRV(pConstantName, m_SRVs[eType][iTextureIndex]);    
}



CMaterial* CMaterial::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pModelFilePath, const aiMaterial* pAIMaterial)
{
    CMaterial* pInstance = new CMaterial(pDevice, pContext);

    if (FAILED(pInstance->Initialize(pModelFilePath, pAIMaterial)))
    {
        MSG_BOX("Failed to Created : CMaterial");
        Safe_Release(pInstance);
    }

    return pInstance;
}

HRESULT CMaterial::ExportBinary(ofstream& ofs)
{
    // 1) 매직 넘버
    uint32_t magic = 0x4D42494D; // 'MBIM'
    ofs.write((char*)&magic, sizeof(magic));

    // 2) 텍스처 타입 개수 (고정)
    uint32_t typeCount = AI_TEXTURE_TYPE_MAX;
    ofs.write((char*)&typeCount, sizeof(typeCount));

    // 3) 각 타입별 경로 저장
    for (uint32_t t = 1; t < typeCount; ++t)
    {
        uint32_t cnt = (uint32_t)m_TexturePaths[t].size();
        ofs.write((char*)&cnt, sizeof(cnt));

        for (auto& path : m_TexturePaths[t])
        {
            uint32_t len = (uint32_t)path.size();
            ofs.write((char*)&len, sizeof(len));
            ofs.write(path.data(), len);
        }
    }

    return S_OK;
}

CMaterial* CMaterial::CreateByBinary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& ifs)
{
    uint32_t magic;
    ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != 0x4D42494D) // 'MBIM'
        return nullptr;

    // 2) 텍스처 타입 개수 읽기
    uint32_t typeCount;
    ifs.read(reinterpret_cast<char*>(&typeCount), sizeof(typeCount));
    // 준비
    CMaterial* pMat = new CMaterial(pDevice, pContext);
    pMat->m_TexturePaths.resize(typeCount);

    // 3) 타입별 경로 읽어 SRV 생성
    for (uint32_t t = 1; t < typeCount; ++t) {
        uint32_t cnt;
        ifs.read(reinterpret_cast<char*>(&cnt), sizeof(cnt));
        for (uint32_t i = 0; i < cnt; ++i) {
            uint32_t len;
            ifs.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string path(len, '\0');
            ifs.read(&path[0], len);

            pMat->m_TexturePaths[t].push_back(path);

            // WideChar 변환
            std::wstring wpath = std::wstring(path.begin(), path.end());
            ID3D11ShaderResourceView* pSRV = nullptr;
            HRESULT hr; 
            if (path.size() >= 4 && path.substr(path.size() - 4) == ".dds")
                hr = DirectX::CreateDDSTextureFromFile(pDevice, wpath.c_str(), nullptr, &pSRV);
            else
                hr = DirectX::CreateWICTextureFromFile(pDevice, wpath.c_str(), nullptr, &pSRV);
            if (FAILED(hr)) {
                delete pMat;
                return nullptr;
            }
            pMat->m_SRVs[t].push_back(pSRV);
        }
    }

    return pMat;
}

void CMaterial::Free()
{
    __super::Free();

    for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
    {
        for (auto& pSRV : m_SRVs[i])
            Safe_Release(pSRV);
        m_SRVs[i].clear();
    }

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
