# src/common/port_bytes.c

This file writes the address of the loopback, and reads a port of TCP, in the order of the bytes of the network.
`src/posix/port.c` and `src/windows/port.c` call it.

## The order

The network gives the first byte the most weight. The file writes and reads the bytes one at a time, so the order of the bytes of the processor does not count.
`htonl` and `ntohs` do the same, but on macOS they are macros with a choice in them, which `test --branches` cannot measure.

## Functions

| Function | Result |
| --- | --- |
| `os_set_loopback(address)` | Writes `127.0.0.1` into the 4 bytes of `address`. |
| `os_get_port(port)` | Gives the port of the 2 bytes of `port`. |
