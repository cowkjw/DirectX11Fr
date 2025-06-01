#pragma once

namespace Engine
{
	typedef struct tagEngineDesc 
	{
		HWND			hWnd;
		bool			isWindowed;
		unsigned int	iWinSizeX;
		unsigned int	iWinSizeY;
		unsigned int	iNumLevels;		
	}ENGINE_DESC;

    struct Capsule
    {
        XMVECTOR A;    // 위 끝점
        XMVECTOR B;    // 아래 끝점
        float            Radius;
    };

    typedef struct tagLightDesc
    {
        enum TYPE { TYPE_DIRECTIONAL, TYPE_POINT, TYPE_END };

        TYPE				eType;
        XMFLOAT4			vDirection;
        XMFLOAT4			vPosition;
        float				fRange;

        XMFLOAT4			vDiffuse;
        XMFLOAT4			vAmbient;
        XMFLOAT4			vSpecular;
    }LIGHT_DESC;



    typedef struct tagKeyFrame
    {
        /* 행렬이 아닌이유? : 상태와 상태 사이를 보간해주기위해서. */
        XMFLOAT3		vScale;
        XMFLOAT4		vRotation;
        XMFLOAT3		vTranslation;

        /* 이 상태를 취해야하는 재생위치  */
        float			fTrackPosition;
    }KEYFRAME;

    struct ICollisionListener
    {
        virtual void OnCollisionEnter(class CCollider* other) = 0;
        virtual void OnCollisionStay(class CCollider* other, float fTimeDelta) = 0;
        virtual void OnCollisionExit(class CCollider* other) = 0;
    };

	typedef struct tagVector : public XMFLOAT3
	{
        tagVector() : XMFLOAT3(0.f, 0.f, 0.f) {}

        tagVector(float _x, float _y, float _z) : XMFLOAT3(_x, _y, _z) {}

        // XMVECTOR에서 변환하는 생성자
        tagVector(FXMVECTOR v) { XMStoreFloat3(this, v); }

        // XMVECTOR 대입 연산자
        tagVector& operator=(FXMVECTOR v)
        {
            XMStoreFloat3(this, v);
            return *this;
        }

        // XMVECTOR로의 암시적 변환 (벡터 연산 시 편리)
        operator XMVECTOR() const
        {
            return XMLoadFloat3(this);
        }


        tagVector operator+(const tagVector& rhs) const
        {
            return tagVector(XMVectorAdd(*this, rhs));
        }

        tagVector& operator+=(const tagVector& rhs)
        {
            *this = tagVector(XMVectorAdd(*this, rhs));
            return *this;
        }

        tagVector operator-(const tagVector& rhs) const
        {
            return tagVector(XMVectorSubtract(*this, rhs));
        }

        tagVector& operator-=(const tagVector& rhs)
        {
            *this = tagVector(XMVectorSubtract(*this, rhs));
            return *this;
        }

        tagVector operator*(float s) const
        {
            return tagVector(XMVectorScale(*this, s));
        }

        tagVector& operator*=(float s)
        {
            *this = tagVector(XMVectorScale(*this, s));
            return *this;
        }

        tagVector operator/(float s) const
        {
            return tagVector(XMVectorScale(*this, 1.f / s));
        }

        tagVector& operator/=(float s)
        {
            *this = tagVector(XMVectorScale(*this, 1.f / s));
            return *this;
        }

        float Dot(const tagVector& rhs) const
        {
            return XMVectorGetX(XMVector3Dot(*this, rhs));
        }

        tagVector Cross(const tagVector& rhs) const
        {
            return tagVector(XMVector3Cross(*this, rhs));
        }

        // 길이(length) 계산
        float Length() const
        {
            return XMVectorGetX(XMVector3Length(*this));
        }

        float LengthSq() const
        {
            return XMVectorGetX(XMVector3LengthSq(*this));
        }

        tagVector Normalize() const
        {
            return tagVector(XMVector3Normalize(*this));
        }

        // 매트릭스 변환 (좌표/법선)
        tagVector Transform(const XMMATRIX& m) const
        {
            return tagVector(XMVector3Transform(*this, m));
        }
        tagVector TransformCoord(const XMMATRIX& m) const
        {
            return tagVector(XMVector3TransformCoord(*this, m));
        }
        tagVector TransformNormal(const XMMATRIX& m) const
        {
            return tagVector(XMVector3TransformNormal(*this, m));
        }

        static const tagVector Zero() { return tagVector(0.f, 0.f, 0.f); }
        static const tagVector Right() { return tagVector(1.f, 0.f, 0.f); }
        static const tagVector Up() { return tagVector(0.f, 1.f, 0.f); }
        static const tagVector Forward() { return tagVector(0.f, 0.f, 1.f); }
	}VECTOR3;



    typedef struct ENGINE_DLL tagVertexPositionTexcoord
    {
        XMFLOAT3		vPosition;
        XMFLOAT2		vTexcoord;

        static const unsigned int					iNumElements = { 2 };

        static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
    }VTXPOSTEX;

    typedef struct ENGINE_DLL tagVertexPositionNormalTexcoord
    {
        XMFLOAT3		vPosition;
        XMFLOAT3		vNormal;
        XMFLOAT2		vTexcoord;

        static const unsigned int					iNumElements = { 3 };
        static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
    }VTXNORTEX;

    typedef struct ENGINE_DLL tagVertexMesh
    {
        XMFLOAT3		vPosition;
        XMFLOAT3		vNormal;
        XMFLOAT3		vTangent;
        XMFLOAT2		vTexcoord;

        static const unsigned int					iNumElements = { 4 };
        static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
    }VTXMESH;

    typedef struct ENGINE_DLL tagVertexAnimMesh
    {
        XMFLOAT3		vPosition;
        XMFLOAT3		vNormal;
        XMFLOAT3		vTangent;
        XMFLOAT2		vTexcoord;
        XMUINT4			vBlendIndices;
        XMFLOAT4		vBlendWeights;

        static const unsigned int					iNumElements = { 6 };
        static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
    }VTXANIMMESH;


}