﻿/*
* Copyright 2011 Wolfgang Keller
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "DNS/DNS.h"
#include <winsock2.h>
#include <assert.h>
#include "MiniStdlib/cstdint.h"
#include "MiniStdlib/cstring.h"
#include "MiniStdlib/cstdbool.h"

#pragma pack(push, 1)

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4214)
#endif

typedef struct
{
	uint16_t ID;
	uint16_t QR : 1;
	uint16_t OPCODE : 4;
	uint16_t AA : 1;
	uint16_t TC : 1;
	uint16_t RD : 1;
	uint16_t RA : 1;
	uint16_t Z : 3;
	uint16_t RCODE : 4;
	uint16_t QDCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
} Header;

#ifdef _WIN32
#pragma warning(pop)
#endif

#pragma pack(pop)

void fillHeader(Header *out_pHeader)
{
	memset(out_pHeader, 0, sizeof(Header));

	out_pHeader->ID = 0;     // TODO: Change
	out_pHeader->QR = 0;     // Query
	out_pHeader->OPCODE = 0; // 0 a standard query (QUERY)
	                         // 1  an inverse query (IQUERY)
	                         // 2  a server status request (STATUS)
	/*
	* RFC 1035 - 4.1.1. Header section format
	* "Recursion Desired - this bit may be set in a query and
	* is copied into the response.  If RD is set, it directs
	* the name server to pursue the query recursively.
	* Recursive query support is optional."
	*/
	out_pHeader->RD = 1;

	/*
	* "QDCOUNT an unsigned 16 bit integer specifying the number of 
	* entries in the question section."
	*/
	out_pHeader->QDCOUNT = 1;
}

int prepareQNAME(char *in_out_preQNAME)
{
	/*
	* Automaton for parsing QNAMEs (not including that each label must not
	* be larger than 256 octets).
	*
	*
	*  != '.', 0
	*    ┌───┐
	*    │   V
	*   ┌─────┐    '.'     ┌─────┐
	*   │     │───────────>│     │
	*   │  1  │	           │  0  │<
	*   │     │<───────────│     │
	*   └─────┘	 != '.', 0 └─────┘
	*      │                  │
	*      │ 0                │ '.'
	*      │                  │
	*      V                  V
	*   ╔═════╗          ╔═════════╗
	*   ║ OK  ║          ║ FAILURE ║
	*   ╚═════╝          ╚═════════╝
	*/
	
	char *pCurrentLength = in_out_preQNAME;
	char *pCurrentCharacterInLabel = in_out_preQNAME + 1;
	// bytesCount == 0 <=> state == 0 
	uint8_t bytesCount = 0;

	while (*pCurrentCharacterInLabel)
	{
		if ('.' == *pCurrentCharacterInLabel)
		{
			if (0 == bytesCount)
				// Failure
				return 1;
			else
			{
				*pCurrentLength = (char) bytesCount;
				bytesCount = 0;
				pCurrentLength = pCurrentCharacterInLabel;
				pCurrentCharacterInLabel++;
			}
		}
		else
		{
			if (0xFF == bytesCount)
				// Failure
				return 1;

			pCurrentCharacterInLabel++;
			bytesCount++;
		}
	}

	if (0 == bytesCount)
		return 1;
	else
	{
		*pCurrentLength = (char) bytesCount;
		return 0;
	}
}

int readDNS(const char *in_cDnsServer, const char *in_cDomain)
{
	SOCKET udpSocket;
	SOCKADDR_IN udpAddr;
	SOCKADDR_IN remoteAddr;
	int remoteAddrLen = sizeof(remoteAddr);
	int result;
	char buffer0[512];
	int buffer0Size = 0;
	char buffer1[512];
	int buffer1Size = 512;
	size_t domainLen = strlen(in_cDomain);
	
	/*
	* Comments taken from
	* http://msdn.microsoft.com/en-us/library/windows/desktop/ms740506(v=vs.85).aspx
	*/
	udpSocket = socket(
		/*
		* The Internet Protocol version 4 (IPv4) address family.
		*/
		AF_INET,
		/*
		* A socket type that supports datagrams, which are connectionless, 
		* unreliable buffers of a fixed (typically small) maximum length. This 
		* socket type uses the User Datagram Protocol (UDP) for the Internet 
		* address family (AF_INET or AF_INET6).
		*/
		SOCK_DGRAM,
		/*
		* The User Datagram Protocol (UDP). This is a possible value when 
		* the af parameter is AF_INET or AF_INET6 and the type parameter 
		* is SOCK_DGRAM.
		*/
		IPPROTO_UDP);

	if (INVALID_SOCKET == udpSocket)
	{
		return -1;
	}

	udpAddr.sin_family = AF_INET;
	udpAddr.sin_port = htons(53);
	udpAddr.sin_addr.s_addr = inet_addr(in_cDnsServer);

	if (sizeof(Header)
		+ 1 // length byte
		+ domainLen
		+ 1 // zero byte terminating the string
		+ 2 // QTYPE
		+ 2 // QNAME
		> 512)
	{
		return -1;
	}

	fillHeader((Header *) buffer0);
	
	memcpy(buffer0 + sizeof(Header) + 1, in_cDomain, domainLen + 1);
	// TODO Fill buffer

	result = sendto(udpSocket, buffer0, buffer0Size, 0, (SOCKADDR *) &udpAddr, sizeof(udpAddr));

	/*
	* To quote
	* http://msdn.microsoft.com/en-us/library/windows/desktop/ms740148(v=vs.85).aspx
	*
	* "If no error occurs, sendto returns the total number of bytes sent, 
	* which can be less than the number indicated by len. Otherwise, a value 
	* of SOCKET_ERROR is returned, and a specific error code can be retrieved 
	* by calling WSAGetLastError."
	*/
	if (SOCKET_ERROR == result)
	{
		return -1;
	}

	assert(buffer0Size == result);

	result = recvfrom(udpSocket, buffer1, sizeof(buffer1), 0, (SOCKADDR*) &remoteAddr, &remoteAddrLen);

    if (SOCKET_ERROR == result)
	{
		return -1;
	}

	closesocket(udpSocket);
	return 0;
}
