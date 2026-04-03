#pragma once
#include "Substrate/RefCounted.h"
#include "Substrate/TypeInfo.h"

#include <concepts>


namespace Substrate {

	template<typename TRefCounted>
	class RefPtr
	{
	public:
		/// <summary>
		/// Default constructor -> nullptr
		/// </summary>
		RefPtr()
			: m_Ptr(nullptr)
		{
			static_assert(std::is_base_of_v<RefCounted, TRefCounted>, "RefPtr can only be used with classes derived from RefCounted");
		}

		/// <summary>
		/// Conversion constructor for nullptr (To allow RefPtr<T> ptr = nullptr)
		/// </summary>
		RefPtr(std::nullptr_t)
			: m_Ptr(nullptr)
		{
			static_assert(std::is_base_of_v<RefCounted, TRefCounted>, "RefPtr can only be used with classes derived from RefCounted");
		}

		/// <summary>
		/// Standard type constructor
		/// </summary>
		explicit RefPtr(TRefCounted* ptr)
			: m_Ptr(ptr)
		{
		}

		/// <summary>
		/// Copy constructor -> Increases ref count
		/// </summary>
		RefPtr(const RefPtr& other)
			: m_Ptr(other.Get())
		{
			if (m_Ptr)
				m_Ptr->AddRef();
		}

		/// <summary>
		/// Copy constructor -> Increases ref count
		/// </summary>
		template <typename TOtherRefCounted> 
			requires std::is_base_of_v<TRefCounted, TOtherRefCounted>
			RefPtr(const RefPtr<TOtherRefCounted>& other)
			: m_Ptr(other.Get())
		{
			if (m_Ptr)
				m_Ptr->AddRef();
		}

		/// <summary>
		/// Move constructor -> Transfers ownership, sets other to nullptr
		/// </summary>
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
		/// Copy assignment with nullptr -> Decrease current ref count, replace pointer with nullptr
		/// </summary>
		RefPtr& operator=(std::nullptr_t)
		{
				if (m_Ptr)
					m_Ptr->DecRef();
				m_Ptr = nullptr;

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
		/// This function is for down casting only (Base->Derived). Upcasting is implicit via constructor/assignment.
		/// This function is only available, if both TRefCounted and TOtherRefCounted implement TypeInfo via the DECLARE_TYPE macro.
		/// This function increases the RefCount of the returned RefPtr if the cast is valid.
		/// </summary>
		/// <returns>RefPtr<TOtherRefCounted>(this.Get()) for valid casts, or RefPtr<TOtherRefCounted>(nullptr)</returns>
		template<typename TOtherRefCounted>
			requires IsDerivedWithTypeInfo<TOtherRefCounted> && IsDerivedWithTypeInfo<TRefCounted> && std::is_base_of_v<TRefCounted, TOtherRefCounted>
		RefPtr<TOtherRefCounted> As() const
		{
			//static_assert(std::is_base_of<TRefCounted, TDerivedRefCounted>::value, "TDerivedRefCounted must be derived from TRefCounted");
			if (!m_Ptr)
				return RefPtr<TOtherRefCounted>(nullptr);

			// We check the inheritance chain of the current object's type info to see if it matches the target type
			// Example:
			// class Base;
			// class Derived : public Base;
			// class DerivedDerived : public Derived;
			// if m_Ptr->GetTypeInfo()->Name = Derived, we can only cast to Derived or Base, and vice versa, not DerivedDerived
			uint32_t targetTypeID = TOtherRefCounted::GetStaticTypeInfo()->ID;
			const TypeInfo* checkTypeInfo = m_Ptr->GetTypeInfo();
			while (checkTypeInfo)
			{
				if (checkTypeInfo->ID == targetTypeID)
				{
					//Just creating the new RefPtr does not increase the ref count, so we need to do it manually
					m_Ptr->AddRef();
					return RefPtr<TOtherRefCounted>(static_cast<TOtherRefCounted*>(m_Ptr));
				}
				checkTypeInfo = const_cast<TypeInfo*>(checkTypeInfo->BaseType);
			}

			// If we reach here, the type is not in the inheritance chain
			return RefPtr<TOtherRefCounted>(nullptr);
		}

		/// <summary>
		/// Boolean equality operator.
		/// </summary>
		/// <returns>Returns true if pointers are the same</returns>
		bool operator==(const RefPtr<TRefCounted>& other) const
		{
			return m_Ptr == other.Get();
		}

		/// <summary>
		/// Convenience equality operator for nullptr comparisons. Returns true if the RefPtr is currently holding a nullptr.
		/// </summary>
		bool operator==(std::nullptr_t) const
		{
			return m_Ptr == nullptr;
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
		/// Convenience inequality operator for nullptr comparisons. Returns true if the RefPtr is currently holding a non-nullptr.
		/// </summary>
		bool operator!=(std::nullptr_t) const
		{
			return m_Ptr != nullptr;
		}

		/// <summary>
		/// Convenience boolean operator. Returns true if the RefPtr is currently holding a non-nullptr, false otherwise.
		/// </summary>
		explicit operator bool() const
		{
			return m_Ptr != nullptr;
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
		TRefCounted* m_Ptr;
	};
}