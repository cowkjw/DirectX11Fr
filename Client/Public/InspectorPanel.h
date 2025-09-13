#pragma once
#include "Panel.h"
#include "Inspector.h"

BEGIN_NAMESPACE(Client)
class CInspectorPanel : public CPanel, public IInspector
{
private:
	CInspectorPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CInspectorPanel() = default;
public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

	void TreePop() override;
	_bool TreeNode(const char* label) override;
	_bool Checkbox(const char* label, _bool* v) override;
	_bool DragFloat(const char* label, _float* v, _float speed) override;
	_bool DragFloat3(const char* label, _float* v, _float speed) override;
	_bool InputFloat(const char* label, _float* v) override;
	_bool InputFloat3(const char* label, _float* v) override;
	_bool InputInt(const char* label, int* v) override;
private:
	void DrawInspector();
	void DrawComponentList();
	void DrawAddComponentPopup();

	void DrawColliderInspector();
	void RenderPhysicsDebug();


private:
	_bool bPhysXDebug = false;
	

public:
	static CInspectorPanel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

