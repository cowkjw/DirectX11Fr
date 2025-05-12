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
}
