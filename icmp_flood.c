#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

/* Calculate ICMP checksum */
uint16_t calculate_checksum(const void *data, size_t len)
{
    const unsigned char *bytes = data;
    uint32_t sum = 0;

    while (len >= 2)
    {
        uint16_t word = ((uint16_t)bytes[0] << 8) | bytes[1];
        sum += word;

        bytes += 2;
        len -= 2;
    }

    if (len == 1)
    {
        sum += (uint16_t)bytes[0] << 8;
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

int main(void)
{
    int sockfd;

    /* 1. Create raw socket */
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sockfd == -1)
    {
        perror("socket (Did you run with sudo?)");
        return 1;
    }

    printf("Raw socket created successfully!\n");

    /* 2. Prepare destination address */
    struct sockaddr_in dest_addr = {0};

    dest_addr.sin_family = AF_INET;
   /* Set the destination IPv4 address */
    if (inet_pton(AF_INET, "192.168.56.8",
                  &dest_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid destination IP address\n");
        close(sockfd);
        return 1;
    }

    /* 3. Infinite Loop */
    int i = 1;
    while (1)
    {
        struct icmphdr packet = {0};

        packet.type = ICMP_ECHO;
        packet.code = 0;
        packet.un.echo.id = htons(1234);
        packet.un.echo.sequence = htons(i);
        packet.checksum = 0;

        /* Calculate checksum */
        size_t packet_len = sizeof(packet);
        packet.checksum = calculate_checksum(&packet, packet_len);

        /* Send ICMP packet */
        ssize_t sent = sendto(
            sockfd,
            &packet,
            packet_len,
            0,
            (struct sockaddr *)&dest_addr,
            sizeof(dest_addr)
        );

        if (sent == -1)
        {
            perror("sendto");
            close(sockfd);
            return 1;
        }

        printf("Packet sent to destination: %zd bytes (Seq: %d)\n", sent, i);

        /* Increment sequence number */
        i++;
        usleep(100);
    }

    /* 4. Close socket (reached only via Ctrl+C) */
    close(sockfd);

    return 0;
}
