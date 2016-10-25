// Both pragmas are obligatory to have cc65 generate code
// suitable to access the W5100 auto-increment registers.
#pragma optimize      (on)
#pragma static-locals (on)

#include <stdio.h>
#include <conio.h>

#include "stream.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

void __fastcall__ udp_init(void *parms);

unsigned int  udp_recv_init(void);
unsigned char udp_recv_byte(void);
void          udp_recv_done(void);

unsigned char __fastcall__ udp_send_init(unsigned int  len);
void          __fastcall__ udp_send_byte(unsigned char val);
void                       udp_send_done(void);

struct
{
  unsigned char serverip   [4];
  unsigned char cfg_ip     [4];
  unsigned char cfg_netmask[4];
  unsigned char cfg_gateway[4];
}
parms =
{
  {192, 168,   0,   2}, // IP addr of machine running peer.c
  {192, 168,   0, 123},
  {255, 255, 255,   0},
  {192, 168,   0,   1}
};

void main(void)
{
  char key;

  videomode(VIDEOMODE_80COL);
  printf("Init\n");
  udp_init(&parms);
  if (!stream_init(0xC0B4, parms.cfg_ip,
                           parms.cfg_netmask,
                           parms.cfg_gateway))
  {
    printf("No Hardware Found\n");
    return;
  }
  printf("Connect\n");
  if (!stream_connect(parms.serverip, 6502))
  {
    printf("Faild To Connect To %d.%d.%d.%d\n", parms.serverip[0],
                                                parms.serverip[1],
                                                parms.serverip[2],
                                                parms.serverip[3]);
    return;
  }
  printf("Connected To %d.%d.%d.%d\n", parms.serverip[0],
                                       parms.serverip[1],
                                       parms.serverip[2],
                                       parms.serverip[3]);

  printf("(U)DP, (T)CP or e(X)it\n");
  do
  {
    word len, all;

    if (kbhit())
    {
      key = cgetc();
    }
    else
    {
      key = '\0';
    }

    if (key == 'u')
    {
      word i;

      len = 500;
      printf("Send Len %d To %d.%d.%d.%d", len, parms.serverip[0],
                                                parms.serverip[1],
                                                parms.serverip[2],
                                                parms.serverip[3]);
      while (!udp_send_init(len))
      {
        printf("!");
      }
      for (i = 0; i < len; ++i)
      {
        udp_send_byte(i);
      }
      udp_send_done();
      printf(".\n");
    }

    if (key == 't')
    {
      all = 500;
      printf("Send Len %d", all);
      do
      {
        word i;

        while (!(len = stream_send_request()))
        {
          printf("!");
        }
        len = MIN(all, len);
        for (i = 0; i < len; ++i)
        {
          *stream_data = 500 - all + i;
        }
        stream_send_commit(len);
        all -= len;
      }
      while (all);
      printf(".\n");
    }

    len = udp_recv_init();
    if (len)
    {
      word i;

      printf("Recv Len %d From %d.%d.%d.%d", len, parms.serverip[0],
                                                  parms.serverip[1],
                                                  parms.serverip[2],
                                                  parms.serverip[3]);
      for (i = 0; i < len; ++i)
      {
        if ((i % 24) == 0)
        {
          printf("\n$%04X:", i);
        }
        printf(" %02X", udp_recv_byte());
      }
      udp_recv_done();
      printf(".\n");
    }

    len = stream_receive_request();
    if (len)
    {
      word i;

      printf("Recv Len %d", len);
      for (i = 0; i < len; ++i)
      {
        if ((i % 24) == 0)
        {
          printf("\n$%04X:", i);
        }
        printf(" %02X", *stream_data);
      }
      stream_receive_commit(len);
      printf(".\n");
    }

    if (!stream_connected())
    {
      printf("Disconnect\n");
      return;
    }
  }
  while (key != 'x');

  stream_disconnect();
  printf("Done\n");
}
