#include "StdAfx.h"
#include "PythonNetworkStream.h"

void CPythonNetworkStream::OnRemoteDisconnect()
{
	PyCallClassMemberFunc(m_poHandler, "SetLoginPhase", Py_BuildValue("()"));
}

void CPythonNetworkStream::OnDisconnect()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Game
void CPythonNetworkStream::OnScriptEventStart(int iSkin, int iIndex)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenQuestWindow", Py_BuildValue("(ii)", iSkin, iIndex));
}
//martysama0134's 39109a5bb10ccb7aff1313d369804b74
