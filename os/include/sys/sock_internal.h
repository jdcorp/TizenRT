/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * include/sys/sock_internal.h
 *
 *   Copyright (C) 2007, 2009, 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_SYS_SOCK_INTERNAL_H
#define __INCLUDE_SYS_SOCK_INTERNAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <sys/types.h>

/* Note that,
   This header is only used to when CONFIG_NET_LWIP is not used
 */
#ifndef CONFIG_NET_LWIP

/****************************************************************************
 * Definitions
 ****************************************************************************/

/* The socket created by socket() has the indicated type, which specifies
 * the communication semantics.
 */

#define SOCK_STREAM    0		/* Provides sequenced, reliable, two-way, connection-based byte streams.
								 * An  out-of-band data transmission mechanism may be supported. */
#define SOCK_DGRAM     1		/* Supports  datagrams (connectionless, unreliable messages of a fixed
								 * maximum length). */
#define SOCK_SEQPACKET 2		/* Provides a sequenced, reliable, two-way connection-based data
								 * transmission path for datagrams of fixed maximum length; a consumer
								 * is required to read an entire packet with each read system call. */
#define SOCK_RAW       3		/* Provides raw network protocol access. */
#define SOCK_RDM       4		/* Provides a reliable datagram layer that does not guarantee ordering. */
#define SOCK_PACKET    5		/* Obsolete and should not be used in new programs */

/* Bits in the FLAGS argument to `send', `recv', et al. These are the bits
 * recognized by Linus, not all are supported by TinyAra.
 */

#define MSG_OOB        0x0001	/* Process out-of-band data.  */
#define MSG_PEEK       0x0002	/* Peek at incoming messages.  */
#define MSG_DONTROUTE  0x0004	/* Don't use local routing.  */
#define MSG_CTRUNC     0x0008	/* Control data lost before delivery.  */
#define MSG_PROXY      0x0010	/* Supply or ask second address.  */
#define MSG_TRUNC      0x0020
#define MSG_DONTWAIT   0x0040	/* Enable nonblocking IO.  */
#define MSG_EOR        0x0080	/* End of record.  */
#define MSG_WAITALL    0x0100	/* Wait for a full request.  */
#define MSG_FIN        0x0200
#define MSG_SYN        0x0400
#define MSG_CONFIRM    0x0800	/* Confirm path validity.  */
#define MSG_RST        0x1000
#define MSG_ERRQUEUE   0x2000	/* Fetch message from error queue.  */
#define MSG_NOSIGNAL   0x4000	/* Do not generate SIGPIPE.  */
#define MSG_MORE       0x8000	/* Sender will send more.  */

/* Socket options */

#define SO_DEBUG        0		/* Enables recording of debugging information (get/set).
								 * arg: pointer to integer containing a boolean value */
#define SO_ACCEPTCONN   1		/* Reports whether socket listening is enabled (get only).
								 * arg: pointer to integer containing a boolean value */
#define SO_BROADCAST    2		/* Permits sending of broadcast messages (get/set).
								 * arg: pointer to integer containing a boolean value */
#define SO_REUSEADDR    3		/* Allow reuse of local addresses (get/set)
								 * arg: pointer to integer containing a boolean value */
#define SO_KEEPALIVE    4		/* Keeps connections active by enabling the periodic transmission
								 * of messages (get/set).
								 * arg: pointer to integer containing a boolean value */
#define SO_LINGER       5		/* Lingers on a close() if data is present (get/set)
								 * arg: struct linger */
#define SO_OOBINLINE    6		/* Leaves received out-of-band data (data marked urgent) inline
								 * (get/set) arg: pointer to integer containing a boolean value */
#define SO_SNDBUF       7		/* Sets send buffer size. arg: integer value (get/set). */
#define SO_RCVBUF       8		/* Sets receive buffer size. arg: integer value (get/set). */
#define SO_ERROR        9		/* Reports and clears error status (get only).  arg: returns
								 * an integer value */
#define SO_TYPE        10		/* Reports the socket type (get only). return: int */
#define SO_DONTROUTE   11		/* Requests that outgoing messages bypass standard routing (get/set)
								 * arg: pointer to integer containing a boolean value */
#define SO_RCVLOWAT    12		/* Sets the minimum number of bytes to process for socket input
								 * (get/set). arg: integer value */
#define SO_RCVTIMEO    13		/* Sets the timeout value that specifies the maximum amount of time
								 * an input function waits until it completes (get/set).
								 * arg: struct timeval */
#define SO_SNDLOWAT    14		/* Sets the minimum number of bytes to process for socket output
								 * (get/set). arg: integer value */
#define SO_SNDTIMEO    15		/* Sets the timeout value specifying the amount of time that an
								 * output function blocks because flow control prevents data from
								 * being sent(get/set). arg: struct timeval */

/* Protocol levels supported by get/setsockopt(): */

#define SOL_SOCKET     0xfff	/* Only socket-level options supported */

/* Values for the 'how' argument of shutdown() */

#define SHUT_RD        1		/* Bit 0: Disables further receive operations */
#define SHUT_WR        2		/* Bit 1: Disables further send operations */
#define SHUT_RDWR      3		/* Bits 0+1: Disables further send and receive operations */

/****************************************************************************
 * Type Definitions
 ****************************************************************************/

/* sockaddr_storage structure. This structure must be (1) large enough to
 * accommodate all supported protocol-specific address structures, and (2)
 * aligned at an appropriate boundary so that pointers to it can be cast
 * as pointers to protocol-specific address structures and used to access
 * the fields of those structures without alignment problems
 */

struct sockaddr_storage {
	sa_family_t ss_family;		/* Address family: See AF_* definitions */
	char ss_data[14];			/* 14-bytes of address data */
};

/* The sockaddr structure is used to define a socket address which is used
 * in the bind(), connect(), getpeername(), getsockname(), recvfrom(), and
 * sendto() functions.
 */

struct sockaddr {
	sa_family_t sa_family;		/* Address family: See AF_* definitions */
	char sa_data[14];			/* 14-bytes of address data */
};

/* Used with the SO_LINGER socket option */

struct linger {
	int l_onoff;				/* Indicates whether linger option is enabled. */
	int l_linger;				/* Linger time, in seconds. */
};

#endif /* !CONFIG_NET_LWIP */

#endif							/* __INCLUDE_SYS_SOCK_INTERNAL_H */
