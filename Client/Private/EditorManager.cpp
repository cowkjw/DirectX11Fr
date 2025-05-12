#include "EditorManager.h"
#include "GameInstance.h"
#include "UIProgressBar.h"
#include "Transform.h"
#include "UICanvas.h"
#include "UIButton.h"
#include "UIImage.h"
#include "Gizmo.h"

using Gizmo = CGizmo;
Gizmo::Operation GizmoOp{ CGizmo::Operation::TRANSLATE };
CEditorManager::CEditorManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pDevice);
}


HRESULT CEditorManager::Initialize()
{
    RegisterDefaultPrototypes();
    // 첫 번째 프로토타입 선택
    if (!m_PrototypeMap.empty())
        m_CurrentPrototype = m_PrototypeMap.begin()->first;

    m_ShaderKeys =  m_pGameInstance->GetShaderKeys();
    m_TextureKeys =  m_pGameInstance->GetTextureKeys();
    SetLevelEnumToString();
    return S_OK;
}

void CEditorManager::Update(_float fTimeDelta)
{

    if (m_pGameInstance->IsKeyPressed(VK_F2) && m_pSelectedObject)
    {
        // F2 처리 로직
    }
    if (m_pSelectedObject)
    {
        if (m_pGameInstance->IsKeyPressed('W')) GizmoOp = CGizmo::Operation::TRANSLATE;
        if (m_pGameInstance->IsKeyPressed('R')) GizmoOp = CGizmo::Operation::ROTATE;
        if (m_pGameInstance->IsKeyPressed('E')) GizmoOp = CGizmo::Operation::SCALE;
    }
}

HRESULT CEditorManager::Render()
{
    DrawHierarchy();

    DrawInspector();


    DrawToolbar();

    if (m_pSelectedObject)
        CGizmo::Manipulate(
            m_pSelectedObject->GetTransform(),
            GizmoOp,
            m_bOrthoGizmo
        );

    return S_OK;
}

void CEditorManager::DrawHierarchy()
{
    ImGui::Begin("Hierarchy");

    for (auto* obj : m_vecSceneObjects)
    {
        if (!obj) continue;
        // UI 객체인지, 그리고 캔버스인지 체크
        _bool isUI = dynamic_cast<CUIObject*>(obj) != nullptr;
        _bool isCanvas = dynamic_cast<CUICanvas*>(obj) != nullptr;

        // 트리 노드 플래그 설정
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | (obj == m_pSelectedObject ? ImGuiTreeNodeFlags_Selected : 0);

        // 트리 노드 생성
        _bool open = ImGui::TreeNodeEx((void*)obj, flags, "%s",
            WStringToString(obj->Get_Name()).c_str());

        // 드래그 소스로 등록
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("DND_OBJ", &obj, sizeof(CGameObject*));
            ImGui::Text("%s",
                WStringToString(obj->Get_Name()).c_str());
            ImGui::EndDragDropSource();
        }

        // 캔버스에만 드롭 가능
        if (isCanvas && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJ"))
            {
                CGameObject* dragged = *(CGameObject**)payload->Data;
                if (dragged->GetParent() != obj)
                {
                    // 이전 부모에서 제거
                    if (auto* oldP = dragged->GetParent())
                        oldP->RemoveChild(dragged);
                    else
                        EraseFromVector(dragged);

                    // 새 부모에 추가
                    static_cast<CUICanvas*>(obj)->AddChildUI(static_cast<CUIObject*>(dragged));
                }
            }
            ImGui::EndDragDropTarget();
        }

        // 클릭 시 선택
        if (ImGui::IsItemClicked())
            m_pSelectedObject = obj;

        // 자식이 열려 있으면 재귀적으로 그리기
        if (open)
        {
            DrawChildHierarchy(obj);
            ImGui::TreePop();
        }
    }

    ImGui::End();
}

void CEditorManager::DrawChildHierarchy(CGameObject* parent)
{
    for (auto* child : parent->GetChildren())
    {
        bool isCanvas = dynamic_cast<CUICanvas*>(child) != nullptr;
        ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf
            | ImGuiTreeNodeFlags_NoTreePushOnOpen
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | (child == m_pSelectedObject ? ImGuiTreeNodeFlags_Selected : 0);

        ImGui::TreeNodeEx((void*)child, leafFlags, "%s",
            WStringToString(child->Get_Name()).c_str());

        // 드래그 소스 등록
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("DND_OBJ", &child, sizeof(CGameObject*));
            ImGui::Text("%s",
                WStringToString(child->Get_Name()).c_str());
            ImGui::EndDragDropSource();
        }

        // 캔버스인 경우 드롭 처리
        if (isCanvas && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJ"))
            {
                CGameObject* dragged = *(CGameObject**)payload->Data;
                if (dragged->GetParent() != child)
                {
                    if (auto* oldP = dragged->GetParent())
                        oldP->RemoveChild(dragged);
                    else
                        EraseFromVector(dragged);

                    child->AddChild(dragged);
                    dragged->SetParent(child);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked())
            m_pSelectedObject = child;
    }
}

void CEditorManager::DrawInspector()
{
    ImGui::Begin("Inspector");

    if (!m_pSelectedObject)
    {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }

    // 이름 편집
    char nameBuf[128];

    strcpy_s(nameBuf, WStringToString(m_pSelectedObject->Get_Name()).c_str());
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue))
        m_pSelectedObject->Set_Name(StringToWString(nameBuf));

    // Transform 편집
    CTransform* trans = m_pSelectedObject->GetTransform();
    _vector pos = trans->Get_State(STATE::POSITION);
   // XMFLOAT3 rot = trans->GetRotation();
    _float3 scl = trans->Get_Scaled();
    // 슬라이드 거리 설정
    if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f, -10000.f, 10000.f))
        trans->Set_State(STATE::POSITION, pos);
  /*  if (ImGui::InputFloat3("Rotation", reinterpret_cast<float*>(&rot)))
        trans->SetRotation(rot);*/
    if (ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&scl), 0.1f, 0.1f, 1000.f))
        trans->Scaling(scl);

    ImGui::End();
}

void CEditorManager::DrawToolbar()
{
    ImGui::Begin("Toolbar");

    ImGui::Checkbox("Orthographic Gizmo", &m_bOrthoGizmo);
    // UI 여부 판단
    _bool isUI = (m_CurrentPrototype == "Canvas" || m_CurrentPrototype == "Button" ||
        m_CurrentPrototype == "Image" || m_CurrentPrototype == "Bar");

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

        string curLevel = "Unknown";
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
                _bool selected = (uiDesc.iLevel == value);

                if (ImGui::Selectable(key.c_str(), selected))
                {
                    uiDesc.iLevel = value;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();
    }

    // 프로토타입 콤보
    if (ImGui::BeginCombo("UI Type", m_CurrentPrototype.c_str()))
    {
        for (auto& kv : m_PrototypeMap)
        {
            _bool sel = (kv.first == m_CurrentPrototype);
            if (ImGui::Selectable(kv.first.c_str(), sel))
                m_CurrentPrototype = kv.first;
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
            obj = ClonePrototype(m_CurrentPrototype);
        }
        if (obj)
        {
            m_vecSceneObjects.push_back(obj);
            if (auto pCanvas = dynamic_cast<CUICanvas*>(obj))
            {
                m_pGameInstance->AddCanvasUI(pCanvas);
            }
        }
    }

    ImGui::End();
}

void CEditorManager::EraseFromVector(CGameObject* pObj)
{
    auto it = find(m_vecSceneObjects.begin(), m_vecSceneObjects.end(), pObj);
    if (it != m_vecSceneObjects.end())
        m_vecSceneObjects.erase(it);
}

void CEditorManager::RegisterDefaultPrototypes()
{
    m_PrototypeMap["Canvas"] = CUICanvas::Create(m_pDevice, m_pContext);
    m_PrototypeMap["Button"] = CUIButton::Create(m_pDevice, m_pContext);
    m_PrototypeMap["Image"]  = CUIImage::Create(m_pDevice, m_pContext);
    m_PrototypeMap["Bar"]    = CUIProgressBar::Create(m_pDevice, m_pContext);
}

CGameObject* CEditorManager::ClonePrototype(const string& name)
{
    auto it = m_PrototypeMap.find(name);
    return (it != m_PrototypeMap.end()) ? it->second->Clone(nullptr) : nullptr;
}

void CEditorManager::SetLevelEnumToString()
{
    m_LevelStringMap["Static"] = 0;
    m_LevelStringMap["Logo"] = 2;
    m_LevelStringMap["GamePlay"] = 3;
}

CEditorManager* CEditorManager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEditorManager* pInstance = new CEditorManager(pDevice, pContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CEditorManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CEditorManager::Free()
{
    __super::Free();
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    for (auto* obj : m_vecSceneObjects)
        Safe_Release(obj);
    for (auto& kv : m_PrototypeMap)
        Safe_Release(kv.second);
}
