#include "StdAfx.h"
#include "ActorInstance.h"
#include "WeaponTrace.h"

void CActorInstance::TraceProcess()
{
	if (!m_WeaponTraceVector.empty())
	{
		std::vector<CWeaponTrace*>::iterator it;
		for (it = m_WeaponTraceVector.begin(); it != m_WeaponTraceVector.end(); ++it)
		{
			CWeaponTrace* pWeaponTrace = (*it);
			pWeaponTrace->SetPosition(m_x, m_y, m_z);

			Frustum::Instance().weaponPosition = { m_x, m_y + 75.0f, m_z + 75.0f };
		

			pWeaponTrace->SetRotation(m_fcurRotation);
			pWeaponTrace->Update(__GetReachScale());
		}
	}
}

void CActorInstance::RenderTrace()
{
	for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(), std::void_mem_fun(&CWeaponTrace::Render));
}

void CActorInstance::__DestroyWeaponTrace()
{
	std::for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(), CWeaponTrace::Delete);
	m_WeaponTraceVector.clear();
}

void CActorInstance::__ShowWeaponTrace()
{
	for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(), std::void_mem_fun(&CWeaponTrace::TurnOn));
}

void CActorInstance::__HideWeaponTrace()
{
	for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(), std::void_mem_fun(&CWeaponTrace::TurnOff));
}
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
