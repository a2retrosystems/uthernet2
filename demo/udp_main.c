#include <stdio.h>
#include <conio.h>

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

  printf("(U)DP or e(X)it\n");
  do
  {
    unsigned len;

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
      unsigned i;

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

    len = udp_recv_init();
    if (len)
    {
      unsigned i;

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
  }
  while (key != 'x');

  printf("Done\n");
}
