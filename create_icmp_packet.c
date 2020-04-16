#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define ICMP6_ECHO 128

struct ip6_pseudo_hdr 
{
    struct in6_addr ip6_src;
    struct in6_addr ip6_dst;
    uint32_t ip6_plen;
    uint8_t ip6_zero[3];
    uint8_t ip6_nxt;
};

static uint16_t compute_checksum(char *buf, size_t size, struct sockaddr *sa) 
{
  size_t i;
  uint64_t sum = 0;
  char *buffer;

  if(sa->sa_family == AF_INET6)
  {
    struct {
    	struct ip6_pseudo_hdr ip6_hdr;
    	struct icmp icmp;
		} data = {0};
		data.ip6_hdr.ip6_src.s6_addr[15] = 1;
		data.ip6_hdr.ip6_dst = ((struct sockaddr_in6 *) sa)->sin6_addr;
		data.ip6_hdr.ip6_plen = htonl((uint32_t)sizeof(struct icmp));
		data.ip6_hdr.ip6_nxt = IPPROTO_ICMPV6;
		data.icmp = *(struct icmp *)&buf;
		buffer = (char *)&data;
  }
  else
  {
  	buffer = buf;
  }

  for (i = 0; i < size; i += 2) {
      sum += *(uint16_t *)buffer;
      buffer += 2;
  }
  if (size - i > 0) {
      sum += *(uint8_t *)buffer;
  }
  while ((sum >> 16) != 0) {
      sum = (sum & 0xffff) + (sum >> 16);
  }

  return (uint16_t)~sum;
}

struct icmp create_ICMP_pkt(struct sockaddr *sa, int seq_num)
{
	// doesnt seem to work for IPv6 addresses
	struct icmp icmp_request = {0};
	icmp_request.icmp_type = (sa->sa_family == AF_INET6) ? ICMP6_ECHO : ICMP_ECHO;
	icmp_request.icmp_code = 0;
	uint16_t id = (uint16_t)getpid();
	icmp_request.icmp_id = htons(id);
	uint16_t seq = seq_num;
	icmp_request.icmp_seq = htons(seq);
	icmp_request.icmp_cksum = compute_checksum((char *)&icmp_request, sizeof(icmp_request), sa);

	return icmp_request;
}