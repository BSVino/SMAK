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

#include "sockets.h"

#include "strutils.h"

#ifdef __GNUC__
#define INVALID_SOCKET -1
#endif

CClientSocket::CClientSocket(const char* pszHostname, int iPort)
{
	m_sHostname = pszHostname;
	m_iPort = iPort;
	m_bOpen = false;

	Initialize();

	if (!Connect(m_sHostname.c_str(), m_iPort))
		Close();
}

CClientSocket::~CClientSocket()
{
	Close();
}

void CClientSocket::Initialize()
{
	m_iSocket = -1;
	m_sError = "";

#ifdef _WIN32
    if(WSAStartup(MAKEWORD(2,0), &m_WSAData) !=0)
    {
		m_sError = "WSAStartup failed.";
        return;
    }
#endif
}

bool CClientSocket::Connect(const char* pszHostname, int iPort)
{
	struct addrinfo hints;
	struct addrinfo *pResult;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	tstring sPort = sprintf("%d", iPort);

	int iResult = getaddrinfo(pszHostname, sPort.c_str(), &hints, &pResult);
	if ( iResult != 0 )
	{
		m_sError = "getaddrinfo() failed.";
		return false;
	}

	for (struct addrinfo *pCurrent = pResult; pCurrent != NULL; pCurrent=pCurrent->ai_next)
	{
		m_iSocket = socket(pCurrent->ai_family, pCurrent->ai_socktype, pCurrent->ai_protocol);
		if (m_iSocket == INVALID_SOCKET)
		{
			m_sError = "socket() failed";
#ifdef _WIN32
			WSACleanup();
#endif
			return false;
		}

		// Connect to server.
		iResult = connect( m_iSocket, (struct sockaddr *)pCurrent->ai_addr, (int)pCurrent->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			m_sError = "connect() failed";
#ifdef _WIN32
			closesocket(m_iSocket);
#else
			shutdown(m_iSocket, SHUT_RDWR);
			close(m_iSocket);
#endif
			iResult = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(pResult);

	m_bOpen = true;
	return true;
}

int CClientSocket::Send(const char* pszData, size_t iLength)
{
	return Send(pszData, (int)iLength);
}

int CClientSocket::Send(const tstring& sData)
{
	return (Send(sData.c_str(), (size_t)sData.length()));
}

int CClientSocket::Send(const char* pszData, int iLength)
{
	return (send(GetSocket(), pszData, iLength, 0));
}

int CClientSocket::Recv(char* pszData, int iLength)
{
	int iResult = recv(GetSocket(), pszData, iLength, 0);

	if (iResult < iLength)
		pszData[iResult] = '\0';

	return iResult;
}

tstring CClientSocket::RecvAll()
{
	tstring sOutput;
	char szBuffer[1028];
	int iResult;

	while ((iResult = Recv(szBuffer, sizeof(szBuffer))) > 0)
	{
		sOutput.append(szBuffer);

		if (iResult < 1000)
			break;
	}
	return sOutput;
}

void CClientSocket::Close()
{
	if (!m_bOpen)
		return;

#ifdef _WIN32
	closesocket(GetSocket());
#else
	shutdown(GetSocket(), SHUT_RDWR);
	close(GetSocket());
#endif

	m_iSocket = -1;

	m_bOpen = false;
}

CHTTPPostSocket::CHTTPPostSocket(const char* pszHostname, int iPort) : CClientSocket(pszHostname, iPort)
{
}

void CHTTPPostSocket::AddPost(const char* pszKey, const tstring& pszValue)
{
	tstring sPostContent = m_sPostContent;

	if (sPostContent.length())
		sPostContent += "&";

	sPostContent += pszKey;
	sPostContent += "=";
	sPostContent += pszValue;

	SetPostContent(sPostContent);
}

void CHTTPPostSocket::SendHTTP11(const char* pszPage)
{
	tstring p;
	tstring sOutput;

	sOutput  = tstring("POST ") + pszPage + " HTTP/1.1\n";
	sOutput += "Host: " + m_sHostname + "\n";
	sOutput += sprintf("Content-Length: %d\n", m_sPostContent.length());
	sOutput += "Content-Type: application/x-www-form-urlencoded\n\n";
	Send(sOutput);

	Send(m_sPostContent);
}

void CHTTPPostSocket::SetPostContent(const tstring& sPostContent)
{
	m_sPostContent = sPostContent;
}

void CHTTPPostSocket::ParseOutput()
{
	tstring sReturn = RecvAll();
	bool bHTTPOver = false;
	tvector<tstring> vTokens;

	strtok(sReturn, vTokens, "\n");
	for (unsigned int i = 1; i < vTokens.size(); i++)
	{
		tstring sToken = vTokens[i];
		if (!trim(sToken).length())
		{
			bHTTPOver = true;
			continue;
		}

		if (!bHTTPOver)
			continue;

		tstring::size_type iColon = sToken.find(":");
		KeyValue(sToken.substr(0, iColon).c_str(), sToken.substr(iColon+2).c_str());
	}
}

void CHTTPPostSocket::KeyValue(const char* pszKey, const char* pszValue)
{
	m_aKeys.push_back(CPostReply(pszKey, pszValue));
}
