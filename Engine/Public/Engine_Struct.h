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


	typedef struct tagVector : public XMFLOAT3
	{
        tagVector() : XMFLOAT3(0.f, 0.f, 0.f) {}

        tagVector(_float _x, _float _y, _float _z) : XMFLOAT3(_x, _y, _z) {}

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

        _float Dot(const tagVector& rhs) const
        {
            return XMVectorGetX(XMVector3Dot(*this, rhs));
        }

        tagVector Cross(const tagVector& rhs) const
        {
            return tagVector(XMVector3Cross(*this, rhs));
        }

        // 길이(length) 계산
        _float Length() const
        {
            return XMVectorGetX(XMVector3Length(*this));
        }

        _float LengthSq() const
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
	}VECTOR;
}