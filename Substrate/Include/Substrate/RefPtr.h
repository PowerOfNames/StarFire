#pragma once
#include "Substrate/RefCounted.h"

#include <concepts>

namespace Substrate {

	template<typename TDerived>
	concept IsDerivedRefCounted = std::is_base_of<RefCounted, TDerived>::value;

	template<typename TRefCounted>
	class RefPtr
	{
	public:
		/// <summary>
		/// Default constructor -> nullptr
		/// </summary>
		RefPtr()
		{
			static_assert(IsDerivedRefCounted<TRefCounted>, "RefPtr can only be used with classes derived from RefCounted");
		}

		/// <summary>
		/// Standard type constructor
		/// </summary>
		/// <param name="ptr"></param>
		RefPtr(TRefCounted* ptr)
			: m_Ptr(ptr)
		{
		}

		/// <summary>
		/// Copy constructor -> Increases ref count
		/// </summary>
		/// <param name="other"></param>
		RefPtr(const RefPtr& other)
			: m_Ptr(other.Get())
		{
			if (m_Ptr)
				m_Ptr->AddRef();
		}

		/// <summary>
		/// Move constructor -> Transfers ownership, sets other to nullptr
		/// </summary>
		/// <param name="other"></param>
		RefPtr(RefPtr&& other) noexcept
			: m_Ptr(other.m_Ptr)
		{
			other.m_Ptr = nullptr;
		}

		/// <summary>
		/// Destructor -> Decreases ref count
		/// </summary>
		~RefPtr()
		{
			if (m_Ptr)
				m_Ptr->DecRef();
		}

		/// <summary>
		/// Copy assignment -> If not self assignment, decrease current ref count, replace pointer, then increase new ref count
		/// </summary>
		/// <param name="other"></param>
		/// <returns></returns>
		RefPtr& operator=(const RefPtr& other)
		{
			if (this != &other)
			{
				if (other.Get())
					other.Get()->AddRef();
				if (m_Ptr)
					m_Ptr->DecRef();
				m_Ptr = other.Get();
			}
			return *this;
		}

		/// <summary>			 
		//Move assignment -> If not self assignment, decrease current ref count, replace pointer, set other to nullpointer
		/// </summary>
		RefPtr& operator=(RefPtr&& other) noexcept
		{
			if (this != &other)
			{
				if (m_Ptr)
					m_Ptr->DecRef();
				m_Ptr = other.Get();
				other.Release();
			}
			return *this;
		}

		/// <summary>
		/// CAUTION: This MUST be handled with care to avoid dangling pointers!
		/// </summary>
		/// <returns>Returns the raw pointer of T</returns>
		TRefCounted* Get() const { return m_Ptr; }

		/// <summary>
		/// CAUTION: This MUST be handled with care to avoid dangling pointers!
		/// 
		/// Releases ownership of the pointer and decreases ref count
		/// </summary>
		TRefCounted* Release()
		{
			if (m_Ptr)
			{
				TRefCounted* temp = m_Ptr;
				m_Ptr = nullptr;
				return temp;
			}
			return nullptr;
		}

		/// <summary>
		/// Boolean equality operator.
		/// </summary>
		/// <param name="other"></param>
		/// <returns>Returns true if pointers are the same</returns>
		bool operator==(const RefPtr<TRefCounted>& other) const
		{
			return m_Ptr == other.Get();
		}

		/// <summary>
		/// Inequality operator
		/// </summary>
		/// <param name="other"></param>
		/// <returns>Returns true if pointers are not the same</returns>
		bool operator!=(const RefPtr<TRefCounted>& other) const
		{
			return m_Ptr != other.Get();
		}

		/// <summary>
		/// Dereference operator
		/// </summary>
		/// <returns>Returns object of T</returns>
		TRefCounted& operator*() const { return *m_Ptr; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns>Returns the raw pointer of T</returns>
		TRefCounted* operator->() const { return m_Ptr; }

	private:
		TRefCounted* m_Ptr = nullptr;
	};
}
