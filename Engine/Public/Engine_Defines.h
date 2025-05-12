#pragma once

#pragma warning (disable : 4251)

#include <d3d11.h>
#include <DirectXMath.h>
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/Keyboard.h"
#include "DirectXTK/Mouse.h"
#include "DirectXTK/VertexTypes.h"

#include "Fx11/d3dx11effect.h"
#include <d3dcompiler.h>
#include "PhysX/PxPhysicsAPI.h"
using namespace DirectX;

#include <string>
#include <map>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <shobjidl.h> // IFileOpenDialog 사용을 위한 헤더
#include <set>
#include <codecvt> // for wstring_convert
#include <locale>  // for codecvt_utf8
#include <unordered_map>
#include <unordered_set>

using namespace std;

#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Enum.h"
#include "Engine_Function.h"
#include "Engine_Typedef.h"

#include "../../External Libraries/json.hpp"

namespace Engine
{
	const _wstring	g_strTransformTag = TEXT("Com_Transform");
}
using namespace Engine;

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif
#endif



