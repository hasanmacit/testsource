#include "stdafx.h"
#ifndef DISABLE_POST_PROCESSING
#include "../EterLib/StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterPack/EterPackManager.h"
#include "PostProcessingEffect.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

CPostProcessingEffect::CPostProcessingEffect() : m_pPixelShader(nullptr), m_pConstants(nullptr)
{
}
CPostProcessingEffect::CPostProcessingEffect(const CPostProcessingEffect& c_kOld) : m_pPixelShader(nullptr), m_pConstants(nullptr), m_strName(c_kOld.m_strName), m_kConfig(c_kOld.m_kConfig)
{
	SetShader(c_kOld.m_pPixelShader, c_kOld.m_pConstants);
}
CPostProcessingEffect::CPostProcessingEffect(CPostProcessingEffect&& r_kOld) : m_pConstants(r_kOld.m_pConstants), m_pPixelShader(r_kOld.m_pPixelShader), m_strName(std::move(r_kOld.m_strName)), m_kConfig(std::move(r_kOld.m_kConfig))
{
	r_kOld.m_pPixelShader = nullptr;
	r_kOld.m_pConstants = nullptr;
}
CPostProcessingEffect::~CPostProcessingEffect()
{
	SetShader(nullptr, nullptr);
}

CPostProcessingEffect& CPostProcessingEffect::operator=(const CPostProcessingEffect& c_kOld)
{
	if (&c_kOld == this)
	{
		return *this;
	}

	m_kConfig = c_kOld.m_kConfig;
	SetShader(c_kOld.m_pPixelShader, c_kOld.m_pConstants);
	m_strName = c_kOld.m_strName;

	return *this;
}
CPostProcessingEffect& CPostProcessingEffect::operator=(CPostProcessingEffect&& r_kOld)
{
	if (&r_kOld == this)
	{
		return *this;
	}

	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
	}
	if (m_pConstants)
	{
		m_pConstants->Release();
	}
	m_kConfig = std::move(r_kOld.m_kConfig);
	m_pPixelShader = r_kOld.m_pPixelShader;
	m_pConstants = r_kOld.m_pConstants;
	m_strName = std::move(r_kOld.m_strName);
	r_kOld.m_pPixelShader = nullptr;
	r_kOld.m_pConstants = nullptr;

	return *this;
}

bool CPostProcessingEffect::operator==(const char* c_pszName) const
{
	return !stricmp(m_strName.c_str(), c_pszName);
}
bool CPostProcessingEffect::operator==(const CPostProcessingEffect& c_kOld) const
{
	return !stricmp(m_strName.c_str(), c_kOld.GetName());
}

void CPostProcessingEffect::SetName(const char* c_pszName)
{
	m_strName = c_pszName;
}
const char* CPostProcessingEffect::GetName() const
{
	return m_strName.c_str();
}

bool CPostProcessingEffect::CreateShader(CMappedFile& rkDescriptor)
{
	char* pszBuffer = new char[rkDescriptor.Size() + 1];
	rkDescriptor.Read(pszBuffer, rkDescriptor.Size());
	pszBuffer[rkDescriptor.Size()] = 0;

	std::stringstream kDescriptorConfig;
	kDescriptorConfig << pszBuffer;
	delete [] pszBuffer;

	try
	{
		read_xml(kDescriptorConfig, m_kConfig);
	}
	catch (std::exception& e)
	{
		TraceError("%s", e.what());
		return false;
	}

	m_strName = m_kConfig.get<std::string>("Effect.<xmlattr>.Name", "<shader>");

	CMappedFile kFile;
	LPCVOID pData;
	if (!CEterPackManager::Instance().Get(kFile, m_kConfig.get<std::string>("Effect.<xmlattr>.Source").c_str(), &pData))
	{
		TraceError("CPostProcessingEffect unable to load effect %s", m_kConfig.get<std::string>("Effect.<xmlattr>.Source").c_str());
		return false;
	}

	std::string shaderCode;
	shaderCode.assign(static_cast<const char*>(pData), kFile.Size());

	LPD3DXBUFFER pCode;
	LPD3DXBUFFER pError = nullptr;
	if (FAILED(D3DXCompileShader(shaderCode.c_str(), shaderCode.length(), nullptr, nullptr, m_kConfig.get<std::string>("Effect.<xmlattr>.Entry", "EffectEntry").c_str(), "ps_3_0", 0, &pCode, &pError, &m_pConstants)))
	{
		char* pszError = new char[pError->GetBufferSize() + 1];
		memcpy(pszError, pError->GetBufferPointer(), pError->GetBufferSize());
		pszError[pError->GetBufferSize()] = 0;
		pError->Release();
		TraceError("CPostProcessingEffect unable to compile effect %s: %s", m_kConfig.get<std::string>("Effect.<xmlattr>.Source").c_str(), pszError);
		delete [] pszError;
		return false;
	}
	if (pError)
	{
		pError->Release();
	}
	if (FAILED(STATEMANAGER.GetDevice()->CreatePixelShader(reinterpret_cast<DWORD*>(pCode->GetBufferPointer()), &m_pPixelShader)))
	{
		TraceError("CPostProcessingEffect unable to create effect instance %s", m_kConfig.get<std::string>("Effect.<xmlattr>.Source").c_str());
		pCode->Release();
		m_pConstants->Release();
		return false;
	}
	pCode->Release();
	return true;
}
void CPostProcessingEffect::SetShader(IDirect3DPixelShader9* lpShader, LPD3DXCONSTANTTABLE lpConstants)
{
	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
	}
	if (m_pConstants)
	{
		m_pConstants->Release();
	}
	m_pPixelShader = lpShader;
	m_pConstants = lpConstants;
	if (m_pPixelShader)
	{
		m_pPixelShader->AddRef();
	}
	if (m_pConstants)
	{
		m_pConstants->AddRef();
	}
}
IDirect3DPixelShader9* CPostProcessingEffect::GetShader()
{
	return m_pPixelShader;
}
LPD3DXCONSTANTTABLE CPostProcessingEffect::GetConstants()
{
	return m_pConstants;
}

bool CPostProcessingEffect::Apply()
{
	if (Frustum::instance().dynamicLight == 1 || Frustum::instance().dynamicLight == 2)
	{
		STATEMANAGER.GetDevice()->SetPixelShader(m_pPixelShader);
	}
	else
	{
		STATEMANAGER.GetDevice()->SetPixelShader(nullptr);
	}

	return true;
}
#endif
