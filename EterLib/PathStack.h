#pragma once

#include <deque>
#include <string>

class CPathStack
{
	public:
		CPathStack();
		virtual ~CPathStack();

		void SetBase();

		void MoveBase();

		void Push();

		bool Pop();

		void Move(const char* c_szPathName);
		void GetCurrentPathName(std::string* pstCurPathName);

	protected:
		std::string				m_stBasePathName;
		std::deque<std::string>	m_stPathNameDeque;
};
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
