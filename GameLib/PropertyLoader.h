#pragma once

#include "../eterbase/FileDir.h"

class CPropertyManager;

class CPropertyLoader : public CDir
{
	public:
		CPropertyLoader();
		virtual ~CPropertyLoader();

		void			SetPropertyManager(CPropertyManager * pPropertyManager);
		DWORD			RegisterFile(const char* c_szPathName, const char* c_szFileName);

		virtual bool	OnFolder(const char* c_szFilter, const char* c_szPathName, const char* c_szFileName);
		virtual bool	OnFile(const char* c_szPathName, const char* c_szFileName);

	protected:
		CPropertyManager * m_pPropertyManager;
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
