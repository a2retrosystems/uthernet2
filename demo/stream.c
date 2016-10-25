/******************************************************************************

Copyright (c) 2015, Oliver Schmidt
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

// Both pragmas are obligatory to have cc65 generate code
// suitable to access the W5100 auto-increment registers.
#pragma optimize      (on)
#pragma static-locals (on)

#include "stream.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

static volatile byte* stream_mode;
static volatile byte* stream_addr_hi;
static volatile byte* stream_addr_lo;
       volatile byte* stream_data;

static void set_addr(word addr)
{
  *stream_addr_hi = addr >> 8;
  *stream_addr_lo = addr;
}

static byte get_byte(word addr)
{
  set_addr(addr);

  return *stream_data;
}

static void set_byte(word addr, byte data)
{
  set_addr(addr);

  *stream_data = data;
}

static word get_word(word addr)
{
  set_addr(addr);

  {
    // The variables are necessary to have cc65 generate code
    // suitable to access the W5100 auto-increment registers.
    byte data_hi = *stream_data;
    byte data_lo = *stream_data;
    return data_hi << 8 | data_lo;
  }
}

static void set_word(word addr, word data)
{
  set_addr(addr);

  {
    // The variables are necessary to have cc65 generate code
    // suitable to access the W5100 auto-increment registers.
    byte data_hi = data >> 8;
    byte data_lo = data;
    *stream_data = data_hi;
    *stream_data = data_lo;
  }
}

static void set_bytes(word addr, byte data[], word size)
{
  set_addr(addr);

  {
    word i;
    for (i = 0; i < size; ++i)
      *stream_data = data[i];
  }
}

byte stream_init(word base_addr, byte *ip_addr,
                                 byte *submask,
                                 byte *gateway)
{
  stream_mode    = (byte*)base_addr;
  stream_addr_hi = (byte*)base_addr + 1;
  stream_addr_lo = (byte*)base_addr + 2;
  stream_data    = (byte*)base_addr + 3;

  // Assert Indirect Bus I/F mode & Address Auto-Increment
  *stream_mode |= 0x03;

  // Retry Time-value Register: Default ?
  if (get_word(0x0017) != 2000)
    return 0;

  // Check for W5100 shared access
  // RX Memory Size Register: Assign 4+2+1+1KB to Socket 0 to 3 ?
  if (get_byte(0x001A) != 0x06)
  {
    // S/W Reset
    *stream_mode = 0x80;
    while (*stream_mode & 0x80)
      ;

    // Indirect Bus I/F mode & Address Auto-Increment
    *stream_mode = 0x03;

    // RX Memory Size Register: Assign 4KB each to Socket 0 and 1
     set_byte(0x001A, 0x0A);

    // TX Memory Size Register: Assign 4KB each to Socket 0 and 1
     set_byte(0x001B, 0x0A);

    // Source Hardware Address Register
    {
      static byte mac_addr[6] = {0x00, 0x08, 0xDC, // OUI of WIZnet
                                 0x11, 0x11, 0x11};
      set_bytes(0x0009, mac_addr, sizeof(mac_addr));
    }
  }

  // Source IP Address Register
  set_bytes(0x000F, ip_addr, 4);

  // Subnet Mask Register
  set_bytes(0x0005, submask, 4);

  // Gateway IP Address Register
  set_bytes(0x0001, gateway, 4);

  return 1;
}

byte stream_connect(byte *server_addr, word server_port)
{
  // Socket 0 Mode Register: TCP
  set_byte(0x0400, 0x01);

  // Socket 0 Source Port Register
  set_word(0x0404, 6502);

  // Socket 0 Command Register: OPEN
  set_byte(0x0401, 0x01);

  // Socket 0 Status Register: SOCK_INIT ?
  while (get_byte(0x0403) != 0x13)
    ;

  // Socket 0 Destination IP Address Register
  set_bytes(0x040C, server_addr, 4);

  // Socket 0 Destination Port Register
  set_word(0x0410, server_port);

  // Socket 0 Command Register: CONNECT
  set_byte(0x0401, 0x04);

  while (1)
  {
    // Socket 0 Status Register
    switch (get_byte(0x0403))
    {
      case 0x00: return 0; // Socket Status: SOCK_CLOSED
      case 0x17: return 1; // Socket Status: SOCK_ESTABLISHED
    }
  }
}

byte stream_connected(void)
{
  // Socket 0 Status Register: SOCK_ESTABLISHED ?
  return get_byte(0x0403) == 0x17;
}

void stream_disconnect(void)
{
  // Socket 0 Command Register: Command Pending ?
  while (get_byte(0x0401))
    ;

  // Socket 0 Command Register: DISCON
  set_byte(0x0401, 0x08);

  // Socket 0 Status Register: SOCK_CLOSED ?
  while (get_byte(0x0403))
    // Wait for disconnect to allow for reconnect
    ;
}

word stream_data_request(byte do_send)
{
  // Socket 0 Command Register: Command Pending ?
  if (get_byte(0x0401))
    return 0;

  {
    word size = 0;
    word prev_size;

    // Reread of nonzero RX Received Size Register / TX Free Size Register
    // until its value settles ...
    // - is present in the WIZnet driver - getSn_RX_RSR() / getSn_TX_FSR()
    // - was additionally tested on 6502 machines to be actually necessary
    do
    {
      prev_size = size;
      {
        static word reg[2] = {0x0426,  // Socket 0 RX Received Size Register
                              0x0420}; // Socket 0 TX Free     Size Register
        size = get_word(reg[do_send]);
      }
    }
    while (size != prev_size);

    if (!size)
      return 0;

    {
      static word reg[2] = {0x0428,  // Socket 0 RX Read  Pointer Register
                            0x0424}; // Socket 0 TX Write Pointer Register

      static word bas[2] = {0x6000,  // Socket 0 RX Memory Base
                            0x4000}; // Socket 0 TX Memory Base

      static word lim[2] = {0x7000,  // Socket 0 RX Memory Limit
                            0x5000}; // Socket 0 TX Memory Limit

      // Calculate and set physical address
      word addr = get_word(reg[do_send]) & 0x0FFF | bas[do_send];
      set_addr(addr);

      // Access to *stream_data is limited both by ...
      // - size of received / free space
      // - end of physical address space
      return MIN(size, lim[do_send] - addr);
    }
  }
}

void stream_data_commit(byte do_send, word size)
{
  {
    static word reg[2] = {0x0428,  // Socket 0 RX Read  Pointer Register
                          0x0424}; // Socket 0 TX Write Pointer Register
    set_word(reg[do_send], get_word(reg[do_send]) + size);
  }

  {
    static byte cmd[2] = {0x40,  // Socket Command: RECV
                          0x20}; // Socket Command: SEND
    // Socket 0 Command Register
    set_byte(0x0401, cmd[do_send]);
  }

  // Do NOT wait for command completion here, rather
  // let W5100 operation overlap with 6502 operation
}
