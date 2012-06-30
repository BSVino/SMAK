/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <memory>

template <class C> class CHandle;

template<class C>
class CResource : public std::shared_ptr<C>
{
public:
	using std::shared_ptr<C>::get;
	using std::shared_ptr<C>::reset;

public:
	CResource()
		: std::shared_ptr<C>()
	{
	}

	explicit CResource(C* c)
		: std::shared_ptr<C>(c)
	{
	}

	CResource(std::shared_ptr<C> c)
		: std::shared_ptr<C>(c)
	{
	}

	CResource(CHandle<C>& c);

public:
	// No run-time checking. Use only if you're sure about the type.
	template <class T>
	T* DowncastStatic() const
	{
		C* p = get();
		return static_cast<T*>(p);
	}

	operator C*() const
	{
		return get();
	}

	bool operator!() const
	{
		return !get();
	}

	CResource& operator=(CResource&& c)
	{
		if (&c == this)
			return *this;

		reset();
		swap(c);

		return *this;
	}
};

template<class C>
class CHandle : public std::weak_ptr<C>
{
public:
	CHandle()
		: std::weak_ptr<C>()
	{
	}

	CHandle(const CResource<C>& r)
		: std::weak_ptr<C>(r)
	{
	}

public:
	using std::weak_ptr<C>::expired;
	using std::weak_ptr<C>::lock;

	template <class T>
	T* Downcast() const
	{
		if (expired())
			return nullptr;

		C* p = lock().get();
		return dynamic_cast<T*>(p);
	}

	// No run-time checking. Use only if you're sure about the type.
	template <class T>
	T* DowncastStatic() const
	{
		if (expired())
			return nullptr;

		C* p = lock().get();
		return static_cast<T*>(p);
	}

	C* Get() const
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	C* operator->()
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	const C* operator->() const
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	operator C*() const
	{
		if (expired())
			return nullptr;

		return lock().get();
	}

	bool operator!() const
	{
		return expired();
	}
};

template <class C>
CResource<C>::CResource(CHandle<C>& c)
	: std::shared_ptr<C>(c)
{
}
