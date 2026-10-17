# ICMP Flood — Raw Socket Programming in C

## Overview

This project demonstrates low-level network programming in C using raw sockets and the Internet Control Message Protocol (ICMP).

The program creates a raw socket, constructs ICMP Echo Request packets, calculates the ICMP checksum, and sends packets to a specified IPv4 destination.



## Learning Objectives

- Understand raw sockets in C.
- Learn how to create a socket using `socket()`.
- Work with IPv4 addresses using `struct sockaddr_in`.
- Convert IPv4 addresses using `inet_pton()`.
- Construct ICMP Echo Request packets using `struct icmphdr`.
- Understand ICMP packet fields, including type, code, identifier, sequence number, and checksum.
- Implement the ICMP checksum calculation.
- Send packets using `sendto()`.

## Technologies

- **Language:** C
- **Protocol:** ICMP
- **Network layer:** IPv4
- **APIs:** POSIX sockets
- **Headers:** `sys/socket.h`, `netinet/in.h`, `netinet/ip_icmp.h`, `arpa/inet.h`

## How It Works

The program follows these steps:

1. **Create a raw socket** using `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`.
2. **Configure the destination address** using `struct sockaddr_in`.
3. **Initialize an ICMP Echo Request** with type `ICMP_ECHO` and code `0`.
4. **Set the identifier and sequence number** for each packet.
5. **Calculate the ICMP checksum** to verify the integrity of the ICMP message.
6. **Send the packet** using `sendto()`.
7. **Increment the sequence number** and repeat the operation.

The current implementation sends only the ICMP header, without additional payload data, and does not receive or process Echo Reply packets.

## Code Structure

```text
icmp_flood/
├── icmp_flood.c
├── README.md
└── .gitignore
```

## Configuration

The destination IPv4 address is currently configured in the source code:

```c
inet_pton(AF_INET, "192.168.56.8",
          &dest_addr.sin_addr);
```

`192.168.56.8` is a private IPv4 address commonly used in local virtual machine networks. The destination must be reachable in your lab environment.

## Compilation

On a compatible Linux system with GCC:

```bash
gcc -Wall -Wextra -o icmp_flood icmp_flood.c
```

Raw sockets generally require root privileges or the appropriate network capabilities on Linux.

**Note:** Raw socket support and privilege requirements differ across operating systems. The code uses Linux-specific ICMP header definitions and may require adjustments to compile on macOS.

## Execution and Safety

The current code contains an infinite loop and uses `usleep(100)`, resulting in a very short delay between send attempts.
