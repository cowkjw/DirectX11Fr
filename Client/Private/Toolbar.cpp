#include "Toolbar.h"
#include <UICanvas.h>
#include <UIButton.h>
#include <UIImage.h>
#include <UIProgressBar.h>
#include <GameInstance.h>
#include <EditorManager.h>
#include <BaseCharacter.h>
#include "Model.h"
#include "Animation.h"
#include "Animator.h"  
#include "AnimController.h"
#include <Environment.h>
#include "Cell.h"
#include "Navigation.h"
#include "ParticleSystem.h"
#include "Texture.h"
#include "Shader.h"

CToolbar::CToolbar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPannel(pDevice, pContext)
{

}
HRESULT CToolbar::Initialize()
{
	Get_PrototypeList();
	RegisterDefaultPrototypes();
	SetLevelEnumToString();
	m_ShaderKeys = m_pGameInstance->GetShaderKeys();
	m_TextureKeys = m_pGameInstance->GetTextureKeys();
	m_ModelKeys = m_pGameInstance->GetModelKeys();
	m_FilePathBuf[0] = '\0';

	m_JsonLoader = new CJsonLoader(m_pDevice, m_pContext);

	m_pNavigation = CNavigation::Create(m_pDevice, m_pContext,nullptr);
	if (nullptr == m_pNavigation)
		return E_FAIL;

	m_pPreviewShader = m_pGameInstance->GetShader(TEXT("Shader_VtxRectInstance"), true);
	m_pPreviewTexture = m_pGameInstance->GetTexture(TEXT("TitleLogo"), true);
	return S_OK;
}

void CToolbar::Update(_float fTimeDelta)
{
	if (m_pParticleSystem)
	{
		if (FAILED(m_pParticleSystem->UpdateVertexInstances(fTimeDelta)))
			return;
	}
}

HRESULT CToolbar::Render()
{
	DrawToolbar();
	FBXLodaer();
	DrawAnimEventEditor();
	DrawParticleEditor();
	if (m_pNavigation)
		m_pNavigation->Render();

	if (m_pPreviewShader && m_pPreviewTexture)
	{
		_float4x4 worldMatrix = { 1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		};
	if (FAILED(m_pPreviewShader->Bind_Matrix("g_WorldMatrix", &worldMatrix)))
		return E_FAIL;
	if (FAILED(m_pPreviewShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::VIEW))))
		return E_FAIL;
	if (FAILED(m_pPreviewShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(TRANSFORM::PROJECTION))))
		return E_FAIL;


	if (FAILED(m_pPreviewTexture->Bind_ShaderResource(m_pPreviewShader, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pPreviewShader->Begin(0)))
		return E_FAIL;
	}
	if (m_pParticleSystem)
	{
		if (FAILED(m_pParticleSystem->Bind_Buffers()))
			return E_FAIL;

		if (FAILED(m_pParticleSystem->Render()))
			return E_FAIL;
	}
	return S_OK;
}

void CToolbar::DrawToolbar()
{
	ImGui::Begin("Toolbar");

	ImGui::Checkbox("Orthographic Gizmo", &CEditorManager::m_bOrthoGizmo);
	// UI 여부 판단
	_bool isUI = (m_CurrentPrototype == "Canvas" || m_CurrentPrototype == "Button" ||
		m_CurrentPrototype == "Image" || m_CurrentPrototype == "Bar");
	string curLevel = "Unknown";
	// UI 파라미터 입력
	static CUIObject::UIOBJECT_DESC uiDesc{};
	static CUIButton::UIButtonDesc buttonDesc{};
	static CUIProgressBar::BAR_DESC barDesc{};
	uiDesc.fSizeX = 1.f;
	uiDesc.fSizeY = 1.f;

	if (isUI)
	{
		ImGui::Text("Configure %s", m_CurrentPrototype.c_str());
		ImGui::DragFloat2("Position", &uiDesc.fX, 1.0f, -10000.f, 10000.f);
		ImGui::DragFloat2("Size", &uiDesc.fSizeX, 0.1f, 0.1f, 1000.f);



		// 현재 선택된 문자열을 표시하기 위해 변환
		string curShader = WStringToString(uiDesc.strShaderKey);
		if (ImGui::BeginCombo("Shader Key", curShader.c_str()))
		{
			for (size_t i = 0; i < m_ShaderKeys.size(); ++i)
			{
				// 벡터에서 꺼낸 wstring을 string으로 변환
				string key = WStringToString(m_ShaderKeys[i]);
				_bool selected = (uiDesc.strShaderKey == m_ShaderKeys[i]);
				if (ImGui::Selectable(key.c_str(), selected))
				{
					uiDesc.strShaderKey = m_ShaderKeys[i];  // 선택 시 wstring으로 저장
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		string curTex = WStringToString(uiDesc.strTextureKey);
		if (ImGui::BeginCombo("Texture Key", curTex.c_str()))
		{
			for (size_t i = 0; i < m_TextureKeys.size(); ++i)
			{
				string key = WStringToString(m_TextureKeys[i]);
				_bool selected = (uiDesc.strTextureKey == m_TextureKeys[i]);
				if (ImGui::Selectable(key.c_str(), selected))
				{
					uiDesc.strTextureKey = m_TextureKeys[i];
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		_int order = uiDesc.iSortingOrder;
		ImGui::InputInt("Sorting Order", &order);
		uiDesc.iSortingOrder = order;

		//if (ImGui::BeginCombo("LEVEL", curLevel.c_str()))
		//{
		//    for (auto& kv : m_LevelStringMap)
		//    {
		//        const string& key = kv.first;
		//        _uint value = kv.second;
		//        _bool selected = (uiDesc.iLevel == value);

		//        if (ImGui::Selectable(key.c_str(), selected))
		//        {
		//            uiDesc.iLevel = value;
		//            m_iCurrentSelectedLevel = value;
		//            UpdatePrototypeList();
		//        }
		//        if (selected)
		//            ImGui::SetItemDefaultFocus();
		//    }
		//    ImGui::EndCombo();
		//}

		if (m_CurrentPrototype == "Button")
		{
			string curTex = WStringToString(buttonDesc.strButtonImageHoverKey);
			if (ImGui::BeginCombo("HoverTexture Key", curTex.c_str()))
			{
				for (size_t i = 0; i < m_TextureKeys.size(); ++i)
				{
					string key = WStringToString(m_TextureKeys[i]);
					_bool selected = (buttonDesc.strButtonImageHoverKey == m_TextureKeys[i]);
					if (ImGui::Selectable(key.c_str(), selected))
					{
						buttonDesc.strButtonImageHoverKey = m_TextureKeys[i];
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else if (m_CurrentPrototype == "Bar")
		{
			string curTex = WStringToString(barDesc.strFillTextureKey);
			if (ImGui::BeginCombo("FillTexture Key", curTex.c_str()))
			{
				for (size_t i = 0; i < m_TextureKeys.size(); ++i)
				{
					string key = WStringToString(m_TextureKeys[i]);
					_bool selected = (barDesc.strFillTextureKey == m_TextureKeys[i]);
					if (ImGui::Selectable(key.c_str(), selected))
					{
						barDesc.strFillTextureKey = m_TextureKeys[i];
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			string curDamageTex = WStringToString(barDesc.strDamageTextureKey);
			if (ImGui::BeginCombo("DamageTexture Key", curDamageTex.c_str()))
			{
				for (size_t i = 0; i < m_TextureKeys.size(); ++i)
				{
					string key = WStringToString(m_TextureKeys[i]);
					_bool selected = (barDesc.strDamageTextureKey == m_TextureKeys[i]);
					if (ImGui::Selectable(key.c_str(), selected))
					{
						barDesc.strDamageTextureKey = m_TextureKeys[i];
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			string curUVShader = WStringToString(barDesc.strBarShaderKey);
			if (ImGui::BeginCombo("BarShader Key", curUVShader.c_str()))
			{
				for (size_t i = 0; i < m_ShaderKeys.size(); ++i)
				{
					// 벡터에서 꺼낸 wstring을 string으로 변환
					string key = WStringToString(m_ShaderKeys[i]);
					_bool selected = (barDesc.strBarShaderKey == m_ShaderKeys[i]);
					if (ImGui::Selectable(key.c_str(), selected))
					{
						barDesc.strBarShaderKey = m_ShaderKeys[i];  // 선택 시 wstring으로 저장
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		ImGui::Separator();
	}
	else
	{
		ImGui::InputText("Instance Name", m_NameBuf, IM_ARRAYSIZE(m_NameBuf));

		if (ImGui::BeginCombo("Model Key", m_ModelKey.c_str()))
		{
			for (size_t i = 0; i < m_ModelKeys.size(); ++i)
			{
				// 벡터에서 꺼낸 wstring을 string으로 변환
				string key = WStringToString(m_ModelKeys[i]);
				_bool selected = (StringToWString(m_ModelKey) == m_ModelKeys[i]);
				if (ImGui::Selectable(key.c_str(), selected))
				{
					m_ModelKey = WStringToString(m_ModelKeys[i]);
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	for (auto& kv : m_LevelStringMap)
	{
		if (kv.second == uiDesc.iLevel)
		{
			curLevel = kv.first;
			break;
		}
	}
	if (ImGui::BeginCombo("LEVEL", curLevel.c_str()))
	{
		for (auto& kv : m_LevelStringMap)
		{
			const string& key = kv.first;
			_uint value = kv.second;
			_bool selected{ false };

			if (ImGui::Selectable(key.c_str(), selected))
			{
				uiDesc.iLevel = value;
				m_iCurrentSelectedLevel = value;
				selected = true;
				UpdatePrototypeList();
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// 프로토타입 콤보
	if (ImGui::BeginCombo("ProtoType", m_CurrentPrototype.c_str()))
	{
		for (auto& kv : m_PrototypeSet)
		{
			_bool sel = (kv == m_CurrentPrototype);
			if (ImGui::Selectable(kv.c_str(), sel))
				m_CurrentPrototype = kv;
			if (sel) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	static _bool spawnMouse = false;
	ImGui::Checkbox("SpawnMouse", &spawnMouse);
	// 생성 버튼
	if (ImGui::Button("Create Object"))
	{
	

		CGameObject* obj = nullptr;
		if (isUI)
		{

			if (m_CurrentPrototype == "Canvas") obj = m_pGameInstance->CreateUI(&uiDesc, UI_TYPE::CANVAS);
			else if (m_CurrentPrototype == "Button")
			{
				memcpy(&buttonDesc, &uiDesc, sizeof(uiDesc));
				obj = m_pGameInstance->CreateUI(&buttonDesc, UI_TYPE::BUTTON);
			}
			else if (m_CurrentPrototype == "Image")  obj = m_pGameInstance->CreateUI(&uiDesc, UI_TYPE::IMAGE);
			else if (m_CurrentPrototype == "Bar")
			{
				memcpy(&barDesc, &uiDesc, sizeof(uiDesc));
				obj = m_pGameInstance->CreateUI(&barDesc, UI_TYPE::BAR);
			}
		}
		else
		{
			wstring wname = m_NameBuf[0]
				? StringToWString(m_NameBuf)
				: StringToWString(m_CurrentPrototype);
		
			obj = ClonePrototype(m_CurrentPrototype, wname);
			// 생성 후 입력란 초기화
			m_NameBuf[0] = '\0';
		}

		// 레벨에 따라 객체를 추가
		if (obj)
		{
			CEditorManager::m_vecSceneObjects.push_back(obj);
		}
		
	}
	/*if (spawnMouse)
	{
		if(m_pGameInstance->IsMousePressed(0))
		SpawnMouse();
	}*/

	ImGui::Separator();
	ImGui::InputText("Scene Path", m_FilePathBuf, IM_ARRAYSIZE(m_FilePathBuf), ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("..."))  // 파일 다이얼로그 버튼
	{
		// OPENFILENAME 구조체 초기화
		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetActiveWindow();           // ImGui 창의 HWND를 넘겨주세요
		ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
		ofn.lpstrFile = m_FilePathBuf;               // 선택된 파일 경로 버퍼
		ofn.nMaxFile = sizeof(m_FilePathBuf);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		// 열기 대화상자 표시
		if (GetOpenFileNameA(&ofn))
		{
			// m_FilePathBuf가 선택된 파일 경로로 업데이트됩니다.
		}
	}

	// Save 버튼 클릭 시
	if (ImGui::Button("Save Scene"))
	{
		std::string path(m_FilePathBuf);
		if (path.empty() || FAILED(m_JsonLoader->Save_Objects(path, []() {})))
			ImGui::OpenPopup("Save Error");

		if (ImGui::BeginPopup("Save Error"))
		{
			ImGui::Text("파일을 저장할 수 없습니다.\n경로를 확인하세요.");
			ImGui::EndPopup();
		}
	}

	ImGui::SameLine();

	// Load 버튼 클릭 시
	if (ImGui::Button("Load Scene"))
	{
		string path(m_FilePathBuf);
		if (path.empty())
		{
			ImGui::OpenPopup("Load Error");
		}
		else
		{
			m_JsonLoader->Load_Objects(path, []() {});
		}

		if (ImGui::BeginPopup("Load Error"))
		{
			ImGui::Text("파일을 불러올 수 없습니다.\n경로 또는 포맷을 확인하세요.");
			ImGui::EndPopup();
		}
	}


	ImGui::Checkbox("NavMeshTool", &m_bIsNavMeshCreating);
	if (m_bIsNavMeshCreating)
	{
		ShowCells();
	}

	ImGui::Separator();
	ImGui::InputText("NavMesh Path", m_NavFilePathBuf, IM_ARRAYSIZE(m_NavFilePathBuf), ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("...."))  // 파일 다이얼로그 버튼
	{
		// OPENFILENAME 구조체 초기화
		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetActiveWindow();           // ImGui 창의 HWND를 넘겨주세요
		ofn.lpstrFilter = "DataFile\0*.dat\0All Files\0*.*\0";
		ofn.lpstrFile = m_NavFilePathBuf;               // 선택된 파일 경로 버퍼
		ofn.nMaxFile = sizeof(m_NavFilePathBuf);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		// 열기 대화상자 표시
		if (GetOpenFileNameA(&ofn))
		{

		}
	}
	if (ImGui::Button("Save NavMesh"))
	{
		if (m_NavFilePathBuf[0] == '\0')
		{
			ImGui::OpenPopup("Save NavMesh Error");
		}
		else
		{
			m_pNavigation->SaveCells(StringToWString(m_NavFilePathBuf).c_str());
		}
	}
	if (ImGui::Button("Load NavMesh"))
	{
		if (m_NavFilePathBuf[0] == '\0')
		{
			ImGui::OpenPopup("Load NavMesh Error");
		}
		else
		{
			m_pNavigation->LoadCells(StringToWString(m_NavFilePathBuf).c_str());
		}
	}



	ImGui::End();
}

void CToolbar::RegisterDefaultPrototypes()
{
	m_PrototypeSet.insert("Canvas");
	m_PrototypeSet.insert("Button");
	m_PrototypeSet.insert("Image");
	m_PrototypeSet.insert("Bar");
}

void CToolbar::SetLevelEnumToString()
{
	m_LevelStringMap["Static"] = 0;
	m_LevelStringMap["Logo"] = 2;
	m_LevelStringMap["GamePlay"] = 3;
	m_LevelStringMap["EnmuBoss"] = 4;
	m_LevelStringMap["ModeSelect"] = 6;
	m_LevelStringMap["Loading"] = 1;
	m_LevelStringMap["Battle"] = 7;
}

void CToolbar::UpdatePrototypeList()
{
	m_PrototypeSet.clear();
	RegisterDefaultPrototypes();

	auto pProtoList = m_pPrototypes[m_iCurrentSelectedLevel];
	if (pProtoList.empty())
		return;

	for (auto& pProto : pProtoList)
	{
		if (dynamic_cast<CGameObject*>(pProto.second) == nullptr)
			continue;
		string name = WStringToString(pProto.first);
		m_PrototypeSet.insert(name);
	}
}

void CToolbar::Get_PrototypeList()
{
	m_pPrototypes.reserve(ToIndex(LEVEL::END));
	for (_uint i = 0; i < ToIndex(LEVEL::END); ++i)
	{
		auto prototypes = m_pGameInstance->Get_Prototypes(i);
		if (prototypes)
		{
			m_pPrototypes.push_back(*prototypes);
		}
		else
		{
			m_pPrototypes.push_back({});
		}
	}
}

void CToolbar::FBXLodaer()
{
	ImGui::Begin("Loader");

	static _bool isAnimation = false;

	vector<wchar_t> buffer(8192);

	if (ImGui::Button("Add FBX Files")) {
		OPENFILENAMEW ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetActiveWindow();
		ofn.lpstrFilter = L"FBX Files\0*.fbx\0All Files\0*.*\0";
		ofn.lpstrFile = buffer.data();
		ofn.nMaxFile = static_cast<DWORD>(buffer.size());
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		if (GetOpenFileNameW(&ofn)) {
			m_FbxFilePaths.clear();
			wchar_t* ptr = buffer.data();
			std::wstring dir = ptr;
			ptr += dir.size() + 1;
			if (*ptr == L'\0') {
				// 단일 파일 선택
				m_FbxFilePaths.push_back(dir);
			}
			else {
				// 다중 파일 선택
				while (*ptr) {
					std::wstring file = ptr;
					ptr += file.size() + 1;
					m_FbxFilePaths.push_back(dir + L"\\" + file);
				}
			}
		}
	}

	// 선택된 FBX 목록 표시
	if (ImGui::CollapsingHeader("Selected FBX Files")) {
		for (const auto& path : m_FbxFilePaths) {
			// 간단히 UTF-16을 ANSI로 변환하여 출력
			string utf8 = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(path);
			ImGui::TextUnformatted(utf8.c_str());
		}
	}

	// 애니메이션 체크박스

	ImGui::Checkbox("Animation", &isAnimation);

	// FBX 로드 버튼
	if (ImGui::Button("Load FBX Files"))
	{
		for (const auto& path : m_FbxFilePaths)
		{
			string pathStr = WStringToString(path);
			CModel::Create(m_pDevice, m_pContext, isAnimation ? MODEL::ANIM : MODEL::NONANIM, pathStr.c_str());
		}
	}

	ImGui::End();
}

void CToolbar::DrawAnimEventEditor()
{

	static _bool  isPlaying = false;
	static _int   selectedAnim = 0;
	static _float playTime = 0.f;
	static _int   selectedListenerIdx = 0;

	// 선택 오브젝트/모델/애니메이터 체크
	if (!CEditorManager::m_pSelectedObject) return;
	auto pModel = static_cast<CModel*>(
		CEditorManager::m_pSelectedObject
		->Get_Component(TEXT("Com_Model"))
		);
	auto pAnimator = dynamic_cast<CAnimator*>(
		CEditorManager::m_pSelectedObject
		->Get_Component(TEXT("Com_Animator"))
		);
	if (!pModel || !pAnimator) return;

	auto animations = pModel->GetAnimations();
	int animCount = (int)animations.size();
	if (animCount == 0) return;

	ImGui::Begin("Animation Event Editor");

	// 1) 애니메이션 선택 재생 
	// 콤보박스
	vector<const char*> animNames(animCount);
	for (_int i = 0; i < animCount; ++i)
		animNames[i] = animations[i]->Get_Name();
	_bool selected = ImGui::Combo("Animation", &selectedAnim, animNames.data(), animCount);

	if (selected)
	{
		// 애니메이션 선택 시 재생 시간 초기화
		playTime = 0.f;
		if (isPlaying)
		{
			pAnimator->PlayClip(animations[selectedAnim]);
		}
	}
	// Play / Stop
	ImGui::SameLine();
	if (ImGui::Button(isPlaying ? "Stop" : "Play"))
	{
		isPlaying = !isPlaying;
		if (isPlaying)      pAnimator->PlayClip(animations[selectedAnim]);
		else                pAnimator->StopAnimation();
	}



	// 동기화된 재생 시간
	CAnimation* anim = animations[selectedAnim];
	float duration = anim->GetDuration();
	playTime = anim->GetCurrentTrackPosition();

	ImGui::Text("Play Time: %.2f / %.2f", playTime, duration);
	if (ImGui::SliderFloat("Time", &playTime, 0.f, duration))
	{
		// 슬라이더로 타임 직접 세팅
		anim->SetCurrentTrackPosition(playTime);
	}
	ImGui::Separator();

	//  2) 애니 이벤트 편집
	if (ImGui::Button("Add Manual Event"))
		anim->AddEvent({ playTime, "NewEvent" });

	auto& events = anim->GetEvents();
	for (_int i = 0; i < (_int)events.size(); ++i)
	{
		auto& ev = events[i];
		ImGui::PushID(i);
		char buf[64];
		strncpy_s(buf, ev.name.c_str(), sizeof(buf));
		if (ImGui::InputText("Name", buf, sizeof(buf)))
			ev.name = buf;
		ImGui::Separator();
		if (ImGui::DragFloat("Time", &ev.fTime, 0.01f, 0.f, duration, "%.2f"))
			ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			events.erase(events.begin() + i);
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
		ImGui::Separator();
	}

	const auto& listeners = pAnimator->GetEventListeners();
	vector<const char*> listenerNames;
	listenerNames.reserve(listeners.size());
	for (auto& kv : listeners)
		listenerNames.push_back(kv.first.c_str());

	if (!listenerNames.empty())
	{
		ImGui::Text("Available Animator Events:");
		ImGui::Combo("##listener_combo", &selectedListenerIdx,
			listenerNames.data(), (int)listenerNames.size());
		ImGui::SameLine();
		if (ImGui::Button("Assign To Anim"))
		{
			anim->AddEvent({ playTime, listenerNames[selectedListenerIdx] });
		}
		ImGui::Separator();
	}

	if (ImGui::Button("Save All Clips Events to JSON"))
	{
		json root;
		// 선택된 오브젝트 이름
		string objName = WStringToString(CEditorManager::m_pSelectedObject->Get_Name());
		root["object"] = objName;

		// 모든 애니메이션 순회
		json animArray = json::array();
		for (auto* clip : animations)
		{
			json jclip;
			string clipName = clip->Get_Name();
			jclip["clipName"] = clipName;
			jclip["duration"] = clip->GetDuration();

			// 이벤트 배열
			json evArray = json::array();
			for (auto& ev : clip->GetEvents())
			{
				evArray.push_back({
					{"time", ev.fTime},
					{"name", ev.name}
					});
			}
			jclip["events"] = move(evArray);
			animArray.push_back(move(jclip));
		}
		root["animations"] = move(animArray);

		// 파일 경로: ../Asset/Json/ObjectName_events.json
		string path = std::string("../Asset/Json/") + objName + "_events.json";
		ofstream ofs(path);
		ofs << root.dump(4);
	}

	ImGui::SameLine();
	if (ImGui::Button("Load All Clips Events from JSON"))
	{
		string objName = WStringToString(CEditorManager::m_pSelectedObject->Get_Name());
		string path = string("../Asset/Json/") + objName + "_events.json";

		json root;
		ifstream ifs(path);
		if (ifs.is_open()) {
			ifs >> root;
			// "animations" 배열 순회
			for (auto& jclip : root["animations"])
			{
				string clipName = jclip["clipName"];
				// 해당 이름의 CAnimation* 찾기
				for (auto* clip : animations)
				{
					if (clip->Get_Name() == clipName)
					{
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
	}
	ImGui::End();
}

void CToolbar::SpawnMouse(void* pArg)
{
	ImVec2 winPos = ImGui::GetItemRectMin();
	ImVec2 mpos = ImGui::GetMousePos();
	float mx = mpos.x - winPos.x;
	float my = mpos.y - winPos.y;
	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);
	XMVECTOR worldPos = m_pGameInstance->UnprojectToGround(mx, my, ViewportDesc);

	// 3) 일정 거리 이상 이동했을 때만 생성 (너무 빡빡하면 성능 저하)
	static XMVECTOR lastPos = XMVectorSet(FLT_MAX, 0, 0, 0);
	if (XMVectorGetX(XMVector3Length(worldPos - lastPos)) < 0.2f)
		return;
	lastPos = worldPos;

	// 4) 프로토타입 복제 & 위치 설정
	CGameObject* obj = ClonePrototype(m_CurrentPrototype, L"", pArg);
	if (!obj) return;
	// TransformComponent 가져와서 위치 직접 세팅
	auto pTrans = obj->GetTransform();
	if (pTrans)
		pTrans->Set_State(STATE::POSITION,worldPos);
	CEditorManager::m_vecSceneObjects.push_back(obj);
}


void CToolbar::ShowCells()
{
	if (m_pNavigation && m_bIsNavMeshCreating)
	{
		static const auto& cells = m_pNavigation->GetCells();  // 셀 리스트 (const ref)
		static int selectedCellIndex = -1;

		ImGui::Begin("Cell List");

		for (int i = 0; i < static_cast<int>(cells.size()); ++i)
		{
			char label[32];
			sprintf_s(label, "Cell %d", i);

			if (ImGui::Selectable(label, selectedCellIndex == i))
			{
				selectedCellIndex = i;
				m_pNavigation->SetIndex(i);  // 셀 인덱스 전달
			}
		}
		ImGui::Separator();

		// 선택 초기화 버튼
		if (ImGui::Button("Reset Selection"))
		{
			selectedCellIndex = -1;
			m_pNavigation->SetIndex(-1); // 선택 해제
		}

		// 선택된 셀 삭제 버튼
		if (selectedCellIndex != -1)
		{
			ImGui::SameLine();
			if (ImGui::Button("Delete Selected"))
			{
				m_pNavigation->DeleteCell(selectedCellIndex); // 셀 삭제
				selectedCellIndex = -1;
				m_pNavigation->SetIndex(-1); // 선택 해제
			}
		}

	}
	ImGui::End();
}

void CToolbar::DrawParticleEditor()
{
	if (!ImGui::Begin("Particle Editor"))
		return;

	static CParticleSystem::PARTICLE_DESC desc{};

	int iInstance = static_cast<_int>(desc.iNumInstance);
	if (ImGui::InputInt("Num Instances", &iInstance))
		desc.iNumInstance = static_cast<_uint>(max(iInstance, 0)); // 음수 방지
	ImGui::DragFloat3("Center", &desc.vCenter.x, 0.1f);
	ImGui::DragFloat3("Range", &desc.vRange.x, 0.1f);
	ImGui::DragFloat2("Size", &desc.vSize.x, 0.01f);
	ImGui::DragFloat2("Lifetime", &desc.vLifeTime.x, 0.1f);
	ImGui::DragFloat2("Speed", &desc.vSpeed.x, 0.1f);
	ImGui::Checkbox("Loop", &desc.isLoop);
	ImGui::DragFloat3("Velocity", &desc.vVelocity.x, 0.1f);
	ImGui::DragFloat("Spread Angle", &desc.fSpreadAngle, 1.0f);
	ImGui::DragFloat("Gravity", &desc.fGravity, 0.01f);
	ImGui::ColorEdit3("Start Color", &desc.vStartColor.x);
	ImGui::ColorEdit3("End Color", &desc.vEndColor.x);
	ImGui::DragFloat("AlphaVariation", &desc.fAlphaVariation, 0.01f, 0.f, 1.f);

	if (ImGui::Button("Create Particle"))
	{
		Safe_Release(m_pParticleSystem);
		m_pParticleSystem = CParticleSystem::Create(
			m_pDevice, m_pContext, desc);
		if (m_pParticleSystem)
		{
			m_pParticleSystem->Initialize(nullptr);
		}
	}

	ImGui::End();
}

CGameObject* CToolbar::ClonePrototype(const string& prototypeName, const wstring& instanceName, void* pArg)
{
	auto& protoMap = m_pPrototypes[m_iCurrentSelectedLevel];
	auto it = protoMap.find(StringToWString(prototypeName));
	if (it == protoMap.end())
		return nullptr;

	if (dynamic_cast<CEnvironment*>(it->second))
	{
		CEnvironment::ENVIRONMENT_DESC envDesc;
		envDesc.strModelTag = StringToWString(m_ModelKey);
		envDesc.strName = instanceName;
		CGameObject* pClone = m_pGameInstance->Add_GameObject(
			m_iCurrentSelectedLevel,
			StringToWString(prototypeName),
			m_iCurrentSelectedLevel,
			instanceName,
			&envDesc
		);
		return pClone;
	}

	CGameObject* pClone = m_pGameInstance->Add_GameObject(
		m_iCurrentSelectedLevel,
		StringToWString(prototypeName),
		m_iCurrentSelectedLevel,
		instanceName,
		pArg ? pArg : nullptr
	);
	return pClone;

}

void CToolbar::CreatePoints(const _float3& worldPos)
{
	if (!m_bIsNavMeshCreating)
		return;



	m_NavMeshPoints.push_back(worldPos);

	if (m_NavMeshPoints.size() % 3 == 0)
	{
		//// 최초 삼각형일 때만 전체 삭제
		//if (m_NavMeshPoints.size() == 3)
		//	m_NavMeshPoints->ClearCells();

		// 바로 직전 3점으로만 Tri 만들기
		size_t i = m_NavMeshPoints.size() - 3;
		_float3 a = m_NavMeshPoints[i + 0];
		_float3 b = m_NavMeshPoints[i + 1];
		_float3 c = m_NavMeshPoints[i + 2];

		// 시계방향 보정
		if (CrossZ(a, b, c) > 0) 
			swap(b, c);

		_float3 vPts[3] = { a, b, c };
		CCell* pCell = CCell::Create(
			m_pDevice, m_pContext,
			vPts,
			m_pNavigation->GetCellCount()
		);
		if (pCell)
			m_pNavigation->AddCell(pCell);
		m_NavMeshPoints.clear();
	}
}

void CToolbar::DeletePoints(const _float3& worldPos)
{
	if (m_pNavigation&&m_bIsNavMeshCreating)
	{
		_vector vPos = XMLoadFloat3(&worldPos);
		m_pNavigation->DeleteCell(vPos);
	}
}

CToolbar* CToolbar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolbar* pInstance = new CToolbar(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CToolbar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CToolbar::Free()
{
	__super::Free();
	m_PrototypeSet.clear();
	m_ShaderKeys.clear();
	m_TextureKeys.clear();
	m_LevelStringMap.clear();
	m_CurrentPrototype = "";
	Safe_Release(m_JsonLoader);
	Safe_Release(m_pNavigation);
	Safe_Release(m_pPreviewShader);
	Safe_Release(m_pPreviewTexture);
	Safe_Release(m_pParticleSystem);
}
