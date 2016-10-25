/******************************************************************************

Copyright (c) 2014, Oliver Schmidt
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL OLIVER SCHMIDT BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef _STREAM_H_
#define _STREAM_H_

typedef unsigned char  byte;
typedef unsigned short word;

word stream_data_request(byte do_send);
void stream_data_commit(byte do_send, word size);

// After stream_receive_request() every read operation returns the next byte
// from the server.
// After stream_send_request() every write operation prepares the next byte
// to be sent to the server.
extern volatile byte* stream_data;

// Initialize W5100 Ethernet controller with indirect bus interface located
// at <base_addr>. Use <ip_addr>, <submask> and <gateway> to configure the
// TCP/IP stack.
// Return <1> if a W5100 was found at <base_addr>, return <0> otherwise.
byte stream_init(word base_addr, byte *ip_addr,
                                 byte *submask,
                                 byte *gateway);

// Connect to server with IP address <server_addr> on TCP port <server_port>.
// Use <6502> as fixed local port.
// Return <1> if the connection is established, return <0> otherwise.
byte stream_connect(byte *server_addr, word server_port);

// Check if still connected to server.
// Return <1> if the connection is established, return <0> otherwise.
byte stream_connected(void);

// Disconnect from server.
void stream_disconnect(void);

// Request to receive data from the server.
// Return maximum number of bytes to be received by reading from *stream_data.
#define stream_receive_request() stream_data_request(0)

// Commit receiving of <size> bytes from server. <size> may be smaller than
// the return value of stream_receive_request(). Not commiting at all just
// makes the next request receive the same data again.
#define stream_receive_commit(size) stream_data_commit(0, (size))

// Request to send data to the server.
// Return maximum number of bytes to be send by writing to *stream_data.
#define stream_send_request() stream_data_request(1)

// Commit sending of <size> bytes to server. <size> is usually smaller than
// the return value of stream_send_request(). Not commiting at all just turns
// the stream_send_request() - and the writes to *stream_data - into NOPs.
#define stream_send_commit(size) stream_data_commit(1, (size))

#endif
