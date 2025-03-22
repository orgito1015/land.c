The land.c program is a simple but dangerous exploit designed to trigger a Denial of Service (DoS) condition on vulnerable systems by sending a specially crafted TCP SYN packet to the target machine. This packet causes the machine to enter a loop where the system attempts to process a packet with the same source and destination address, leading to resource exhaustion, potential crashes, or freezes. The vulnerability was discovered in 1997 and affects multiple operating systems like Windows, Linux, FreeBSD, and others.

Vulnerability:
The core issue in the Land Attack is that the target machine fails to handle the incoming packet with both the source and destination IP addresses and ports being the same. This creates a situation where the system continuously attempts to establish a connection with itself, consuming resources and potentially causing a system crash.

Key Features:
Crafted TCP SYN packet: The program constructs a TCP SYN packet that is malformed to exploit the vulnerability.

Same IP and Port for Source and Destination: The source IP, destination IP, source port, and destination port are all set to the target machine's IP address and port number.

Raw Socket Creation: The program uses raw sockets (SOCK_RAW) to send the malicious packet, which allows it to construct custom packets directly at the IP level.

Functionality:
Command-line Arguments: The program expects two arguments:

The target IP address (e.g., 127.0.0.1 for localhost).

The target port (typically port 80 for HTTP).

Host Resolution: It uses gethostbyname() to resolve the IP address of the target system from its hostname (if provided), or it directly converts the provided IP address using inet_addr().

Packet Construction:

The program first prepares the IP header using the ipheader structure, which contains the IP address and packet-related information.

It sets the TCP header using the tcpheader structure, including the source and destination ports, sequence number, flags (SYN in this case), and the window size.

Pseudohdr Construction:

A pseudohdr is constructed to be used for the TCP checksum calculation. The pseudohdr includes the source and destination IP addresses, the protocol (TCP), and the length of the TCP data.

Checksum Calculation:

The program computes the TCP checksum using the checksum() function. This checksum ensures that the packet is correctly formatted before sending it.

Packet Sending:

The crafted packet is then sent to the target machine using the sendto() function.

Result Output:

If the packet is successfully sent, the program outputs a message like: 127.0.0.1:80 landed, indicating that the attack was executed.

Code Breakdown:
Headers: The program includes several system and network headers like <stdio.h>, <netdb.h>, <arpa/inet.h>, <netinet/in.h>, and <sys/socket.h> for networking functions, socket handling, and checksum calculation.

struct pseudohdr: This structure is used for calculating the checksum, which combines the IP address information and TCP header data.

Checksum Calculation: The checksum is calculated using the checksum() function, which sums up the data in the TCP header and the pseudohdr, adjusts for overflow, and returns the one's complement of the result.

Socket Creation: The program creates a raw socket (SOCK_RAW) that allows it to send custom IP packets.

Sending Malformed Packet: The packet is crafted and sent to the target IP address using sendto(), and the program closes the socket afterward.

Potential Effects of the Attack:
Resource Exhaustion: If the target system is vulnerable and does not have protections or patches, it will attempt to process the malformed packet, causing it to loop indefinitely.

Denial of Service (DoS): The system may eventually become unresponsive due to resource exhaustion, potentially causing a crash or freeze.
