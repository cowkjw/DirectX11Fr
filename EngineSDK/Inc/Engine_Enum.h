#pragma once

namespace Engine
{
	enum class RENDERGROUP : unsigned int { PRIORITY, NONBLEND, BLEND, UI, END };
	enum class PROTOTYPE : unsigned int { GAMEOBJECT, COMPONENT };
	enum class STATE { RIGHT, UP, LOOK, POSITION, END };
	enum class MOUSEBUTTON { LEFT, RIGHT};
	
}