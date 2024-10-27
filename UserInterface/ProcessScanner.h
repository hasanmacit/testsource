#pragma once

typedef std::pair<DWORD, std::string> CRCPair;

void ProcessScanner_Destroy();
bool ProcessScanner_Create();
void ProcessScanner_ReleaseQuitEvent();

bool ProcessScanner_PopProcessQueue(std::vector<CRCPair>* pkVct_crcPair);
//martysama0134's 39109a5bb10ccb7aff1313d369804b74
