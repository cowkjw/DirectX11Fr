#include "JsonLoader.h"
#include "UIImage.h"
#include "UIButton.h"
#include "GameInstance.h"
#include "EditorManager.h"
#include "Model.h"
#include <regex>

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
			else if (layout == "VTXANIMMESH")
			{
				pElems = VTXANIMMESH::Elements;
				iNum = VTXANIMMESH::iNumElements;
			}
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

HRESULT CJsonLoader::Load_Models(const string& filePath, function<void()> onEntryLoaded)
{
	ifstream ifs(filePath);
	if (!ifs.is_open())
		return E_FAIL;
	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));
	json j;
	try { ifs >> j; }
	catch (json::parse_error&)
	{
		return E_FAIL;
	}
	if (j.contains("models") && j["models"].is_array())
	{
		for (auto& entry : j["models"])
		{
			string tag = entry.value("tag", "");
			string path = entry.value("path", "");
			_bool bAnim = entry.value("Anim", false);

			if (tag.empty() || path.empty())
				continue;
			m_pGameInstance->LoadModel(
				StringToWString(tag).c_str(),
				StringToWString(path).c_str(),
				bAnim ? MODEL::ANIM : MODEL::NONANIM,
				PreTransformMatrix
			);
			//if (FAILED(m_pGameInstance->Add_Prototype(ToIndex(LEVEL::GAMEPLAY), StringToWString(tag),
			//	CModel::CreateByBinary(pDevice, pContext, bAnim ? MODEL::ANIM : MODEL::NONANIM, path.c_str(), PreTransformMatrix))))
			//	return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CJsonLoader::Load_Objects(const string& filePath, function<void()> onEntryLoaded)
{
	ifstream ifs(filePath);
	if (!ifs.is_open())
		return E_FAIL;
	json j;
	try {
		ifs >> j;
	}
	catch (json::parse_error&) {
		return E_FAIL;
	}
	if (!j.is_array())
		return E_FAIL;

	CEditorManager::m_vecSceneObjects.clear();
	for (_uint i = 1; i < ToIndex(LEVEL::END); ++i)
	{
		m_pGameInstance->ClearObejcts(i);
	}
	m_pGameInstance->ClearUI();


	// 3) ID → 객체 매핑 준비
	unordered_map<_uint, CGameObject*> idMap;
	idMap.reserve(j.size());
	vector<CGameObject*> allObjs;
	allObjs.reserve(j.size());

	// 4) JSON 각 엔트리별로 '클론' 생성
	for (const auto& entry : j)
	{
		_uint createLevel = entry["CreateLevel"].get<_uint>();
		_uint protoLevel = entry["ProtoLevel"].get<_uint>();
		wstring name = StringToWString(entry["name"].get<std::string>());
		wstring protoTag = StringToWString(entry["ProtoTypeTag"].get<std::string>());
		CGameObject* pObj = nullptr;
		if (entry.contains("UI"))
		{
			FactoryUI(reinterpret_cast<CUIObject**>(&pObj), entry);
		}
		else
		{
			// GameInstance에 미리 등록해둔 프로토타입으로 클론 생성
			pObj = m_pGameInstance->Add_GameObject(
				protoLevel,
				protoTag,
				createLevel,
				name
			);
		}
		if (!pObj) continue;

		pObj->Deserialize(entry);

		_uint id = entry["ID"].get<_uint>();
		idMap[id] = pObj;
		allObjs.push_back(pObj);
	}

	// 5) 부모·자식 관계 재구성
	for (const auto& entry : j)
	{
		_uint id = entry["ID"].get<_uint>();
		_uint parentId = entry["parentId"].get<_uint>();
		auto itChild = idMap.find(id);
		if (itChild == idMap.end())
			continue;

		CGameObject* pChild = itChild->second;
		if (parentId != 0)
		{
			auto itParent = idMap.find(parentId);
			if (itParent != idMap.end())
			{
				CGameObject* pParent = itParent->second;
				pParent->AddChild(pChild);
				continue;
			}
		}
	// parentId == 0 이면 최상위로 씬 루트 벡터에 추가
		CEditorManager::m_vecSceneObjects.push_back(pChild);
	}

	for (auto& entry : j)
	{
		_uint id = entry["ID"].get<_uint>();
		CGameObject* pObj = idMap[id];
		if (!pObj)
			continue;

		FactoryComponent(pObj, entry);
	}
	allObjs.clear();

	return S_OK;

}

HRESULT CJsonLoader::Save_Objects(const string& filePath, function<void()> onEntryLoaded)
{

	vector<CGameObject*> allObjs;
	for (auto* root : CEditorManager::m_vecSceneObjects)
	{
		if (root)
			CollectAllObjects(root, allObjs);
	}
	json jArr = json::array();

	for (auto* pObj : allObjs)
	{
		if (!pObj) continue;
		jArr.push_back(pObj->Serialize());
	}

	//// 2) 모든 실수를 소수점 2자리로 반올림
	//RoundJsonFloats(jArr, 2);


	string jsonStr = jArr.dump(4);

	// 5) 정규식으로 소수점 둘째 자리까지만 남기기
	//    음수 지원: -? 추가, 숫자가 문자열 안일 경우 영향을 줄 수 있으므로 JSON 구조에 숫자 문자열이 없을 때만 사용하세요.
	static const regex floatPattern(R"((-?\d+)\.(\d{1,2})\d*)");
	jsonStr = regex_replace(jsonStr, floatPattern, "$1.$2");

	// 6) 파일에 쓰기
	ofstream ofs(filePath);
	if (!ofs.is_open())
		return E_FAIL;
	ofs << jsonStr;

	return S_OK;
}

void CJsonLoader::CollectAllObjects(CGameObject* root, vector<CGameObject*>& out)
{
	out.push_back(root);
	for (auto& child : root->GetChildren())
	{
		CollectAllObjects(child, out);
	}
}

void CJsonLoader::FactoryComponent(CGameObject* pObj, const json& j)
{  // JSON 의 components 객체 순회
	for (auto& kv : j["components"].items())
	{
		const string& compKey = kv.key();       // e.g. "Com_Collider"
		const json& compData = kv.value();
		// Prototype 정보
		_uint compCreateLevel = compData.contains("ComponentLevel") ? compData["ComponentLevel"].get<_uint>() : -1;
		wstring protoCompTag = compData.contains("ComponentTag") ? StringToWString(compData["ComponentTag"].get<string>()) : L"";
		wstring wCompKey = StringToWString(compKey);
		CComponent* pComp = nullptr;
		if ((pComp = pObj->Get_Component(wCompKey)) == nullptr)
		{
			pObj->Add_Component(compCreateLevel, protoCompTag, wCompKey, &pComp);
			Safe_Release(pComp);
		}
		if (pComp)
			pComp->Deserialize(compData);
	}
}

void CJsonLoader::FactoryUI(CUIObject** pObjOut, const json& j)
{
	CUIObject::UIOBJECT_DESC uiDesc{};

	_uint createLevel = j["CreateLevel"].get<_uint>();
	_uint protoLevel = j["ProtoLevel"].get<_uint>();
	wstring name = StringToWString(j["name"].get<std::string>());
	UI_TYPE uiType = static_cast<UI_TYPE>(j["UIType"].get<_int>());


	uiDesc.strName = name;
	uiDesc.fX = j["Position"][0].get<_float>();
	uiDesc.fY = j["Position"][1].get<_float>();
	uiDesc.fSizeX = j["Size"][0].get<_float>();
	uiDesc.fSizeY = j["Size"][1].get<_float>();
	if (j.contains("ShaderKey"))
		uiDesc.strShaderKey = StringToWString(j["ShaderKey"].get<string>());
	if (j.contains("TextureKey"))
	{
		uiDesc.strTextureKey = StringToWString(j["TextureKey"].get<string>());
	}
	uiDesc.iSortingOrder = j["SortingOrder"].get<_int>();


	*pObjOut = static_cast<CUIObject*>(m_pGameInstance->CreateUI(&uiDesc, uiType));
	if (!*pObjOut)
		return;
}

void CJsonLoader::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}

