#pragma once

#include "../eterLib/GrpScreen.h"

#include "EffectElementBaseInstance.h"
#include "SimpleLightData.h"

class CLightInstance : public CEffectElementBaseInstance
{
	public:
		friend class CLightData;

		CLightInstance();
		virtual ~CLightInstance();

	protected:

		void OnSetDataPointer(CEffectElementBase * pElement);

		void OnInitialize();
		void OnDestroy();

		bool OnUpdate(float fElapsedTime);
		void OnRender();

		DWORD			m_LightID;
		CLightData *	m_pData;
		DWORD			m_dwRangeIndex;

		DWORD			m_iLoopCount;

	public:
		static void DestroySystem();

		static CLightInstance* New();
		static void Delete(CLightInstance* pkData);

		static CMemoryPool<CLightInstance>		ms_kPool;
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
