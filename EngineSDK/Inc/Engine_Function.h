#pragma once

namespace Engine
{
	template<typename T>
	void Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete[] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned int Safe_AddRef(T& Instance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != Instance)
			iRefCnt = Instance->AddRef();

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_Release(T& Instance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != Instance)
		{
 			iRefCnt = Instance->Release();
			if (0 == iRefCnt)
				Instance = nullptr;
		}
		return iRefCnt;
	}


	template<typename E>
	constexpr unsigned int ToIndex(E e) {
		static_assert(is_enum<E>::value, "ToIndex only accepts enum types.");
		return static_cast<unsigned int>(e);
	}

	inline wstring StringToWString(const string& str)
	{
		return wstring(str.begin(), str.end());
	}

	inline string WStringToString(const wstring& w) {
	/*	int size = ::WideCharToMultiByte(
			CP_UTF8, 0,
			w.c_str(), -1,
			nullptr, 0,
			nullptr, nullptr
		);
		string s(size, 0);
		::WideCharToMultiByte(
			CP_UTF8, 0,
			w.c_str(), -1,
			&s[0], size,
			nullptr, nullptr
		);
		return s;*/
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		string s = converter.to_bytes(w);
		return s;

	}


	inline void WriteUInt(std::ofstream& ofs, uint32_t v) {
		ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
	}
	inline uint32_t ReadUInt(std::ifstream& ifs) {
		uint32_t v;
		ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
		return v;
	}

    // 점 P와 선분 AB 사이의 최단점(closest point)을 구하는 함수
    inline XMVECTOR ClosestPointOnSegment(
        const XMVECTOR& P,
        const XMVECTOR& A,
        const XMVECTOR& B)
    {
        XMVECTOR AB = B - A;
        float ab2 = XMVectorGetX(XMVector3Dot(AB, AB));
        if (ab2 < 1e-6f)
            return A; // 선분 길이가 너무 작으면 A를 반환

        float t = XMVectorGetX(XMVector3Dot(P - A, AB)) / ab2;
		t = max(0.f, min(1.f, t)); // t를 [0, 1] 범위로 클램프
        return A + AB * t;
    }

    //두 선분 P1Q1, P2Q2 사이 최단 거리 두 점(closest points)을 구하는 함수
    inline void ClosestPointsSegmentSegment(
        const XMVECTOR& P1,
        const XMVECTOR& Q1,
        const XMVECTOR& P2,
        const XMVECTOR& Q2,
        XMVECTOR& outC1,
        XMVECTOR& outC2)
    {
        XMVECTOR   u = Q1 - P1;
        XMVECTOR   v = Q2 - P2;
        XMVECTOR   w = P1 - P2;

        float a = XMVectorGetX(XMVector3Dot(u, u));      // u·u
        float b = XMVectorGetX(XMVector3Dot(u, v));      // u·v
        float c = XMVectorGetX(XMVector3Dot(v, v));      // v·v
        float d = XMVectorGetX(XMVector3Dot(u, w));      // u·w
        float e = XMVectorGetX(XMVector3Dot(v, w));      // v·w

        float D = a * c - b * b;  // 분모

        float sc, sN, sD = D;     // sc = sN / sD
        float tc, tN, tD = D;     // tc = tN / tD

        // (1) 두 축이 거의 평행한지 검사
        if (D < 1e-6f) {
            sN = 0.0f;
            sD = 1.0f;
            tN = e;
            tD = c;
        }
        else {
            sN = (b * e - c * d);
            tN = (a * e - b * d);
            if (sN < 0.0f) {
                sN = 0.0f;
                tN = e;
                tD = c;
            }
            else if (sN > sD) {
                sN = sD;
                tN = e + b;
                tD = c;
            }
        }

        // (2) t 값 클램프
        if (tN < 0.0f) {
            tN = 0.0f;
            if (-d < 0.0f) {
                sN = 0.0f;
            }
            else if (-d > a) {
                sN = sD;
            }
            else {
                sN = -d;
                sD = a;
            }
        }
        else if (tN > tD) {
            tN = tD;
            if ((-d + b) < 0.0f) {
                sN = 0.0f;
            }
            else if ((-d + b) > a) {
                sN = sD;
            }
            else {
                sN = (-d + b);
                sD = a;
            }
        }

        // (3) 최종 sc, tc 계산
        sc = (fabsf(sN) < 1e-6f ? 0.0f : (sN / sD));
        tc = (fabsf(tN) < 1e-6f ? 0.0f : (tN / tD));

        // (4) 두 축 위의 최단점
        outC1 = P1 + u * sc;
        outC2 = P2 + v * tc;
    }
}
