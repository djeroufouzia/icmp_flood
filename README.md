# ICMP Raw Socket Programming in C

## Overview

This project explores low-level network programming in C using Linux raw sockets. It demonstrates two approaches to generating ICMP Echo Request packets:

- **ICMP Flood (`icmp_flood.c`):** Generates ICMP Echo Request messages while allowing the Linux kernel to construct the IPv4 header.
- **ICMP Packet Construction (`icmp_flood_spoofed.c`):** Constructs the IPv4 and ICMP headers in application code and uses `IP_HDRINCL` to supply the IPv4 header to the kernel.

**Important distinction:** The second implementation studies manual IPv4 header construction. Its current source and destination addresses are both `127.0.0.1`, so it does not demonstrate spoofing a different source IP address.

## Technologies

- **Programming language:** C
- **Operating system:** Linux
- **Network layer:** IPv4
- **Transport/control protocol:** ICMP
- **Networking interface:** Raw sockets
- **System APIs:** POSIX sockets and Linux-specific raw socket options
- **Compilation:** GCC
- **Packet analysis:** Wireshark and tcpdump

## Repository Structure

```text
icmp_flood/
├── icmp_flood.c
├── icmp_flood_spoofed.c
├── README.md
└── .gitignore
```

## Implementation 1 — ICMP Flood

**Source file:** `icmp_flood.c`

### Objective

Explore ICMP Echo Request generation through a raw socket and understand how an application interacts with the Linux networking stack.

### How It Works

1. Creates a raw socket using `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`.
2. Configures the destination IPv4 address using `struct sockaddr_in`.
3. Initializes an ICMP Echo Request with type `ICMP_ECHO` and code `0`.
4. Sets the ICMP identifier and sequence number.
5. Calculates the ICMP checksum.
6. Sends the ICMP message using `sendto()`.
7. Increments the sequence number and repeats the operation.

### IPv4 Header Handling

The application supplies the ICMP message, while Linux normally constructs the IPv4 header.

### Current Configuration

- **Destination address:** `192.168.56.8`
- **ICMP identifier:** `1234`
- **ICMP message:** 8-byte header without an additional payload
- **Loop behavior:** Infinite loop
- **Delay between send attempts:** `usleep(100)`, requesting a delay of 100 microseconds

The destination must correspond to an authorized and reachable lab host. The configured delay does not guarantee a specific transmission rate.

## Implementation 2 — Manual IPv4 and ICMP Packet Construction

**Source file:** `icmp_flood_spoofed.c`

### Objective

Study how an application can prepare an IPv4 header, an ICMP Echo Request header, and a payload before submitting the complete packet through a raw socket.

### How It Works

1. Creates a raw socket using `socket(AF_INET, SOCK_RAW, IPPROTO_RAW)`.
2. Enables `IP_HDRINCL` using `setsockopt()`.
3. Initializes the IPv4 header using Linux's `struct iphdr`.
4. Configures the IPv4 version, header length, TTL, and protocol.
5. Initializes the ICMP Echo Request header.
6. Appends the payload to the ICMP message.
7. Calculates the ICMP checksum over the ICMP header and payload.
8. Combines the IPv4 header and ICMP message into a packet buffer.
9. Submits the packet using `sendto()`.

### IPv4 Header Handling

Unlike the first implementation, this program supplies an IPv4 header constructed in application code. On Linux, `IP_HDRINCL` tells the kernel that the application includes the IPv4 header in the submitted packet.

The program sets the IPv4 total length and initializes the IPv4 checksum field to zero. Linux raw-socket behavior includes kernel handling of the IPv4 header checksum and total length when `IP_HDRINCL` is enabled.

### Current Configuration

- **Source address:** `192.168.56.8`
- **Destination address:** `192.168.56.7`
- **TTL:** `64`
- **IPv4 protocol:** `IPPROTO_ICMP`
- **ICMP identifier:** `1`
- **Payload:** `Hello from my lab :) `
- **Loop behavior:** Infinite loop
- **Delay between send attempts:** `sleep(1)`, requesting a one-second delay

### Note on Source IP Spoofing

Although the source filename contains `spoofed`, the current implementation configures both the source and destination as loopback addresses.

Consequently, this version demonstrates manual IPv4 header construction, but does not demonstrate using a different source address. Manual header construction and source IP spoofing are related but distinct concepts.

## Comparison

| Feature | `icmp_flood.c` | `icmp_flood_spoofed.c` |
|---|---|---|
| Socket type | Raw ICMP socket | Raw IP socket |
| Socket protocol | `IPPROTO_ICMP` | `IPPROTO_RAW` |
| `IP_HDRINCL` | Not used | Enabled |
| IPv4 header | Normally constructed by Linux | Prepared by the application |
| ICMP header | Prepared by the application | Prepared by the application |
| Additional payload | None | Present |
| ICMP checksum | Calculated by the application | Calculated over the ICMP header and payload |
| Source address configuration | Normally selected by the networking stack | Explicitly set to `127.0.0.1` |
| Requested delay | 100 microseconds | 1 second |

## Compilation

Compile the first implementation on a compatible Linux system:

```bash
gcc -Wall -Wextra -o icmp_flood icmp_flood.c
```

Compile the second implementation:

```bash
gcc -Wall -Wextra -o icmp_flood_spoofed icmp_flood_spoofed.c
```

Raw sockets generally require root privileges or suitable network capabilities on Linux.


## Packet Analysis

Wireshark and tcpdump can help inspect:

- IPv4 version and header length
- IPv4 source and destination addresses
- TTL and protocol fields
- ICMP type and code
- ICMP identifier and sequence number
- ICMP payload
- IPv4 and ICMP checksum validity
- Differences between kernel-generated and application-supplied IPv4 headers


## Learning Objectives

This project provides practical experience with:

- Raw socket programming in C
- IPv4 and ICMP packet structure
- ICMP Echo Request generation
- Manual header construction
- Internet checksum calculation
- Host byte order and network byte order
- Linux raw socket configuration
- Packet inspection and troubleshooting
- Differences between kernel-managed and application-supplied headers

## Limitations

- The source code uses Linux-specific structures such as `struct iphdr` and `struct icmphdr`.
- macOS may require changes to header definitions and socket handling.
- The checksum implementation and resulting packet bytes should be validated on the target platform.

