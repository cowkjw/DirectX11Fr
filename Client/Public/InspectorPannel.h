#pragma once
#include "Pannel.h"
#include "Inspector.h"

BEGIN_NAMESPACE(Client)
class CInspectorPannel : public CPannel, public IInspector
{
private:
	CInspectorPannel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	~CInspectorPannel() = default;
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
private:
	void DrawInspector();
	void DrawComponentList();
	void DrawAddComponentPopup();

	void DrawColliderInspector();
	void RenderPhysicsDebug();


private:
	_bool bPhysXDebug = false;
	

public:
	static CInspectorPannel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
END_NAMESPACE

