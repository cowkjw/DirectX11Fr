#include "JsonLoader.h"
#include "GameInstance.h"

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
	/*ifstream file(filePath);
	if (!file.is_open())
	{
		return E_FAIL;
	}

	json j;
	file >> j;*/


	return S_OK;
}

void CJsonLoader::Free()
{
	__super::Free();	
	Safe_Release(m_pGameInstance);
}

