#include <stdio.h>

#ifdef _MSC_VER

#include <conio.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#define SOCKET int
#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr

#define WSAGetLastError() errno
#define WSAEWOULDBLOCK EWOULDBLOCK

#define closesocket close
#define Sleep(x) usleep(1000 * x)
#define kbhit() 1

// one must press ENTER for it to work
// and all characters will be returned (inclusing ENTER)
char getch()
{
  int c = getchar();
  // it return EOF = -1 on error, or a unsigned char
  return c >= 0 ? c : 0;
}

#endif

static void dump(unsigned char *buf, unsigned len)
{
  unsigned i;

  for (i = 0; i < len; ++i)
  {
    if ((i % 24) == 0)
    {
      printf("\n$%04X:", i);
    }
    printf(" %02X", buf[i]);
  }
  printf(".\n");
}

static int set_non_blocking_flag(SOCKET s)
{
#ifdef _MSC_VER
  u_long arg = 1;
  if (ioctlsocket(s, FIONBIO, &arg) == SOCKET_ERROR)
  {
    return SOCKET_ERROR;
  }
  return 0;
#else
  return fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
#endif
}

void main(void)
{
  printf("Init\n");

#ifdef _MSC_VER
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
  {
    return;
  }
#else
  // STDIN non blocking
  set_non_blocking_flag(STDIN_FILENO);
#endif

  SOCKET udp = socket(AF_INET, SOCK_DGRAM , IPPROTO_UDP);
  if (udp == INVALID_SOCKET)
  {
    return;
  }
  SOCKET srv = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP);
  if (srv == INVALID_SOCKET)
  {
    return;
  }

  if (set_non_blocking_flag(udp) == SOCKET_ERROR)
  {
    return;
  }
  if (set_non_blocking_flag(srv) == SOCKET_ERROR)
  {
    return;
  }

  SOCKADDR_IN local;
  local.sin_family      = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port        = htons(6502);
  if (bind(udp, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
  {
    return;
  }
  if (bind(srv, (SOCKADDR *)&local, sizeof(local)) == SOCKET_ERROR)
  {
    return;
  }

  if (listen(srv, 1) == SOCKET_ERROR)
  {
    return;
  }

  SOCKADDR_IN remote;
  remote.sin_addr.s_addr = INADDR_NONE;

  SOCKET tcp = INVALID_SOCKET;

  printf("(U)DP, (T)CP or e(X)it\n");
  char key;
  do
  {
    int len;
    unsigned char buf[1500];

    if (kbhit())
    {
      key = getch();
    }
    else
    {
      key = '\0';
    }

    if (key == 'u')
    {
      if (remote.sin_addr.s_addr == INADDR_NONE)
      {
        printf("Peer Unknown As Yet\n", len);
      }
      else
      {
        unsigned i;

        len = 500;
        for (i = 0; i < len; ++i)
        {
          buf[i] = i;
        }
        printf("Send Len %d To %s", len, inet_ntoa(remote.sin_addr));
        if (sendto(udp, buf, len, 0, (SOCKADDR *)&remote, sizeof(remote)) == SOCKET_ERROR)
        {
          return;
        }
        printf(".\n");
      }
    }

    if (key == 't')
    {
      if (tcp == INVALID_SOCKET)
      {
        printf("No Connection\n", len);
      }
      else
      {
        unsigned i;

        len = 500;
        for (i = 0; i < len; ++i)
        {
          buf[i] = i;
        }
        printf("Send Len %d", len);
        if (send(tcp, buf, len, 0) == SOCKET_ERROR)
        {
          return;
        }
        printf(".\n");
      }
    }

    unsigned remote_size = sizeof(remote);
    len = recvfrom(udp, buf, sizeof(buf), 0, (SOCKADDR *)&remote, &remote_size);
    if (len == SOCKET_ERROR)
    {
      if (WSAGetLastError() != WSAEWOULDBLOCK)
      {
        return;
      }
    }
    else if (len)
    {
      printf("Recv Len %d From %s", len, inet_ntoa(remote.sin_addr));
      dump(buf, len);
    }

    if (tcp == INVALID_SOCKET)
    {
      SOCKADDR_IN conn;
      unsigned conn_size = sizeof(conn);
      tcp = accept(srv, (SOCKADDR *)&conn, &conn_size);
      if (tcp == INVALID_SOCKET)
      {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
          return;
        }
      }
      else
      {
        printf("Connect From %s\n", inet_ntoa(conn.sin_addr));

        if (set_non_blocking_flag(tcp) == SOCKET_ERROR)
        {
          return;
        }
      }
    }
    else
    {
      len = recv(tcp, buf, sizeof(buf), 0);
      if (len == SOCKET_ERROR)
      {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
          return;
        }
      }
      else if (len)
      {
        printf("Recv Len %d", len);
        dump(buf, len);
      }
      else
      {
        printf("Disconnect\n");
        closesocket(tcp);
        tcp = INVALID_SOCKET;
      }
    }

    Sleep(10);
  }
  while (key != 'x');

  closesocket(udp);
  closesocket(tcp);
  closesocket(srv);

#ifdef _MSC_VER
  WSACleanup();
#endif

  printf("Done\n");
}
