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

#include <EASTL/string.h>

#include <strutils.h>

// EASTL expects us to define these, see allocator.h line 194
void* operator new[](size_t size, const char* pName, int flags,
    unsigned debugFlags, const char* file, int line)
{
    return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset,
    const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    // this allocator doesn't support alignment
    EASTL_ASSERT(alignment <= 8);
    return malloc(size);
}

// EASTL also wants us to define this (see string.h line 197)
int Vsnprintf8(char* pDestination, size_t n, const char* pFormat, va_list arguments)
{
    #ifdef _MSC_VER
        return _vsnprintf(pDestination, n, pFormat, arguments);
    #else
        return vsnprintf(pDestination, n, pFormat, arguments);
    #endif
}

int Vsnprintf16(char16_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments)
{
    #ifdef _MSC_VER
        return _vsnwprintf((wchar_t*)pDestination, n, (wchar_t*)pFormat, arguments);
    #else
		char* d = new char[n+1];
		int r = vsnprintf(d, n, convertstring<char16_t, char>(pFormat).c_str(), arguments);
		memcpy(pDestination, convertstring<char, char16_t>(d).c_str(), (n+1)*sizeof(char16_t));
		delete[] d;
		return r;
    #endif
}
