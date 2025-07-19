#include "JsonLoader.h"
#include "UIImage.h"
#include "UIButton.h"
#include "GameInstance.h"
#include "EditorManager.h"
#include "Model.h"
#include <regex>
#include "Animation.h"
#include "ParticleSystem.h"
#include "UIProgressBar.h"
#include <Environment.h>

CJsonLoader::CJsonLoader()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

CJsonLoader::CJsonLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pContext{ pContext }
	, m_pDevice{ pDevice }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
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
			else if (layout == "VTXRECT_PARTICLE_INSTANCE") 
			{
				pElems = VTXRECT_PARTICLE_INSTANCE::Elements;
				iNum = VTXRECT_PARTICLE_INSTANCE::iNumElements;
			}
			else if (layout == "VTXPOINT_PARTICLE_INSTANCE")
			{
				pElems = VTXPOINT_PARTICLE_INSTANCE::Elements;
				iNum = VTXPOINT_PARTICLE_INSTANCE::iNumElements;
			}
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

	//CEditorManager::m_vecSceneObjects.clear();
	//for (_uint i = 1; i < ToIndex(LEVEL::END); ++i)
	//{
	//	m_pGameInstance->ClearObejcts(i);
	//}
	//m_pGameInstance->ClearUI();


    //ID 객체 매핑 준비
	unordered_map<_uint, CGameObject*> idMap;
	idMap.reserve(j.size());
	vector<CGameObject*> allObjs;
	allObjs.reserve(j.size());

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

HRESULT CJsonLoader::Load_Particle(const string& filePath, CParticleSystem** ppParticle)
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

	CParticleSystem::PARTICLE_DESC particleDesc{};

	if (j.contains("ParticleType"))
		particleDesc.eParticleType = static_cast<PARTICLE_TYPE>(j["ParticleType"].get<int>());

	if (j.contains("NumInstance"))
		particleDesc.iNumInstance = j["NumInstance"].get<int>();

	if (j.contains("Range") && j["Range"].is_array() && j["Range"].size() == 3)
		particleDesc.vRange = { j["Range"][0], j["Range"][1], j["Range"][2] };

	if (j.contains("Size") && j["Size"].is_array() && j["Size"].size() == 2)
		particleDesc.vSize = { j["Size"][0], j["Size"][1] };

	if (j.contains("Center") && j["Center"].is_array() && j["Center"].size() == 3)
		particleDesc.vCenter = { j["Center"][0], j["Center"][1], j["Center"][2] };

	if (j.contains("IsLoop"))
		particleDesc.isLoop = j["IsLoop"].get<bool>();

	if (j.contains("LifeTime") && j["LifeTime"].is_array() && j["LifeTime"].size() == 2)
		particleDesc.vLifeTime = { j["LifeTime"][0], j["LifeTime"][1] };

	if (j.contains("Speed") && j["Speed"].is_array() && j["Speed"].size() == 2)
		particleDesc.vSpeed = { j["Speed"][0], j["Speed"][1] };

	if (j.contains("StartColor") && j["StartColor"].is_array() && j["StartColor"].size() == 3)
		particleDesc.vStartColor = { j["StartColor"][0], j["StartColor"][1], j["StartColor"][2] };

	if (j.contains("EndColor") && j["EndColor"].is_array() && j["EndColor"].size() == 3)
		particleDesc.vEndColor = { j["EndColor"][0], j["EndColor"][1], j["EndColor"][2] };

	if (j.contains("Velocity") && j["Velocity"].is_array() && j["Velocity"].size() == 3)
		particleDesc.vVelocity = { j["Velocity"][0], j["Velocity"][1], j["Velocity"][2] };

	if (j.contains("Gravity"))
		particleDesc.fGravity = j["Gravity"].get<float>();

	if (j.contains("SpreadAngle"))
		particleDesc.fSpreadAngle = j["SpreadAngle"].get<float>();

	if (j.contains("AlphaVariation"))
		particleDesc.fAlphaVariation = j["AlphaVariation"].get<float>();

	if (j.contains("PlayAwake"))
		particleDesc.bPlayAwake = j["PlayAwake"].get<bool>();
	if (j.contains("Use3DSize"))
		particleDesc.b3DSize = j["Use3DSize"].get<bool>();
	if (j.contains("3DSize") && j["3DSize"].is_array() && j["3DSize"].size() == 3)
	{
		particleDesc.v3DSize = {
			j["3DSize"][0].get<float>(),
			j["3DSize"][1].get<float>(),
			j["3DSize"][2].get<float>()
		};
	}
	if (j.contains("Use3DRotation"))
		particleDesc.b3DRotation = j["Use3DRotation"].get<bool>();
	if (j.contains("3DRotation") && j["3DRotation"].is_array() && j["3DRotation"].size() == 3)
	{
		particleDesc.v3DRotation = {
			j["3DRotation"][0].get<float>(),
			j["3DRotation"][1].get<float>(),
			j["3DRotation"][2].get<float>()
		};
	}

	*ppParticle = CParticleSystem::Create(m_pDevice, m_pContext, particleDesc);

	if (*ppParticle)
	{
		(*ppParticle)->Initialize(nullptr);
		return S_OK;
	}

	return E_FAIL;
}

HRESULT CJsonLoader::Load_CutSceneCamera(const string& filePath, CGameObject* pCamera)
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
	pCamera->Deserialize(j);

	return S_OK;
}

json CJsonLoader::Load_CutScene_PropertyAsJson(const string& filePath)
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
	return j;
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
		if (dynamic_cast<CUIButton*>(pObj->GetParent())|| dynamic_cast<CUIProgressBar*>(pObj->GetParent())) // 버튼에 있는 Image는 제외
			continue;
		jArr.push_back(pObj->Serialize());
	}

	string jsonStr = jArr.dump(4);
	static const regex floatPattern(R"((-?\d+)\.(\d{1,2})\d*)");
	jsonStr = regex_replace(jsonStr, floatPattern, "$1.$2");

	// 6) 파일에 쓰기
	ofstream ofs(filePath);

	if (!ofs.is_open())
		return E_FAIL;
	ofs << jsonStr;

	return S_OK;
}

HRESULT CJsonLoader::Save_Particle(const string& filePath, CParticleSystem* pParticle)
{
	if (!pParticle)
		return E_FAIL;
	json j = pParticle->Serialize();
	ofstream ofs(filePath);
	if (!ofs.is_open())
		return E_FAIL;
	ofs << j.dump(4);
	return S_OK;
}

HRESULT CJsonLoader::Save_CutSceneCamera(const string& filePath, CGameObject* pCamera)
{
	if (!pCamera)
		return E_FAIL;
	json j = pCamera->Serialize();
	ofstream ofs(filePath);
	if (!ofs.is_open())
		return E_FAIL;
	ofs << j.dump(4);
	return S_OK;
}

HRESULT CJsonLoader::LoadAnimEvent(const string& filePath, vector<CAnimation*>& animations)
{
	json root;
	ifstream ifs(filePath);
	if (ifs.is_open()) {
		ifs >> root;
		// "animations" 배열 순회
		for (auto& jclip : root["animations"])
		{
			string clipName = jclip["clipName"];
			for (auto& clip : animations)
			{
		
				if (clip->Get_Name() == clipName)
				{
					if (clipName == "A_P0012_V00_C00_AtkSkl03_0")
					{
						int a = 0;
					}
					// 기존 이벤트 지우고 새로 채우기
					clip->GetEvents().clear();
					for (auto& jev : jclip["events"])
					{
						float t = jev["time"];
						string n = jev["name"];
						clip->AddEvent({ t, n });
					}
					break;
				}
			}
		}
	}
	else
	{
		return E_FAIL; // 파일 열기 실패
	}
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


		if (compKey.find("Collider") != string::npos)
		{
			if (compData.contains("Type"))
			{
				string typeStr = compData["Type"].get<string>();

				if (typeStr == "CapsuleCollider")
				{
					pComp = CCapsuleCollider::Create(m_pDevice, m_pContext,
						compData["Radius"].get<_float>(),
						compData["Height"].get<_float>());
				}
				else if (typeStr == "BoxCollider")
				{
					pComp = CBoxCollider::Create(m_pDevice, m_pContext);
				}
				else if (typeStr == "SphereCollider")
				{
					pComp = CSphereCollider::Create(m_pDevice, m_pContext,
						compData["Radius"].get<_float>());
				}

				CComponent* tmp = nullptr;
				pObj->Add_Component(wCompKey,pComp,&tmp);
				pComp->Initialize(nullptr);
				Safe_Release(tmp);
			}
		}

		else
		{
			if ((pComp = pObj->Get_Component(wCompKey)) == nullptr)
			{
				pObj->Add_Component(compCreateLevel, protoCompTag, wCompKey, &pComp);
				Safe_Release(pComp);
			}
		}
		if (pComp)
		{
			pComp->Deserialize(compData);
			if (auto pEnv = dynamic_cast<CEnvironment*>(pComp->GetOwner()))
			{
				pEnv->SetInitPos();
			}
		}
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

	if (uiType == UI_TYPE::BUTTON)
	{
		auto pbutton = static_cast<CUIButton*>(*pObjOut);
		CUIObject* pBtnImgObj = nullptr;
		CUIObject* pHoverImgObj = nullptr;

		// JSON 안 키 이름이 정확한지 확인 ("ButtonImage", "HoverImage" 와 일치해야 함)
		FactoryUI(&pBtnImgObj, j["ButtonImage"]);
		FactoryUI(&pHoverImgObj, j["HoverImage"]);

		// CUIImage* 타입으로 변환
		CUIImage* pBtnImage = dynamic_cast<CUIImage*>(pBtnImgObj);
		CUIImage* pHoverImage = dynamic_cast<CUIImage*>(pHoverImgObj);
		pBtnImage->Deserialize(j["ButtonImage"]);
		pHoverImage->Deserialize(j["HoverImage"]);

		FactoryComponent(pBtnImage, j["ButtonImage"]);
		FactoryComponent(pHoverImage, j["HoverImage"]);


		// 버튼에 설정
		pbutton->SetButtonImage(pBtnImage, pHoverImage);

	}
	else if (uiType == UI_TYPE::BAR)
	{
		auto pUiBar = static_cast<CUIProgressBar*>(*pObjOut);
		CUIObject* pBGImageObj = nullptr;
		CUIObject* pFillImageObj = nullptr;
		CUIObject* pDamageObj = nullptr;
		FactoryUI(&pBGImageObj, j["BGImage"]);
		FactoryUI(&pFillImageObj, j["FillImage"]);
		FactoryUI(&pDamageObj, j["DamageImage"]);

		CUIImage* pBGImage = dynamic_cast<CUIImage*>(pBGImageObj);
		CUIImage* pFillImage = dynamic_cast<CUIImage*>(pFillImageObj);
		CUIImage* pDamageImage = dynamic_cast<CUIImage*>(pDamageObj);
		pBGImage->Deserialize(j["BGImage"]);
		pFillImage->Deserialize(j["FillImage"]);
		pDamageImage->Deserialize(j["DamageImage"]);
		FactoryComponent(pBGImage, j["BGImage"]);
		FactoryComponent(pFillImage, j["FillImage"]);
		FactoryComponent(pDamageImage, j["DamageImage"]);
		pUiBar->SetBarImage(pBGImage, pFillImage, pDamageImage);
	}

	if (auto pUiImage = dynamic_cast<CUIImage*>(*pObjOut))
	{
		if (j.contains("UVOffset"))
		{
			pUiImage->Deserialize(j);
		}
	}
}

void CJsonLoader::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}

