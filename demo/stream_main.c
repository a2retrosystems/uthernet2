// Both pragmas are obligatory to have cc65 generate code
// suitable to access the W5100 auto-increment registers.
#pragma optimize      (on)
#pragma static-locals (on)

#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <6502.h>

#include "stream.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

byte ip_addr[4] = {192, 168,   0, 123};
byte submask[4] = {255, 255, 255,   0};
byte gateway[4] = {192, 168,   0,   1};

byte server[4] = {192, 168, 0, 2}; // IP addr of machine running peer.c

void main(void)
{
  char key;

  videomode(VIDEOMODE_80COL);
  printf("Init\n");
  if (!stream_init(0xC0B4, ip_addr,
                           submask,
                           gateway))
  {
    printf("No Hardware Found\n");
    return;
  }
  printf("Connect\n");
  if (!stream_connect(server, 6502))
  {
    printf("Faild To Connect To %d.%d.%d.%d\n", server[0],
                                                server[1],
                                                server[2],
                                                server[3]);
    return;
  }
  printf("Connected To %d.%d.%d.%d\n", server[0],
                                       server[1],
                                       server[2],
                                       server[3]);

  printf("(T)CP or e(X)it\n");
  do
  {
    word len, all;

    if (kbhit())
    {
      key = tolower(cgetc());
    }
    else
    {
      key = '\0';
    }

    if (key == 't')
    {
      all = 500;
      printf("Send Len %d", all);
      do
      {
        word i;

#ifdef SOCKET_IRQ
        SEI();
#endif
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
#ifdef SOCKET_IRQ
        CLI();
#endif
        all -= len;
      }
      while (all);
      printf(".\n");
    }

#ifdef SOCKET_IRQ
    SEI();
#endif
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
#ifdef SOCKET_IRQ
    CLI();
#endif

#ifdef SOCKET_IRQ
    if (!stream_connected)
#else
    if (!stream_connected())
#endif
    {
      printf("Disconnect\n");
      return;
    }
  }
  while (key != 'x');

  stream_disconnect();
  printf("Done\n");
}
