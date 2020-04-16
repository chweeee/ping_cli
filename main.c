#include <limits.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <errno.h>

#include "resolver.c"
#include "create_icmp_packet.c"
#include "ip_to_string.c"

/**
usage: ./a.out [-h hostname]
			         [-m ttl]
			         [-c count]
**/

#define DEFAULT_TTL 30
#define MAX_IP_HEADER_SIZE 60

int total_pkts = 0;
int recv_pkts = 0;
double min_rtt = INT_MAX;
double max_rtt = INT_MIN;
double total_rtt = 0.0;

void process_args(char *argv[], int argc, int *ttl, char **hostname, int *count) 
{
  for(int i = 1; i < argc; i++) 
  {
  	if(strcmp(argv[i],"-h") == 0)
  	{
  		*hostname = argv[++i];
  	} else if(strcmp(argv[i],"-m") == 0) 
  	{
  		*ttl = atoi(argv[++i]);
  	} else if(strcmp(argv[i],"-c") == 0)
  	{
  		*count = atoi(argv[++i]);
  	}
  	else 
  	{
  		printf("invalid option: %s\n", argv[i]);
  		break;
  	}
  }
}

int process_resp(char* recv_buf, struct sockaddr sa, long double rtt, int ttl)
{
	struct icmp *icmp_response;
	uint8_t ip_header_size = (sa.sa_family == AF_INET) ? ((*(uint8_t *)recv_buf) & 0x0F) * 4 : 0;
	uint8_t resp_len = sizeof(recv_buf) + ip_header_size;

	icmp_response = (struct icmp *)(recv_buf + ip_header_size);
	icmp_response->icmp_cksum = ntohs(icmp_response->icmp_cksum);
 	icmp_response->icmp_id = ntohs(icmp_response->icmp_id);
  icmp_response->icmp_seq = ntohs(icmp_response->icmp_seq);

  printf("%hhu bytes from %s: icmp_seq=%d ttl=%d time=%.3Lf ms\n", resp_len, get_ip_str(&sa), icmp_response->icmp_seq, ttl, rtt);

	return 0;
}

int send_ping_pkt(int sockfd, struct icmp icmp_request, struct sockaddr sa, int ttl)
{
	char recv_buf[MAX_IP_HEADER_SIZE + sizeof(struct icmp)];
	int recv_size = (sa.sa_family == AF_INET) ? (int)(MAX_IP_HEADER_SIZE + sizeof(struct icmp)) : (int)sizeof(struct icmp);
	socklen_t addrlen;
	struct timespec time_start, time_end; 
	long double rtt = 0;

	clock_gettime(CLOCK_MONOTONIC, &time_start); 
	int send_result = sendto(sockfd, (const char *)&icmp_request, sizeof(icmp_request), 0, &sa, sa.sa_len);

	if(send_result < 0) 
	{
		printf("sendto() errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	else
	{
		total_pkts++;
	}

	// setting timeout for receving socket
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    perror("Error");
	}

	int recv_result = recvfrom(sockfd, recv_buf, recv_size, 0, &sa, &addrlen);

	if(recv_result < 0) 
	{
		printf("Request timeout for icmp_seq %d\n", ntohs(icmp_request.icmp_seq));
	}
	else
	{
		recv_pkts++;
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
		rtt = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;

		min_rtt = (rtt < min_rtt) ? rtt : min_rtt;
		max_rtt = (rtt > max_rtt) ? rtt : max_rtt;;
		total_rtt += rtt;

		process_resp(recv_buf, sa, rtt, ttl);
	}

	return 0;
}

void print_statistics()
{
	printf("\n--- ping statistics ---\n");
	printf("%d packets transmitted, %d packets received, %0.1f%% packet loss\n", total_pkts, recv_pkts, (double)((total_pkts-recv_pkts)/total_pkts));
	printf("round-trip min/avg/max = %0.3f/%0.3f/%0.3f ms\n", min_rtt, (double)total_rtt/total_pkts, max_rtt);

	return;
}

void signal_callback_handler(int signum) 
{
	print_statistics();
  exit(signum);
}

int main(int argc, char *argv[])  {

	int ttl = DEFAULT_TTL;
	int count = 0;
	int sockfd;
	char *hostname;
	struct sockaddr sa;

  if( argc < 2 || strcmp(argv[1],"-h") != 0) 
  {
  	fprintf(stderr, "Usage: %s requires positional argument -h.\n", argv[0]);
  	exit(EXIT_FAILURE);
  }

  process_args(argv, argc, &ttl, &hostname, &count);
  sockfd = resolve(hostname, &sa, ttl);

  signal(SIGINT, signal_callback_handler);

  printf("PING %s (%s)\n", hostname, get_ip_str(&sa));

  if(count != 0)
  {
  	for(int i = 0; i < count; i++)
	  {
	  	struct icmp icmp_request = create_ICMP_pkt(&sa, i);
	  	send_ping_pkt(sockfd, icmp_request, sa, ttl);
	  	sleep(1);
	  }
	  print_statistics();
  }
  else
  {
  	int i = 0;
  	while(1)
  	{
  		struct icmp icmp_request = create_ICMP_pkt(&sa, i);
	  	send_ping_pkt(sockfd, icmp_request, sa, ttl);
	  	sleep(1);
	  	i++;
  	}
  }
}	














