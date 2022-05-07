#include <stdio.h>
#include <conio.h>
#include <ctype.h>

unsigned char __fastcall__ tcp_init(void *parms);
void                       tcp_done(void);

int           tcp_recv_init(void);
unsigned char tcp_recv_byte(void);
void          tcp_recv_done(void);

unsigned char __fastcall__ tcp_send_init(unsigned int  len);
void          __fastcall__ tcp_send_byte(unsigned char val);
void                       tcp_send_done(void);

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
  if (!tcp_init(&parms))
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

  printf("(T)CP or e(X)it\n");
  do
  {
    unsigned len;

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
      unsigned i;

      len = 500;
      printf("Send Len %d", len);
      while (!tcp_send_init(len))
      {
        printf("!");
      }
      for (i = 0; i < len; ++i)
      {
        tcp_send_byte(i);
      }
      tcp_send_done();
      printf(".\n");
    }

    len = tcp_recv_init();
    if (len == -1)
    {
      printf("Disconnect\n");
      return;
    }
    else if (len)
    {
      unsigned i;

      printf("Recv Len %d", len);
      for (i = 0; i < len; ++i)
      {
        if ((i % 24) == 0)
        {
          printf("\n$%04X:", i);
        }
        printf(" %02X", tcp_recv_byte());
      }
      tcp_recv_done();
      printf(".\n");
    }
  }
  while (key != 'x');

  tcp_done();
  printf("Done\n");
}
