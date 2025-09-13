#include "Toolbar.h"
#include "AnimController.h"
#include "CutSceneCamera.h"
#include "ParticleSystem.h"
#include "ParticleEffect.h"
#include "UIProgressBar.h"
#include "BaseCharacter.h"
#include "EditorManager.h"
#include "GameInstance.h"
#include "Environment.h"
#include "FreeCamera.h"
#include "Navigation.h"
#include "Animation.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "Animator.h"
#include "Texture.h"
#include "UIImage.h"
#include "Shader.h"
#include "Model.h"
#include "Cell.h"


CToolbar::CToolbar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel(pDevice, pContext)
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


	// 파티클용 임시 쉐이더와 텍스쳐 초기화
	m_pPreviewShader = m_pGameInstance->GetShader(TEXT("Shader_VtxRectInstance"), true);
	m_pPreviewTexture = m_pGameInstance->GetTexture(TEXT("TitleLogo"), true);
	
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};
	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	// 파티클용 렌더 타겟 초기화
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Particle"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 255.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_MultiParticle"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 255.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Tools"), TEXT("Target_Particle"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EffectTools"), TEXT("Target_MultiParticle"))))
		return E_FAIL;
	m_pEffectPreviewSRV = m_pGameInstance->Get_RenderTargetSRV(TEXT("Target_Particle"));
	m_pMultiEffectPreviewSRV = m_pGameInstance->Get_RenderTargetSRV(TEXT("Target_MultiParticle"));



	// 컷씬 카메라용 초기화
	if (m_pCutSceneCamera == nullptr)
	{
		m_pCutSceneCamera = CCutSceneCamera::Create(m_pDevice, m_pContext);
		if (m_pCutSceneCamera == nullptr)
			return E_FAIL;
		m_pCutSceneCamera->Initialize();
	}

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_CutScene"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 255.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_CutScene"), TEXT("Target_CutScene"))))
		return E_FAIL;
	m_pCutSceneCameraSRV = m_pGameInstance->Get_RenderTargetSRV(TEXT("Target_CutScene"));

	if (m_pFreeCamera == nullptr)
	{
		m_pFreeCamera = static_cast<CFreeCamera*>(m_pGameInstance->Find_GameObjectByName(ToIndex(LEVEL::STATIC), TEXT("FreeCamera")));
	}

	m_pMySequence = new CMySequence(m_vecSequenceItems,this);

	return S_OK;
}

void CToolbar::Update(_float fTimeDelta)
{
	if (m_pParticleSystem&& m_pParticleSystem->IsActive())
	{
		if (FAILED(m_pParticleSystem->UpdateVertexInstances(fTimeDelta)))
			return;
	}

	if (m_pParticleEffect)
	{
		m_pParticleEffect->Update(fTimeDelta);
	}
	if (m_bIsCutSceneCameraActive)
	{
		if (m_pFreeCamera&&m_pFreeCamera->IsActive())
			m_pFreeCamera->SetActive(false);
		m_pCutSceneCamera->SetMainpulate(true);
		EditCutSceneCamera();
		RenderCutScene(fTimeDelta);
		m_pCutSceneCamera->Late_Update(fTimeDelta);
	}
	else
	{
		if (m_pFreeCamera && !m_pFreeCamera->IsActive())
			m_pFreeCamera->SetActive(true);
		m_pCutSceneCamera->SetMainpulate(false);
	}

	for (auto& item : m_vecSequenceItems)
	{
		if (item.pPS)
		{
			item.pPS->UpdateVertexInstances(fTimeDelta);
		}
	}
}

// 렌더
HRESULT CToolbar::Render()
{
	DrawToolbar();
	FBXLoader();
	DrawAnimEventEditor();
	ParticleEditor();
	if (m_pNavigation)
		m_pNavigation->Render();
	DrawParticlePreview();

	if (m_pParticleEffect)
	{
		if (FAILED(m_pParticleEffect->Render()))
			return E_FAIL;
	}
	return S_OK;
}

void CToolbar::DrawToolbar()
{
	ImGui::Begin("Toolbar");
	ImGui::Checkbox("CutScene Camera", &m_bIsCutSceneCameraActive);
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
			_wstring wName = m_NameBuf[0]
				? StringToWString(m_NameBuf)
				: StringToWString(m_CurrentPrototype);
		
			obj = ClonePrototype(m_CurrentPrototype, wName);
			// 생성 후 입력란 초기화
			m_NameBuf[0] = '\0';
		}

		// 레벨에 따라 객체를 추가
		if (obj)
		{
			CEditorManager::m_vecSceneObjects.push_back(obj);
		}
		
	}

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

void CToolbar::FBXLoader()
{
	ImGui::Begin("Loader");

	static _bool isAnimation = false;

	vector<wchar_t> buffer(8192);

	if (ImGui::Button("Add FBX Files")) 
	{
		OPENFILENAMEW ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetActiveWindow();
		ofn.lpstrFilter = L"FBX Files\0*.fbx\0All Files\0*.*\0";
		ofn.lpstrFile = buffer.data();
		ofn.nMaxFile = static_cast<DWORD>(buffer.size());
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		if (GetOpenFileNameW(&ofn)) 
		{
			m_FbxFilePaths.clear();
			wchar_t* ptr = buffer.data();
			_wstring dir = ptr;
			ptr += dir.size() + 1;
			if (*ptr == L'\0') 
			{
				// 단일 파일 선택
				m_FbxFilePaths.push_back(dir);
			}
			else
			{
				// 다중 파일 선택
				while (*ptr) 
				{
					_wstring file = ptr;
					ptr += file.size() + 1;
					m_FbxFilePaths.push_back(dir + L"\\" + file);
				}
			}
		}
	}

	// 선택된 FBX 목록 표시
	if (ImGui::CollapsingHeader("Selected FBX Files")) 
	{
		for (const auto& path : m_FbxFilePaths)
		{
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
			_matrix		PreTransformMatrix = XMMatrixIdentity();

			XMVECTOR q = XMQuaternionRotationRollPitchYaw(
				XMConvertToRadians(0.f),
				XMConvertToRadians(90.f),
				XMConvertToRadians(0.f)
			);
			//// 행렬로 변환
			PreTransformMatrix = XMMatrixRotationQuaternion(q);
		//	PreTransformMatrix = XMMatrixRotationY(XMConvertToRadians(180.f));
		//	PreTransformMatrix = XMMatrixRotationX(XMConvertToRadians(90.f));
			CModel::Create(m_pDevice, m_pContext, isAnimation ? MODEL::ANIM : MODEL::NONANIM, pathStr.c_str(), PreTransformMatrix);
		}
	}

	ImGui::End();
}

void CToolbar::DrawAnimEventEditor()
{

	static _bool  isPlaying = false;
	static _int   selectedAnim = 0;
	static _int   selectedListenerIdx = 0;
	static _float playTime = 0.f;

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

	// 애니메이션 선택 재생 
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


	CAnimation* anim = animations[selectedAnim];
	_float duration = anim->GetDuration();
	playTime = anim->GetCurrentTrackPosition();

	ImGui::Text("Play Time: %.2f / %.2f", playTime, duration);
	if (ImGui::SliderFloat("Time", &playTime, 0.f, duration))
	{
		anim->SetCurrentTrackPosition(playTime);
	}
	ImGui::Separator();

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
		string path = string("../Asset/Json/") + objName + "_events.json";
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
			for (auto& jclip : root["animations"])
			{
				string clipName = jclip["clipName"];
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

void CToolbar::ShowCells()
{
	if (m_pNavigation && m_bIsNavMeshCreating)
	{
		static const auto& cells = m_pNavigation->GetCells();  // 셀 리스트
		static _int selectedCellIndex = -1;

		ImGui::Begin("Cell List");

		for (_int i = 0; i < static_cast<_int>(cells.size()); ++i)
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

void CToolbar::ParticleEditor()
{
	if (!ImGui::Begin("Particle Editor"))
		return;
	static _bool bActive = true;
	_bool isShaderKeySet = false;
	_bool isTextureKeySet = false;
	_bool isChangeValue = false;
	_bool bPlayAwakeChange = false;
	_bool bChangeActive = false;
	
	string curShader = WStringToString(m_ShaderKey);
	if (isShaderKeySet = ImGui::BeginCombo("Shader Key", curShader.c_str()))
	{
		for (size_t i = 0; i < m_ShaderKeys.size(); ++i)
		{
			// 벡터에서 꺼낸 wstring을 string으로 변환
			string key = WStringToString(m_ShaderKeys[i]);
			_bool selected = (m_ShaderKey == m_ShaderKeys[i]);
			if (ImGui::Selectable(key.c_str(), selected))
			{
				m_ShaderKey = m_ShaderKeys[i];  // 선택 시 wstring으로 저장
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	string curTex = WStringToString(m_TextureKey);
	if (isTextureKeySet = ImGui::BeginCombo("Texture Key", curTex.c_str()))
	{
		for (size_t i = 0; i < m_TextureKeys.size(); ++i)
		{
			string key = WStringToString(m_TextureKeys[i]);
			_bool selected = (m_TextureKey == m_TextureKeys[i]);
			if (ImGui::Selectable(key.c_str(), selected))
			{
				m_TextureKey = m_TextureKeys[i];
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::InputInt("Texture Index", &m_iTextureIndex);
	m_iTextureIndex = min(m_iTextureIndex, static_cast<_int>(m_iMaxTextureCount));
	if (m_iTextureIndex < 0)
	{
		m_iTextureIndex = 0; // 음수 방지
	}
	ImGui::InputInt("Shader Pass", &m_iShaderPass);
	if (m_iShaderPass < 0)
	{
		m_iShaderPass = 0; // 음수 방지
	}

	if (ImGui::Button("Play"))
	{
		if (m_pParticleSystem)
		{
			m_pParticleSystem->PlayParticle();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		if (m_pParticleSystem)
		{
			m_pParticleSystem->StopParticle();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		if (m_pParticleSystem)
		{
			m_pParticleSystem->ResetParticle();
		}
	}


	static CParticleSystem::PARTICLE_DESC desc{};

	const char* particleTypes[] = { "POINT","RECT" };
	_int iType = static_cast<_int>(desc.eParticleType);
	if (isChangeValue |= ImGui::Combo("Particle Type", &iType, particleTypes, IM_ARRAYSIZE(particleTypes)))
	{
		desc.eParticleType = static_cast<PARTICLE_TYPE>(iType);
		if (isChangeValue)
		{
			isShaderKeySet = true;  // 셰이더 변경
		}
		if (desc.eParticleType == PARTICLE_TYPE::POINT)
		{
			m_bIsPointInstance = true;  // 포인트 인스턴스 활성화
			m_ShaderKey = TEXT("Shader_VtxPointInstance");
		}
		else if (desc.eParticleType == PARTICLE_TYPE::RECT)
		{
			m_bIsPointInstance = false; // 포인트 인스턴스 비활성화
			m_ShaderKey = TEXT("Shader_VtxRectInstance");
		}
	}
	bChangeActive = ImGui::Checkbox("Particle Active", &bActive);
	if (m_pParticleSystem&&bChangeActive)
	{
		m_pParticleSystem->SetActive(bActive);
	}
	_int iInstance = static_cast<_int>(desc.iNumInstance);
	if (isChangeValue |= ImGui::DragInt("Num Instances", &iInstance,1))
		desc.iNumInstance = static_cast<_uint>(max(iInstance, 0)); // 음수 방지
	bPlayAwakeChange = ImGui::Checkbox("PlayAwake", &desc.bPlayAwake);
	isChangeValue |= ImGui::Checkbox("Loop", &desc.isLoop);
	isChangeValue |= ImGui::Checkbox("Use 3D Size", &desc.b3DSize);
	isChangeValue |= ImGui::Checkbox("Use 3D Rotation", &desc.b3DRotation);
	isChangeValue |= ImGui::DragFloat3("Center", &desc.vCenter.x, 0.1f);
	isChangeValue |= ImGui::DragFloat3("Range", &desc.vRange.x, 0.1f);
	if (desc.b3DSize)
	{
		isChangeValue |= ImGui::DragFloat3("3D Size", &desc.v3DSize.x, 0.01f);
	}
	else
	{
		isChangeValue |= ImGui::DragFloat2("Size", &desc.vSize.x, 0.01f);
	}
	isChangeValue |= ImGui::DragFloat2("Lifetime", &desc.vLifeTime.x, 0.1f);
	isChangeValue |= ImGui::DragFloat2("Speed", &desc.vSpeed.x, 0.1f);
	isChangeValue |= ImGui::DragFloat3("Velocity", &desc.vVelocity.x, 0.1f);
	if (desc.b3DRotation)
	{
		isChangeValue |= ImGui::DragFloat3("3D Rotation", &desc.v3DRotation.x, 0.01f);
	}
	isChangeValue |= ImGui::DragFloat("Spread Angle", &desc.fSpreadAngle, 1.0f);
	isChangeValue |= ImGui::DragFloat("Gravity", &desc.fGravity, 0.01f);
	isChangeValue |= ImGui::ColorEdit3("Start Color", &desc.vStartColor.x);
	isChangeValue |= ImGui::ColorEdit3("End Color", &desc.vEndColor.x);
	isChangeValue |= ImGui::DragFloat("AlphaVariation", &desc.fAlphaVariation, 0.01f, 0.f, 1.f);

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

	
	static string fileName;
	char buf[64];
	strncpy_s(buf, fileName.c_str(), sizeof(buf));
	if (ImGui::InputText("Particle File Name", buf, sizeof(buf)))
		fileName = buf;
	if (ImGui::Button("Save Particle JSON"))
	{
		
		if (m_pParticleSystem)
		{
		string path = string("../Asset/Json/Particle/") + fileName + "_Particle.json";

		if (FAILED(m_JsonLoader->Save_Particle(path, m_pParticleSystem)))
		{
			ImGui::OpenPopup("Save Particle Error");
		}
		else
		{
			ImGui::OpenPopup("Save Particle Success");
		}
		}
	}

	ImGui::Separator();
	ImGui::InputText("Particle Path", m_ParticleFilePathBuf, IM_ARRAYSIZE(m_ParticleFilePathBuf), ImGuiInputTextFlags_ReadOnly);
	ImGui::Separator();
	if (ImGui::Button("Load Particle Path"))  // 파일 다이얼로그 버튼
	{
		// OPENFILENAME 구조체 초기화
		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetActiveWindow();         
		ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
		ofn.lpstrFile = m_ParticleFilePathBuf;               // 선택된 파일 경로 버퍼
		ofn.nMaxFile = sizeof(m_ParticleFilePathBuf);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

		if (GetOpenFileNameA(&ofn))
		{
			// m_ParticleFilePathBuf가 선택된 파일 경로로 업데이트
		}
	}

	ImGui::Separator();
	if (ImGui::Button("Load Particle JSON"))
	{
		string path(m_ParticleFilePathBuf);
		if (path.empty())
		{
			ImGui::OpenPopup("Load Particle Error");
		}
		else
		{
			Safe_Release(m_pParticleSystem);  // 기존 파티클 시스템 삭제
			if (FAILED(m_JsonLoader->Load_Particle(path, &m_pParticleSystem)))
			{
				ImGui::OpenPopup("Load Particle Error");
			}
			else
			{
				switch (m_pParticleSystem->GetParticleType())
				{
				case PARTICLE_TYPE::POINT:
				{
					m_ShaderKey = TEXT("Shader_VtxPointInstance");
					m_bIsPointInstance = true;  // 포인트 인스턴스 활성화
				}
					break;
				case PARTICLE_TYPE::RECT:
				{
					m_ShaderKey = TEXT("Shader_VtxRectInstance");
					m_bIsPointInstance = false; // 포인트 인스턴스 비활성화
				}
					break;
				}
				isShaderKeySet = true;  // 셰이더 변경
				desc = m_pParticleSystem->GetParticleDesc();
				ImGui::OpenPopup("Load Particle Success");
			}
		}
		if (ImGui::BeginPopup("Load Particle Error"))
		{
			ImGui::Text("파일을 불러올 수 없습니다.\n경로 또는 포맷을 확인하세요.");
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Load Particle Success"))
		{
			ImGui::Text("파티클 로드 성공!");
			ImGui::EndPopup();
		}
	}

	if (isChangeValue && m_pParticleSystem)
	{
		m_pParticleSystem->ResetDesc(desc);  // 기존 파티클 시스템의 설정을 변경
		if (m_pParticleSystem)
			m_pParticleSystem->Initialize(nullptr);  // GPU 버퍼 생성
	}

	if (bPlayAwakeChange)
	{
		if (m_pParticleSystem)
		{
			m_pParticleSystem->SetPlayAwake(desc.bPlayAwake);
		}
	}

	if (isTextureKeySet)
	{
		Safe_Release(m_pPreviewTexture);
		m_pPreviewTexture = m_pGameInstance->GetTexture(m_TextureKey, true);
		m_iMaxTextureCount = m_pPreviewTexture ? m_pPreviewTexture->Get_NumTextures() : 0;
	}
	if (isShaderKeySet)
	{
		Safe_Release(m_pPreviewShader);
		m_pPreviewShader = m_pGameInstance->GetShader(m_ShaderKey, true);
	}

	static _bool bCreateParticleEffect = false;
	ImGui::Checkbox("Particle Effect Edtior On/Off", &bCreateParticleEffect);

	if (bCreateParticleEffect)
	{
		ParticleEffectEditor();
	}
	ImGui::End();
}


// 파티클 프리뷰 창
HRESULT CToolbar::DrawParticlePreview()
{

	static _float4x4 viewMat, projMat;
	static _float fYaw = 0.0f;
	static _float fPitch = 0.0f;
	static _float fDistance = 10.0f;

	if (m_pPreviewShader && m_pPreviewTexture)
	{
		// 카메라 방향 계산
		// 구면 좌표계로 사용해보기
		_float3 vLook = { 0.f, 0.f, 0.f };
		_float3 vEye;
		vEye.x = vLook.x + fDistance * cosf(fPitch) * sinf(fYaw);
		vEye.y = vLook.y + fDistance * sinf(fPitch);
		vEye.z = vLook.z + fDistance * cosf(fPitch) * cosf(fYaw);

		_matrix vView = XMMatrixLookAtLH(XMLoadFloat3(&vEye), XMLoadFloat3(&vLook), XMVectorSet(0.f, 1.f, 0.f, 0.f));
		_matrix vProj = XMMatrixPerspectiveLH(XMConvertToRadians(60.f), 1.0f, 1.f, 300.f);

		XMStoreFloat4x4(&viewMat, vView);
		XMStoreFloat4x4(&projMat, vProj);

		// 어차피 원점에서 프리뷰로 보기만할 거
		static _float4x4 worldMatrix = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		};

		if (FAILED(m_pPreviewShader->Bind_Matrix("g_WorldMatrix", &worldMatrix)))
			return E_FAIL;
		if (FAILED(m_pPreviewShader->Bind_Matrix("g_ViewMatrix", &viewMat)))
			return E_FAIL;
		if (FAILED(m_pPreviewShader->Bind_Matrix("g_ProjMatrix", &projMat)))
			return E_FAIL;

		if (m_bIsPointInstance)
		{
			_float4 vCamPos = _float4(vEye.x, vEye.y, vEye.z, 1.f);
			if (FAILED(m_pPreviewShader->Bind_RawValue("g_vCamPosition", &vCamPos, sizeof(_float4))))
				return E_FAIL;
		}

		// 테스트용으로 디퓨즈만 
		if (FAILED(m_pPreviewTexture->Bind_ShaderResource(m_pPreviewShader, "g_Texture", m_iTextureIndex)))
			return E_FAIL;

		if (FAILED(m_pPreviewShader->Begin(m_iShaderPass)))
			return E_FAIL;
	}

	if (m_pParticleSystem && m_pParticleSystem->IsActive())
	{
		m_pGameInstance->Begin_MRT(TEXT("MRT_Tools"));
		if (FAILED(m_pParticleSystem->Bind_Buffers()))
			return E_FAIL;
		if (FAILED(m_pParticleSystem->Render()))
			return E_FAIL;
		m_pGameInstance->End_MRT();

		if (m_pEffectPreviewSRV)
		{
			// 투명도 설정
			//ImGui::SetNextWindowBgAlpha(0.0f);
			if (ImGui::Begin("Particle Preview"))
			{
				ImVec2 vAvail = ImGui::GetContentRegionAvail();
				_float fSize = min(vAvail.x, vAvail.y);

				// 정중앙 정렬
				ImVec2 vCursorPos = ImGui::GetCursorPos();
				ImGui::SetCursorPosX(vCursorPos.x + (vAvail.x - fSize) * 0.5f);
				ImGui::SetCursorPosY(vCursorPos.y + (vAvail.y - fSize) * 0.5f);

				ImVec2 vImagePos = ImGui::GetCursorScreenPos();
				ImVec2 vImageSize(fSize, fSize);
				ImVec2 vMousePos = ImGui::GetMousePos();

				// 이미지 내부에서만 카메라 조작
				_bool bIsInImage = vMousePos.x >= vImagePos.x && vMousePos.x <= vImagePos.x + vImageSize.x &&
					vMousePos.y >= vImagePos.y && vMousePos.y <= vImagePos.y + vImageSize.y;

				if (bIsInImage)
				{
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					{
						ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
						fYaw -= delta.x * 0.005f;
						fPitch -= delta.y * 0.005f;
						ImGui::ResetMouseDragDelta();
					}

					_float fWheel = ImGui::GetIO().MouseWheel;
					if (fWheel != 0.0f)
					{
						fDistance -= fWheel * 1.0f;
						if (fDistance < 1.0f) fDistance = 1.0f;
						if (fDistance > 100.0f) fDistance = 100.0f;
					}
				}

				if (fPitch < -XM_PIDIV2 + 0.01f) 
					fPitch = -XM_PIDIV2 + 0.01f;
				if (fPitch > XM_PIDIV2 - 0.01f) 
					fPitch = XM_PIDIV2 - 0.01f;

				ImGui::Image((ImTextureID)m_pEffectPreviewSRV, vImageSize);
			}
			ImGui::End();
		}
	}

	// 멀티 이펙트용 
	MultiParticleEditorBySqeuence();

	return S_OK;
}

void CToolbar::ParticleEffectEditor()
{
	if (!ImGui::Begin("Particle Effect Editor"))
	{
		ImGui::End();
		return;
	}
	static char particleName[64] = "NewParticleEffect";
	static PARTICLE_UV vUV;
	static _uint iTextureIndex = 0;
	static _uint iShaderPass = 0;
	ImGui::InputText("Particle Effect Name", particleName, IM_ARRAYSIZE(particleName));
	ImGui::InputInt("Column Count", &vUV.iCols);
	ImGui::InputInt("Row Count", &vUV.iRows);
	ImGui::InputInt("Texture Index", reinterpret_cast<_int*>(&iTextureIndex));
	ImGui::InputInt("Shader Pass", reinterpret_cast<_int*>(&iShaderPass));
	if (ImGui::Button("Create Particle Effect"))
	{
		string particleNameStr = particleName;
		CreateParticleEffect(StringToWString(particleNameStr), vUV, iTextureIndex, iShaderPass);
	}

	// 현재 파티클 이펙트의 파티클 시스템 표시해서 대입

	if (m_pParticleEffect)
	{
		auto pCurParticleSystems = m_pParticleEffect->GetParticleSystems();
		if (pCurParticleSystems.empty() == false)
		{
			if (!ImGui::BeginListBox("##ParticleList", ImVec2(-FLT_MIN, 200)))
			{
				ImGui::EndListBox();
				ImGui::End();
				return;
			}
			for (auto& pCurParticleSystem : pCurParticleSystems)
			{
				_wstring name = pCurParticleSystem.first;
				string tmpName = WStringToString(name);
				_bool isSelected = (pCurParticleSystem.second == m_pParticleSystem);
				if (ImGui::Selectable(tmpName.c_str(), isSelected))
				{
					Safe_Release(m_pParticleSystem);
					m_pParticleSystem = pCurParticleSystem.second;
					Safe_AddRef(m_pParticleSystem);
				}
			}
			ImGui::EndListBox();
		}
	}
	else
	{
		ImGui::Text("No Particle Effect Created");
	}


	ImGui::End();
}

void CToolbar::MultiParticleEditorBySqeuence()
{
	static _bool          expanded       = true;// 트랙 확장 여부
	static _int           currentFrame   = 0;   // 커서 프레임
	static _int           firstFrame     = 0;   // 보이는 첫 프레임
	static _int           selectedEntry  = -1;  // 선택된 아이템 인덱스
	static _int playSpeed = 1;
	static const _float FRAME = 60.f; // 1초당 60프레임 기준
	ImGui::Begin("MultiEffect Editor");
	if (ImGui::Button("Add Particle System"))
	{
			
		if (m_pParticleSystem)
		{
			CMySequence::SequenceItem item;
			item.pPS = static_cast<CParticleSystem*>(m_pParticleSystem->Clone(nullptr));

			item.start = currentFrame;
			item.end = currentFrame + 10;
			item.type = 0;
			item.color = 0xFF00CCFF;
			item.name = "Effect" + to_string(m_vecSequenceItems.size());
			AddSequence(item);
		}
	}

	if (m_bIsPlaying)
	{
		if (ImGui::Button("Stop"))
		{
			m_bIsPlaying = false;
			// 모든 파티클 정지
			for (auto& it : m_vecSequenceItems) 
			{
				if (it.pPS) 
					it.pPS->StopParticle();
			}
		}
	}
	else
	{
		if (ImGui::Button(" Play"))
		{
			m_bIsPlaying = true;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		currentFrame = 0;
		m_bIsPlaying = false;
		for (auto& it : m_vecSequenceItems)
		{
			if (it.pPS) 
				it.pPS->ResetParticle();
		}
	}

	// 재생 속도 조절
	ImGui::SameLine();
	ImGui::SliderInt("Speed", &playSpeed, 1, 10);

	// 자동 재생 로직
	if (m_bIsPlaying)
	{
		static _float lastTime = 0.0f;
		_float currentTime = static_cast<_float>(ImGui::GetTime());

		if (currentTime - lastTime >= (1.0f / (60.0f * playSpeed))) // 60fps 기준
		{
			currentFrame++;
			if (currentFrame > m_pMySequence->GetFrameMax())
			{
				currentFrame = m_pMySequence->GetFrameMin();
			}
			lastTime = currentTime;
		}
	}

	
	if (ImSequencer::Sequencer(
		m_pMySequence,
		&currentFrame,
		&expanded,
		&selectedEntry,
		&firstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL|
		ImSequencer::SEQUENCER_ADD |
		ImSequencer::SEQUENCER_DEL 
	))
	{
		
	}
	for (auto& it : m_vecSequenceItems) 
	{
		if (!it.pPS) continue;
		if (currentFrame == it.start)
		{
			it.pPS->SetActive(true);
			it.pPS->PlayParticle();
		}
		if (currentFrame == it.end)
		{
			it.pPS->ResetParticle();
			it.pPS->StopParticle();
			it.pPS->SetActive(false);
		}
	}
	if(m_vecSequenceItems.empty() == false)
		DrawMultiParticlePreview();
	ImGui::End();
}

HRESULT CToolbar::DrawMultiParticlePreview()
{
	static _float4x4 viewMat, projMat;
	static _float fYaw = 0.0f;
	static _float fPitch = 0.0f;
	static _float fDistance = 10.0f;
	// 어차피 원점에서 프리뷰로 보기만할 거
	static _float4x4 worldMatrix = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	m_pGameInstance->Begin_MRT(TEXT("MRT_EffectTools"));
	for (_int i = 0; i < static_cast<_int>(m_vecSequenceItems.size()); i++)
	{
		auto pShader = m_vecMultiParticleShaders[i];
		auto pTexture = m_vecMultiParticleTextures[i];
		auto pParticle = m_vecSequenceItems[i].pPS;
		_int iTextureIndex = m_vecMultiParticleTextureIndices[i];
		_int iShaderPass = m_vecMultiParticleShaderPasses[i];

		if (pShader && pTexture)
		{
			_float3 vLook = { 0.f, 0.f, 0.f };
			_float3 vEye;
			vEye.x = vLook.x + fDistance * cosf(fPitch) * sinf(fYaw);
			vEye.y = vLook.y + fDistance * sinf(fPitch);
			vEye.z = vLook.z + fDistance * cosf(fPitch) * cosf(fYaw);

			_matrix vView = XMMatrixLookAtLH(XMLoadFloat3(&vEye), XMLoadFloat3(&vLook), XMVectorSet(0.f, 1.f, 0.f, 0.f));
			_matrix vProj = XMMatrixPerspectiveLH(XMConvertToRadians(60.f), 1.0f, 1.f, 300.f);

			XMStoreFloat4x4(&viewMat, vView);
			XMStoreFloat4x4(&projMat, vProj);

			if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &worldMatrix)))
			{
				return E_FAIL;
			}
			if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", &viewMat)))
			{
				return E_FAIL;
			}
			if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", &projMat)))
			{
				return E_FAIL;
			}

			if (m_bIsPointInstance)
			{
				_float4 vCamPos = _float4(vEye.x, vEye.y, vEye.z, 1.f);
				if (FAILED(pShader->Bind_RawValue("g_vCamPosition", &vCamPos, sizeof(_float4))))
					return E_FAIL;
			}

			if (FAILED(pTexture->Bind_ShaderResource(pShader, "g_Texture", iTextureIndex)))
				return E_FAIL;

			if (FAILED(pShader->Begin(iShaderPass)))
				return E_FAIL;
		}
		if (pParticle && pParticle->IsActive())
		{
			if (FAILED(pParticle->Bind_Buffers()))
				return E_FAIL;
			if (FAILED(pParticle->Render()))
				return E_FAIL;

		}
	}
	m_pGameInstance->End_MRT();

	if (m_pMultiEffectPreviewSRV)
	{
		if (ImGui::Begin("MultiParticle Preview"))
		{
			ImVec2 vAvail = ImGui::GetContentRegionAvail();
			_float fSize = min(vAvail.x, vAvail.y);

			// 정중앙 정렬
			ImVec2 vCursorPos = ImGui::GetCursorPos();
			ImGui::SetCursorPosX(vCursorPos.x + (vAvail.x - fSize) * 0.5f);
			ImGui::SetCursorPosY(vCursorPos.y + (vAvail.y - fSize) * 0.5f);

			ImVec2 vImagePos = ImGui::GetCursorScreenPos();
			ImVec2 vImageSize(fSize, fSize);
			ImVec2 vMousePos = ImGui::GetMousePos();

			// 이미지 내부에서만 카메라 조작
			_bool bIsInImage = vMousePos.x >= vImagePos.x && vMousePos.x <= vImagePos.x + vImageSize.x &&
				vMousePos.y >= vImagePos.y && vMousePos.y <= vImagePos.y + vImageSize.y;

			if (bIsInImage)
			{
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
					fYaw -= delta.x * 0.005f;
					fPitch -= delta.y * 0.005f;
					ImGui::ResetMouseDragDelta();
				}

				_float fWheel = ImGui::GetIO().MouseWheel;
				if (fWheel != 0.0f)
				{
					fDistance -= fWheel * 1.0f;
					if (fDistance < 1.0f) fDistance = 1.0f;
					if (fDistance > 100.0f) fDistance = 100.0f;
				}
			}

			if (fPitch < -XM_PIDIV2 + 0.01f)
				fPitch = -XM_PIDIV2 + 0.01f;
			if (fPitch > XM_PIDIV2 - 0.01f)
				fPitch = XM_PIDIV2 - 0.01f;

			ImGui::Image((ImTextureID)m_pMultiEffectPreviewSRV, vImageSize);
		}
		ImGui::End();
	}
	return S_OK;
}

void CToolbar::AddSequence(const CMySequence::SequenceItem& item)
{
	if (m_pMySequence == nullptr)
	{
			return;
	}
	Safe_AddRef(item.pPS);
	m_vecSequenceItems.push_back(item);
	m_vecMultiParticleShaderPasses.push_back(m_iShaderPass);
	m_vecMultiParticleShaders.push_back(m_pGameInstance->GetShader(m_ShaderKey,true));
	m_vecMultiParticleTextures.push_back(m_pGameInstance->GetTexture(m_TextureKey,true));
	m_vecMultiParticleTextureIndices.push_back(m_iTextureIndex);
}

void CToolbar::RemoveSequence(_int iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<_int>(m_vecSequenceItems.size()))
		return;

	m_vecMultiParticleShaderPasses.erase(
		m_vecMultiParticleShaderPasses.begin() + iIndex);
	m_vecMultiParticleShaders.erase(
		m_vecMultiParticleShaders.begin() + iIndex);
	m_vecMultiParticleTextures.erase(
		m_vecMultiParticleTextures.begin() + iIndex);
	m_vecMultiParticleTextureIndices.erase(
		m_vecMultiParticleTextureIndices.begin() + iIndex);
}

void CToolbar::CreateParticleEffect(const _wstring& particleName, PARTICLE_UV vUV, _uint iTextureIndex, _uint iShaderPass)
{
	if (m_pParticleEffect == nullptr)
	{
		m_pParticleEffect = CParticleEffect::Create(m_pDevice, m_pContext);
		if (m_pParticleEffect == nullptr)
			return;
		m_pParticleEffect->Initialize(nullptr);
	}

	m_pParticleEffect->AddParticleSystem_ForEditor(
		particleName, m_pParticleSystem, m_pPreviewTexture,vUV, iTextureIndex, iShaderPass);
	CEditorManager::m_vecSceneObjects.push_back(m_pParticleEffect);
}

void CToolbar::EditCutSceneCamera()
{
	if (!ImGui::Begin("CutScene Camera Editor"))
	{
		ImGui::End();
		return;
	}

	auto pCam = m_pCutSceneCamera;

	if (!pCam)
		return;
	_bool bLoop = pCam->IsLoop();
	if (ImGui::Checkbox("Loop", &bLoop))
		pCam->SetLoop(bLoop);

	ImGui::Separator();

	// 플레이 버튼, 리셋 버튼
	static _bool isPlaying = false;
	if (ImGui::Button(isPlaying ? "Stop" : "Play"))
	{
		isPlaying = !isPlaying;
		pCam->SetPlay(isPlaying);
		if (isPlaying)
			pCam->ResetCamera();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		pCam->ResetCamera();
		pCam->SetCurrentFrame(0);
		isPlaying = false;
	}

	if (pCam->IsPlaying())
	{
		pCam->SetMainpulate(false); // 플레이 중에는 조작 비활성화
	}
	else
	{
		pCam->SetMainpulate(true); // 플레이 중이 아닐 때 조작 활성화
	}

	// 진행 시간 
	_float fDuration = pCam->GetDuration();
	if (ImGui::InputFloat("Total Duration", &fDuration, 0.1f, 1.0f, "%.2f"))
	{
		pCam->SetDuration(fDuration);
	}

	ImGui::Separator();

	// 현재 씬 오브젝트들 
		auto& sceneObjs = CEditorManager::m_vecSceneObjects;

		// 현재 할당된 타깃 오브젝트 인덱스
		static _int selectedIdx = -1;

		static string stCurObjName = "None";
		if (ImGui::BeginCombo("Target Object", stCurObjName.c_str()))
		{
			for (_uint i = 0; i < static_cast<_uint>(sceneObjs.size()); i++)
			{
				_bool sel = (i == selectedIdx);
				string name = WStringToString(sceneObjs[i]->Get_Name());
				if (ImGui::Selectable(name.c_str(), sel))
				{
					selectedIdx = i;
					stCurObjName = name;
					pCam->SetTargetObject(sceneObjs[i]);
				}
				if (sel)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

	ImGui::Separator();

	auto kfs = pCam->GetKeyFrames();
	int frameCount = (int)kfs.size();
	ImGui::Text("%d KeyFrames", frameCount);

	// ── 타임라인 슬라이더 ──
	if (frameCount > 0)
	{
		// 현재 선택된 프레임 인덱스
		_int selFrame = pCam->GetCurrentFrame();
		// 최초 진입 시 동기화
		if (ImGui::IsWindowAppearing())
			selFrame = pCam->GetCurrentFrame();

		// 영역 확보
		const float H = 24.f;
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 avail = ImGui::GetContentRegionAvail();
		ImVec2 size = ImVec2(avail.x, H);
		ImGui::InvisibleButton("##timeline", size);
		ImDrawList* dl = ImGui::GetWindowDrawList();

		// 배경
		dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
			IM_COL32(50, 50, 50, 255));
		// 진행량
		float tnorm = frameCount > 1 ? (float)selFrame / float(frameCount - 1) : 0.f;
		dl->AddRectFilled(pos, ImVec2(pos.x + size.x * tnorm, pos.y + H),
			IM_COL32(100, 200, 100, 200));
		// 키프레임 마커
		for (int i = 0; i < frameCount; ++i)
		{
			float x = pos.x + size.x * (frameCount > 1 ? (float)i / (frameCount - 1) : 0.f);
			dl->AddLine(ImVec2(x, pos.y), ImVec2(x, pos.y + H),
				IM_COL32(200, 100, 100, 180), 2.f);
		}

		// 드래그로 인덱스 선택
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			float mx = ImGui::GetIO().MousePos.x;
			float rel = (mx - pos.x) / size.x;
			rel = ImClamp(rel, 0.f, 1.f);
			int idx = int(rel * (frameCount - 1) + 0.5f);
			if (idx != selFrame)
			{
				selFrame = idx;
				pCam->SetCurrentFrame(selFrame);
			}
		}

		// 현재 인덱스 표시
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + H + 4));
		ImGui::Text("Frame: %d / %d", selFrame, frameCount - 1);

		ImGui::Dummy(ImVec2(0, H + 20));
	}


		for (_uint i = 0; i < static_cast<_uint>(kfs.size()); i++)
		{
			auto key = kfs[i];
			ImGui::PushID(i);
			ImGui::Text("KeyFrame %d", i);

			bool changed = false;
			changed |= ImGui::DragFloat3("Position", &key.vPosition.x, 0.1f);
			changed |= ImGui::DragFloat3("OffSet", &key.vOffset.x, 0.1f);

			if (changed)
			{
				kfs[i] = key;
				pCam->SetKeyFrames(kfs, pCam->GetDuration());
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				kfs.erase(kfs.begin() + i);
				pCam->SetKeyFrames(kfs, pCam->GetDuration());
				ImGui::PopID();
				break;
			}
			ImGui::Separator();
			ImGui::PopID();
		}
	ImGui::Separator();


	if (ImGui::Button("Clear KeyFrames"))
	{
		pCam->ClearKeyFrames();
	}

	if (ImGui::Button("Capture Current"))
	{
		static CTransform* pCamTransform = pCam->GetTransform();

		_vector vectorPos = pCamTransform->Get_State(STATE::POSITION);
		_vector vectorLook = pCamTransform->Get_State(STATE::LOOK);
		_vector vectorUp = pCamTransform->Get_State(STATE::UP);

		CCutSceneCamera::CamKeyFrame vKf{};
		XMStoreFloat3(&vKf.vPosition, vectorPos);
		XMStoreFloat3(&vKf.vOffset, vectorLook);

		auto& vecKeyFrames = pCam->GetKeyFrames();
		vecKeyFrames.push_back(vKf);
	}
	static string fileName;
	char buf[64];
	strncpy_s(buf, fileName.c_str(), sizeof(buf));
	if (ImGui::InputText("CutScene File Name", buf, sizeof(buf)))
		fileName = buf;
	if (ImGui::Button("Save CutScene Camera"))
	{
		if (m_pCutSceneCamera)
		{
			string path = string("../Asset/Json/CutScene/") + fileName +".json";
			if (FAILED(m_JsonLoader->Save_CutSceneCamera(path, m_pCutSceneCamera)))
			{
				ImGui::OpenPopup("Save CutScene Error");
			}
			else
			{
				ImGui::OpenPopup("Save CutScene Success");
			}
		}
	}

	ImGui::Separator();
	ImGui::InputText("Current CutScene Path", m_CutScenePathBuf, IM_ARRAYSIZE(m_CutScenePathBuf), ImGuiInputTextFlags_ReadOnly);
	ImGui::Separator();

	if (ImGui::Button("Add CutScene Property"))
	{
		OPENFILENAMEA ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetActiveWindow();
		ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
		ofn.lpstrFile = m_CutScenePathBuf;
		ofn.nMaxFile = sizeof(m_CutScenePathBuf);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		if (GetOpenFileNameA(&ofn))
		{
			m_CutScenePropertyFilePaths.clear();
			char* ptr = m_CutScenePathBuf;
			string dir = ptr;
			ptr += dir.size() + 1;
			if (*ptr == '\0') 
			{
				string rel = MakeRelativePath(dir);
				m_CutScenePropertyFilePaths.push_back(rel);
			}
			else 
			{
				string path = dir;
				while (*ptr)
				{
					string file = ptr;
					ptr += file.size() + 1;
					string abs = path + "\\" + file;
					string rel = MakeRelativePath(abs);
					m_CutScenePropertyFilePaths.push_back(rel);
				}
			}
		}
	}
	if (!m_CutScenePropertyFilePaths.empty())
	{
		ImGui::Separator();
		ImGui::Text("Available CutScene Properties:");

		vector<const char*> items;
		items.reserve(m_CutScenePropertyFilePaths.size());
		for (auto& path : m_CutScenePropertyFilePaths)
			items.push_back(path.c_str());

		// 콤보박스
		ImGui::Combo("##CutSceneCombo", &m_iSelectedCutSceneIndex,
			items.data(), (int)items.size());

		// 유효할 때 
		if (m_iSelectedCutSceneIndex >= 0 &&
			m_iSelectedCutSceneIndex < (_int)m_CutScenePropertyFilePaths.size())
		{
			if (ImGui::Button("Load CutScene Property"))
			{
				const string& relPath =
					m_CutScenePropertyFilePaths[m_iSelectedCutSceneIndex];
				m_JsonLoader->Load_CutSceneCamera(relPath, pCam);  // 상대경로 넘겨서 로드
			}
		}
		else
		{
			ImGui::BeginDisabled();
			ImGui::Button("Load CutScene Property");
			ImGui::EndDisabled();
		}

	}
	ImGui::End();
}

void CToolbar::RenderCutScene(_float fTimeDelta)
{
	if (m_pCutSceneCamera && m_pCutSceneCameraSRV)
	{
		m_pCutSceneCamera->Update(fTimeDelta);
	
		m_pGameInstance->Begin_MRT(TEXT("MRT_CutScene"));
		for (const auto& pObj : CEditorManager::m_vecSceneObjects)
		{
			if (pObj && pObj->IsActive())
			{
				pObj->Late_Update(fTimeDelta);
			}
		}
		m_pGameInstance->End_MRT();
	}
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
	Safe_Delete(m_pMySequence);
	Safe_Release(m_JsonLoader);
	Safe_Release(m_pNavigation);
	Safe_Release(m_pPreviewShader);
	Safe_Release(m_pPreviewTexture);
	Safe_Release(m_pParticleEffect);
	Safe_Release(m_pParticleSystem);
	Safe_Release(m_pCutSceneCamera);
	Safe_Release(m_pEffectPreviewSRV);
	Safe_Release(m_pCutSceneCameraSRV);
	Safe_Release(m_pMultiEffectPreviewSRV);
}
