#pragma once

#pragma warning (disable : 4251)

#include <d3d11.h>
#include "DirectXCollision.h"
#include <DirectXMath.h>
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/Keyboard.h"
#include "DirectXTK/Mouse.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/Effects.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"
#include <DirectXColors.h>
#include "DirectXTK/ScreenGrab.h"


#include "Fx11/d3dx11effect.h"
#include <d3dcompiler.h>

using namespace DirectX;

#include "DebugDraw.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "fmod.h"
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"

using namespace FMOD;
#include <thread>
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
#include<iostream>
#include<fstream>

using namespace std;

#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Enum.h"
#include "Engine_Function.h"
#include "Engine_Typedef.h"

#include "../../External Libraries/json.hpp"
using json = nlohmann::json;
namespace Engine
{
	const _wstring	g_strTransformTag = TEXT("Com_Transform");
	const _uint g_iMaxNumBones = 512;


	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;
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



