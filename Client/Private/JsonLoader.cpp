#include "JsonLoader.h"
#include "UIImage.h"
#include "UIButton.h"
#include "GameInstance.h"

CJsonLoader::CJsonLoader()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}


_int CJsonLoader::CountPrototypes(const string& filePath) const
{
	//ifstream ifs(filePath);
	//if (!ifs.is_open()) return 0;

	//json j;
	//try { ifs >> j; }
	//catch (...) { return 0; }

	//int count = 0;
	//if (j.contains("textures") && j["textures"].is_array())
	//	count += static_cast<int>(j["textures"].size());
	//if (j.contains("gameObjects") && j["gameObjects"].is_array())
	//	count += static_cast<int>(j["gameObjects"].size());


	return 0;
}

HRESULT CJsonLoader::Load_Prototypes(const string& filePath, function<void()> onEntryLoaded)
{
  

    return S_OK;
}

HRESULT CJsonLoader::Load_Shaders(const string& filePath, function<void()> onEntryLoaded)
{
    ifstream ifs(filePath);
    if (!ifs.is_open())
        return E_FAIL;

    json j;
    try { ifs >> j; }
    catch (json::parse_error&)
    {
        return E_FAIL;
    }
    if (j.contains("shaders") && j["shaders"].is_array())
    {
        for (auto& entry : j["shaders"])
        {
            string key = entry.value("key", "");
            string path = entry.value("path", "");
            string layout = entry.value("layout", "");
            _bool  isStatic = entry.value("static", false);

            if (key.empty() || path.empty() || layout.empty())
                continue;

            // layout 문자열에 따라 입력 레이아웃 요소 선택
            const D3D11_INPUT_ELEMENT_DESC* pElems = nullptr;
            _uint                              iNum = 0;

            if (layout == "VTXPOSTEX") {
                pElems = VTXPOSTEX::Elements;
                iNum = VTXPOSTEX::iNumElements;
            }
            else if (layout == "VTXNORTEX") {
                pElems = VTXNORTEX::Elements;
                iNum = VTXNORTEX::iNumElements;
            }
            else if (layout == "VTXMESH")
            {
				pElems = VTXMESH::Elements;
				iNum = VTXMESH::iNumElements;
			}
			//else if (layout == "VTXANIM") {
			//	pElems = VTXANIM::Elements;
			//	iNum = VTXANIM::iNumElements;
			//}
			//else if (layout == "VTXBONE") {
			//	pElems = VTXBONE::Elements;
			//	iNum = VTXBONE::iNumElements;
			//}
			//else if (layout == "VTXCOLLIDER") {
			//	pElems = VTXCOLLIDER::Elements;
			//	iNum = VTXCOLLIDER::iNumElements;
			//}
            else
            {
                continue; // 지원하지 않는 레이아웃인 경우
            }

            // LoadShader(키, 파일경로, 레이아웃, 요소 개수, static)
            m_pGameInstance->LoadShader(
                StringToWString(key),
                StringToWString(path),
                pElems,
                iNum,
                isStatic
            );
        }
    }
    return S_OK;
}

HRESULT CJsonLoader::Load_Textures(const string& filePath, function<void()> onEntryLoaded)
{
    ifstream ifs(filePath);
    if (!ifs.is_open())
        return E_FAIL;

    json j;
    try { ifs >> j; }
    catch (json::parse_error&)
    {
        return E_FAIL;
    }

    if (j.contains("textures") && j["textures"].is_array())
    {
        for (auto& entry : j["textures"])
        {
            string key = entry.value("key", "");
            string path = entry.value("path", "");
            _uint iNumTextures = entry.value("numTextures", 1);
            _bool  isStatic = entry.value("static", false);

            if (key.empty() || path.empty())
                continue;

            m_pGameInstance->LoadTexture(
                StringToWString(key).c_str(),
                StringToWString(path).c_str(),
                isStatic,
                iNumTextures
            );
        }
    }
    return S_OK;
}

void CJsonLoader::Free()
{
	__super::Free();	
	Safe_Release(m_pGameInstance);
}

