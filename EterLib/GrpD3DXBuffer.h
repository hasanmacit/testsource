#pragma once

class CDirect3DXBuffer
{
	public:
		CDirect3DXBuffer();
		CDirect3DXBuffer(LPD3DXBUFFER lpd3dxBuffer);
		virtual ~CDirect3DXBuffer();

		void Destroy();
		void Create(LPD3DXBUFFER lpd3dxBuffer);

		void*GetPointer();
		int  GetSize();

	protected:
		LPD3DXBUFFER m_lpd3dxBuffer;
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
