#pragma once

#include "../eterLib/TextFileLoader.h"

#include "EffectElementBase.h"
#include "EmitterProperty.h"
#include "ParticleProperty.h"
//#include "ParticleInstance.h"

class CParticleInstance;

class CParticleSystemData : public CEffectElementBase
{
	public:
		virtual ~CParticleSystemData();
		CParticleSystemData();

		CEmitterProperty * GetEmitterPropertyPointer();
		CParticleProperty * GetParticlePropertyPointer();
		void ChangeTexture(const char * c_szFileName);

		void BuildDecorator(CParticleInstance * pInstance);
	protected:
		BOOL OnLoadScript(CTextFileLoader & rTextFileLoader);

		void OnClear();
		bool OnIsData();

		CEmitterProperty m_EmitterProperty;
		CParticleProperty m_ParticleProperty;

	public:
		static void DestroySystem();

		static CParticleSystemData* New();
		static void Delete(CParticleSystemData* pkData);

		static CMemoryPool<CParticleSystemData>		ms_kPool;
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
