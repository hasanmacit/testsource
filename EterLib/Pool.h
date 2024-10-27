#pragma once

#include <algorithm>
#include "../eterBase/Debug.h"

//#define DYNAMIC_POOL_STRICT

#ifdef NEW_MEMORY_POOL
template<typename T>
class CMemoryPool
{
public:
	CMemoryPool(size_t uChunkSize = 16, bool bGrowChunkSize = true)
		: m_uChunkSize(uChunkSize)
		, m_bGrowChunkSize(bGrowChunkSize)
	{

	}

	~CMemoryPool()
	{
		Clear();
	}

	// LEGACY
	void Create(size_t uChunkSize = 16, bool bGrowChunkSize = true)
	{
		m_uChunkSize = uChunkSize;
		m_bGrowChunkSize = bGrowChunkSize;
	}
	void Destroy()
	{
		Clear();
	}
	void FreeAll()
	{
		Clear();
	}
	// END_OF_LEGACY

	void Clear()
	{
		assert(m_Free.size() == m_Data.size() && "Memory pool has unfreed objects!");	

		if (m_Free.size() != m_Data.size())
		{
			for (T* pData : m_Data)
			{
				if (std::find(m_Free.begin(), m_Free.end(), pData) == m_Free.end())
				{
					pData->~T();
				}
			}
		}

		m_Data.clear();
		m_Data.shrink_to_fit();

		m_Free.clear();
		m_Free.shrink_to_fit();

		for (T* pChunk : m_Chunk)
		{
			::free(pChunk);
		}

		m_Chunk.clear();
		m_Chunk.shrink_to_fit();
	}

	template<class... _Types>
	T* Alloc(_Types&&... _Args)
	{
		if (m_Free.empty())
			Grow();

		T* pNew = m_Free.back();
		m_Free.pop_back();
		return new(pNew) T(std::forward<_Types>(_Args)...);
	}

	void Free(T* pData)
	{
		pData->~T();
		m_Free.emplace_back(pData);
	}

	size_t GetCapacity() const
	{
		return m_Data.size();
	}

private:
	void Grow()
	{
		size_t uChunkSize = m_uChunkSize;

		if (m_bGrowChunkSize)
			uChunkSize += uChunkSize * m_Chunk.size();

		T* pStart = (T*) ::malloc(uChunkSize * sizeof(T));
		m_Chunk.emplace_back(pStart);

		m_Data.reserve(m_Data.size() + uChunkSize);
		m_Free.reserve(m_Free.size() + uChunkSize);

		for (size_t i = 0; i < uChunkSize; ++i)
		{
			m_Data.emplace_back(pStart + i);
			m_Free.emplace_back(pStart + i);
		}
	}

private:
	size_t m_uChunkSize;
	bool m_bGrowChunkSize;

	std::vector<T*> m_Data;
	std::vector<T*> m_Free;

	std::vector<T*> m_Chunk;
};

#else
template<typename T>
class CMemoryPool
{
	public:
		CMemoryPool()
		{
			m_uInitCapacity=0;
			m_uUsedCapacity=0;
		}
		virtual ~CMemoryPool()
		{
			assert(m_kVct_pkData.empty());
		}

		void SetName(const char* c_szName)
		{
		}

		void Clear()
		{
			Destroy();
		}

		void Destroy()
		{
			std::for_each(m_kVct_pkData.begin(), m_kVct_pkData.end(), Delete);
			m_kVct_pkData.clear();
			m_kVct_pkFree.clear();
		}

		void Create(UINT uCapacity)
		{
			m_uInitCapacity=uCapacity;
			m_kVct_pkData.reserve(uCapacity);
			m_kVct_pkFree.reserve(uCapacity);
		}
		T* Alloc()
		{
			if (m_kVct_pkFree.empty())
			{
				T* pkNewData=new T;
				m_kVct_pkData.push_back(pkNewData);
				++m_uUsedCapacity;
				return pkNewData;
			}

			T* pkFreeData=m_kVct_pkFree.back();
			m_kVct_pkFree.pop_back();
			return pkFreeData;
		}
		void Free(T* pkData)
		{
#ifdef DYNAMIC_POOL_STRICT
			assert(__IsValidData(pkData));
			assert(!__IsFreeData(pkData));
#endif
			m_kVct_pkFree.push_back(pkData);
		}
		void FreeAll()
		{
			m_kVct_pkFree=m_kVct_pkData;
		}

		DWORD GetCapacity()
		{
			return m_kVct_pkData.size();
		}

	protected:
		bool __IsValidData(T* pkData)
		{
			if (m_kVct_pkData.end()==std::find(m_kVct_pkData.begin(), m_kVct_pkData.end(), pkData))
				return false;
			return true;
		}
		bool __IsFreeData(T* pkData)
		{
			if (m_kVct_pkFree.end()==std::find(m_kVct_pkFree.begin(), m_kVct_pkFree.end(), pkData))
				return false;

			return true;
		}

		static void Delete(T* pkData)
		{
			delete pkData;
		}

	protected:
		std::vector<T*> m_kVct_pkData;
		std::vector<T*> m_kVct_pkFree;

		UINT m_uInitCapacity;
		UINT m_uUsedCapacity;
};
#endif

template<typename T>
class CMemoryPoolEx
{
	public:
		CMemoryPoolEx()
		{
			m_uInitCapacity=0;
			m_uUsedCapacity=0;
		}
		virtual ~CMemoryPoolEx()
		{
			assert(m_kVct_pkFree.size()==m_kVct_pkData.size());
			Destroy();

#ifdef _DEBUG
			char szText[256];
			sprintf(szText, "--------------------------------------------------------------------- %s Pool Capacity %d\n", typeid(T).name(), m_uUsedCapacity);
			OutputDebugString(szText);
			printf(szText);
#endif
		}

		void Clear()
		{
			Destroy();
		}

		void Destroy()
		{
#ifdef _DEBUG
			if (!m_kVct_pkData.empty())
			{
				char szText[256];
				sprintf(szText, "--------------------------------------------------------------------- %s Pool Destroy\n", typeid(T).name());
				OutputDebugString(szText);
				printf(szText);
			}
#endif
			std::for_each(m_kVct_pkData.begin(), m_kVct_pkData.end(), Delete);
			m_kVct_pkData.clear();
			m_kVct_pkFree.clear();
		}

		void Create(UINT uCapacity)
		{
			m_uInitCapacity=uCapacity;
			m_kVct_pkData.reserve(uCapacity);
			m_kVct_pkFree.reserve(uCapacity);
		}
		T* Alloc()
		{
			if (m_kVct_pkFree.empty())
			{
				T* pkNewData=New();
				m_kVct_pkData.push_back(pkNewData);
				++m_uUsedCapacity;
				return pkNewData;
			}

			T* pkFreeData=m_kVct_pkFree.back();
			m_kVct_pkFree.pop_back();
			return pkFreeData;
		}
		void Free(T* pkData)
		{
#ifdef DYNAMIC_POOL_STRICT
			assert(__IsValidData(pkData));
			assert(!__IsFreeData(pkData));
#endif
			m_kVct_pkFree.push_back(pkData);
		}
		void FreeAll()
		{
			m_kVct_pkFree=m_kVct_pkData;
		}

		DWORD GetCapacity()
		{
			return m_kVct_pkData.size();
		}

	protected:
		bool __IsValidData(T* pkData)
		{
			if (m_kVct_pkData.end()==std::find(m_kVct_pkData.begin(), m_kVct_pkData.end(), pkData))
				return false;
			return true;
		}
		bool __IsFreeData(T* pkData)
		{
			if (m_kVct_pkFree.end()==std::find(m_kVct_pkFree.begin(), m_kVct_pkFree.end(), pkData))
				return false;

			return true;
		}

		static T* New()
		{
			return (T*)::operator new(sizeof(T));
		}
		static void Delete(T* pkData)
		{
			::operator delete(pkData);
		}

	protected:
		std::vector<T*> m_kVct_pkData;
		std::vector<T*> m_kVct_pkFree;

		UINT m_uInitCapacity;
		UINT m_uUsedCapacity;
};

template <class T>
class CPooledObject
{
    public:
		CPooledObject()
		{
		}
		virtual ~CPooledObject()
		{
		}

        void * operator new(unsigned int /*mem_size*/)
        {
            return ms_kPool.Alloc();
        }

        void operator delete(void* pT)
        {
            ms_kPool.Free((T*)pT);
        }

        static void DestroySystem()
        {
            ms_kPool.Destroy();
        }

        static void DeleteAll()
        {
            ms_kPool.FreeAll();
        }

    protected:
        static CMemoryPoolEx<T> ms_kPool;
};

template <class T> CMemoryPoolEx<T> CPooledObject<T>::ms_kPool;
//martysama0134's 7f12f88f86c76f82974cba65d7406ac8
