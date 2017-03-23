Production Test
===============

test.c
------
A TCP client to be run on an Apple II containing the Uthernet II card under
test in slot 3. If the program is run automatically after booting the Apple
II neither needs a keyboard nor a monitor. Therefore the program beeps both
on startup and shutdown. The program echos all data back to the server.

test.py
-------
A TCP server to be run on a peer machine with the IPv4 address 192.168.0.1.
After connected by the client the program sends 100kB of random data to the
client waits until the client echos the data. If the received data differs
from the sent data or if no data is transmitted for more than 1 second the
test fails. For production usage run the program with stderr redirected to
the null device.
