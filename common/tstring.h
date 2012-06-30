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

#ifndef TINKER_STRING_H
#define TINKER_STRING_H

#ifdef WITH_EASTL
#include <EASTL/string.h>

#define TSTRING_BASE eastl::basic_string<char>
#else
#include <string>

#define TSTRING_BASE std::basic_string<char>
#endif

#include "common.h"

class tstring : public TSTRING_BASE
{
public:
	tstring()
		: TSTRING_BASE()
	{}

	tstring(const char* s)
		: TSTRING_BASE(s)
	{}

	tstring(const TSTRING_BASE& s)
		: TSTRING_BASE(s)
	{}

	tstring(TSTRING_BASE&& s)
		: TSTRING_BASE(s)
	{}

	tstring(size_type n, value_type c)
		: TSTRING_BASE(n, c)
	{}

	tstring(const value_type* b, const value_type* e)
		: TSTRING_BASE(b, e)
	{}

public:
	using TSTRING_BASE::replace;

	inline tstring&	replace(const tstring& f, const tstring& r)
	{
		size_t iPosition;
		while ((iPosition = find(f)) != tstring::npos)
			assign(substr(0, iPosition) + r + (c_str()+iPosition+f.length()));

		return *this;
	}

	inline tstring&	replace(const char* f, const char* r)
	{
		size_t iPosition;
		while ((iPosition = find(f)) != tstring::npos)
			assign(substr(0, iPosition) + r + (c_str()+iPosition+strlen(f)));

		return *this;
	}

	inline tstring&	tolower()
	{
		for (size_t i = 0; i < length(); i++)
			(*this)[i] = ::tolower((*this)[i]);

		return *this;
	}

	inline tstring&	toupper()
	{
		for (size_t i = 0; i < length(); i++)
			(*this)[i] = ::toupper((*this)[i]);

		return *this;
	}

	inline bool startswith(const tstring& sBeginning) const
	{
		if (length() < sBeginning.length())
			return false;

		return (substr(0, sBeginning.length()) == sBeginning);
	}

	inline bool endswith(const tstring& sEnding) const
	{
		if (length() < sEnding.length())
			return false;

		return (substr(length() - sEnding.length()) == sEnding);
	}

	inline bool endswith(const char* sEnding) const
	{
		size_t iEndLength = strlen(sEnding);
		if (length() < iEndLength)
			return false;

		return (substr(length() - iEndLength) == sEnding);
	}

#ifndef WITH_EASTL
	int comparei(const tstring& sOther) const
    {
		tstring s1, s2;
		s1 = *this;
		s2 = sOther;
		return s1.tolower().compare(s2.tolower());
    }
#endif
};

typedef tstring::value_type tchar;

#include "strutils.h"

#endif
