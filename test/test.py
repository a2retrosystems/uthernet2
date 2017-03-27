import os, socket, sys, time

data_len = 100 * 1024
data = os.urandom(data_len)

serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serv.bind(('', 6502))
serv.listen(1)

while True:
    try:
        print 'Test Ready'
        print >>sys.stderr, 'Listen On Port 6502'
        conn, addr = serv.accept()

        print 'Test Started'
        print >>sys.stderr, 'Connected To', addr
        conn.settimeout(0.001)

        send_pos = recv_pos = 0
        watchdog = int(time.time())
        while recv_pos < data_len:
            if send_pos < data_len:
                try:
                    sent = conn.send(data[send_pos:])
                except:
                    pass
                else:
                    print >>sys.stderr, 'Send Length:', sent
                    send_pos += sent;
                    watchdog = int(time.time())

            try:
                temp = conn.recv(0x1000)
            except:
                pass
            else:
                sys.stdout.write('.')
                sys.stdout.flush()
                received = len(temp)
                print >>sys.stderr, 'Recv Length:', received
                assert temp == data[recv_pos:recv_pos+received]
                recv_pos += received;
                watchdog = int(time.time())

            assert int(time.time()) < watchdog+2

    except:
        print
        print 'Test Failure !!!'
    else:
        print
        print 'Test Success'
    finally:
        conn.close()
        print >>sys.stderr, 'Disconnected'
