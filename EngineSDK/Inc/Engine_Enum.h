#pragma once

namespace Engine
{
	enum class RENDERGROUP : unsigned int { RG_PRIORITY, RG_NONBLEND, RG_BLEND, RG_UI, RG_END };
	enum class PROTOTYPE : unsigned int { TYPE_GAMEOBJECT, TYPE_COMPONENT };
	enum class STATE { RIGHT, UP, LOOK, POSITION, END };
	
}