#pragma once

#include "GrpBase.h"

class CPixelShader : public CGraphicBase
{
	public:
		CPixelShader();
		virtual ~CPixelShader();

		void Destroy();
		bool CreateFromDiskFile(const char* c_szFileName);

		void Set();

	protected:
		void Initialize();

	protected:
#ifdef DIRECTX9
		LPDIRECT3DPIXELSHADER9 m_handle;
#else
		DWORD m_handle;
#endif
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
