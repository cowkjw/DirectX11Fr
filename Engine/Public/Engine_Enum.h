#pragma once

namespace Engine
{
	enum class RENDERGROUP : unsigned int { PRIORITY, NONBLEND, BLEND, UI, END };
	enum class PROTOTYPE : unsigned int { GAMEOBJECT, COMPONENT };
	enum class STATE { RIGHT, UP, LOOK, POSITION, END };
	enum class MOUSEBUTTON { LEFT, RIGHT};
	enum class TRANSFORM { VIEW, PROJECTION,ORTHO, END };
	enum class UI_TYPE { IMAGE, BUTTON, BAR,CANVAS };
	enum class BUFFER_TYPE { RECT, CUBE, TERRAIN };
	enum class RESOURCE_TYPE { TEXTURE, SHADER };
	
}