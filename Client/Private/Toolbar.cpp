#include "Toolbar.h"
#include <UICanvas.h>
#include <UIButton.h>
#include <UIImage.h>
#include <UIProgressBar.h>
#include <GameInstance.h>
#include <EditorManager.h>
#include <BaseCharacter.h>

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
    return S_OK;
}

void CToolbar::Update(_float fTimeDelta)
{
}

HRESULT CToolbar::Render()
{
	DrawToolbar();
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

        ImGui::Separator();
    }
    else
    {
        ImGui::InputText("Instance Name", m_NameBuf, IM_ARRAYSIZE(m_NameBuf));
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
            if (m_CurrentPrototype == "Canvas") obj = CUICanvas::Create(m_pDevice, m_pContext);
            else if (m_CurrentPrototype == "Button") obj = CUIButton::Create(m_pDevice, m_pContext);
            else if (m_CurrentPrototype == "Image")  obj = CUIImage::Create(m_pDevice, m_pContext);
            else if (m_CurrentPrototype == "Bar")    obj = CUIProgressBar::Create(m_pDevice, m_pContext);
            if (obj) obj->Initialize(&uiDesc);
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
            if (auto pCanvas = dynamic_cast<CUICanvas*>(obj))
            {
                m_pGameInstance->AddCanvasUI(pCanvas);
            }
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

CGameObject* CToolbar::ClonePrototype(const string& prototypeName, const wstring& instanceName)
{
    auto& protoMap = m_pPrototypes[m_iCurrentSelectedLevel];
    auto it = protoMap.find(StringToWString(prototypeName));
    if (it == protoMap.end())
        return nullptr;
    CGameObject* pClone = m_pGameInstance->Add_GameObject(
        m_iCurrentSelectedLevel,
        StringToWString(prototypeName),
        m_iCurrentSelectedLevel,
        instanceName
    );
    return pClone;

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
}
