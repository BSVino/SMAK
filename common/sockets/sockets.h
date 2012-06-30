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

#ifndef LW_SOCKETS_H
#define LW_SOCKETS_H
#ifdef _WIN32
#pragma once
#endif

#include <stdio.h>

#include <tstring.h>
#include <tvector.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/wait.h>
#include <sys/socket.h>
#include <wctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif // _WIN32

#ifndef _WIN32
#define SOCKET int
#define SOCKADDR struct sockaddr
#define SOCKET_ERROR -1
#endif

class CClientSocket
{
public:
					CClientSocket(const char* pszHostname, int iPort);
					~CClientSocket();

	virtual void	Initialize();
	virtual bool	Connect(const char* pszHostname, int iPort);

	virtual bool	IsOpen() { return m_bOpen; };

	virtual int		Send(const char* pszData, size_t iLength);
	virtual int		Send(const tstring& sData);
	virtual int		Send(const char* pszData, int iLength);
	virtual int		Recv(char* pszData, int iLength);
	virtual tstring	RecvAll();

	virtual void	Close();

	inline SOCKET	GetSocket() { return m_iSocket; };

	inline tstring	GetError() { return m_sError; };

protected:
	SOCKET			m_iSocket;
	tstring			m_sError;

	bool			m_bOpen;

#ifdef _WIN32
	WSADATA			m_WSAData;
#endif

	tstring			m_sHostname;
	int				m_iPort;
};

class CPostReply
{
public:
	CPostReply(const tstring& sKey, const tstring& sValue)
	{
		m_sKey = sKey;
		m_sValue = sValue;
	}

public:
	tstring		m_sKey;
	tstring		m_sValue;
};

class CHTTPPostSocket : public CClientSocket
{
public:
					CHTTPPostSocket(const char* pszHostname, int iPort = 80);

	virtual void	SendHTTP11(const char* pszPage);

	virtual void	AddPost(const char* pszKey, const tstring&);
	virtual void	SetPostContent(const tstring& sPostContent);

	virtual void	ParseOutput();
	virtual void	KeyValue(const char* pszKey, const char* pszValue);

	size_t			GetNumReplies() { return m_aKeys.size(); }
	CPostReply*		GetReply(size_t i) { return &m_aKeys[i]; }

protected:
	tstring			m_sPostContent;

	tvector<CPostReply>	m_aKeys;
};

#endif
