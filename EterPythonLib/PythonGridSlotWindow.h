#pragma once

#include "PythonSlotWindow.h"

namespace UI
{
	class CGridSlotWindow : public CSlotWindow
	{
		public:
			static DWORD Type();

		public:
			CGridSlotWindow(PyObject * ppyObject);
			virtual ~CGridSlotWindow();

			void Destroy();

			void ArrangeGridSlot(DWORD dwStartIndex, DWORD dwxCount, DWORD dwyCount, int ixSlotSize, int iySlotSize, int ixTemporarySize, int iyTemporarySize);

		protected:
			void __Initialize();

			BOOL GetPickedSlotPointer(TSlot ** ppSlot);
			BOOL GetPickedSlotList(int iWidth, int iHeight, std::list<TSlot*> * pSlotPointerList);
			BOOL GetGridSlotPointer(int ix, int iy, TSlot ** ppSlot);
			BOOL GetPickedGridSlotPosition(int ixLocal, int iyLocal, int * pix, int * piy);
			BOOL CheckMoving(DWORD dwSlotNumber, DWORD dwItemIndex, const std::list<TSlot*> & c_rSlotList);
			BOOL GetSlotPointerByNumber(DWORD dwSlotNumber, TSlot** ppSlot);
#ifdef ENABLE_SWAP_SLOT_SYSTEM
			BOOL CheckSwapping(DWORD dwSlotNumber, DWORD dwItemIndex, const std::list<TSlot*>& c_rSlotList);
#endif

			BOOL OnIsType(DWORD dwType);

			void OnRefreshSlot();
			void OnRenderPickingSlot();

		protected:
			DWORD m_dwxCount;
			DWORD m_dwyCount;


			std::vector<TSlot *> m_SlotVector;
	};
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
