#pragma once

namespace Engine
{
	enum class RENDERGROUP : unsigned int { PRIORITY, NONBLEND,NONLIGHT ,BLEND,TOON,DISTORTION, UI, END };
	enum class PROTOTYPE : unsigned int { GAMEOBJECT, COMPONENT };
	enum class STATE { RIGHT, UP, LOOK, POSITION, END };
	enum class MOUSEBUTTON { LEFT, RIGHT};
	enum class TRANSFORM { VIEW, PROJECTION,ORTHO, END };
	enum class UI_TYPE { IMAGE, BUTTON, BAR,CANVAS ,END};
	enum class BUFFER_TYPE { RECT, CUBE, TERRAIN };
	enum class RESOURCE_TYPE { TEXTURE, SHADER };
	enum class MODEL { NONANIM, ANIM };
	enum class ParamType { Bool, Trigger, Float, Int };
	enum class ColliderType
	{
		BOX,
		SPHERE,
		CAPSULE,
		HITBOX,
		HURTBOX,
		ENVIRONMENT,
		RANGE,
		NONE
	};
	enum class PARTICLE_TYPE{POINT, RECT};
	
}