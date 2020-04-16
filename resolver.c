#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip.h>

// returns socket file descriptor and a sockaddr
int resolve(char *hostname, struct sockaddr *sas, int ttl)
{
	int errcode, sockfd;

	struct addrinfo hints;
	struct addrinfo *res, *rp;

	struct sockaddr *sa;
	socklen_t sa_len;

	char addrstr[100];
	void *ptr;

	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_RAW; // should we use SOCK_STREAM instead?
	hints.ai_flags |= AI_CANONNAME;

	// host resolution
	errcode = getaddrinfo(hostname, NULL, &hints, &res);
	if (errcode != 0)
	{   
	  fprintf(stderr, "error in host resolution: %s\n", gai_strerror(errcode));
	  return EXIT_FAILURE;
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) 
	{
		switch(rp->ai_addr->sa_family) 
		{
			case AF_INET:
			{
				sockfd = socket(rp->ai_family, rp->ai_socktype, IPPROTO_ICMP);
				break;
			}
			case AF_INET6:
			{
				sockfd = socket(rp->ai_family, rp->ai_socktype, IPPROTO_ICMPV6);
				break;
			}
	  }

	  if(sockfd > -1) {
	  	if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(ttl)) == -1)
			{
				perror("set ttl error");
			}
	  	break;
	  }
	}

	if(sockfd > -1) 
	{
		sa = malloc(rp->ai_addrlen);
		sa_len = rp->ai_addrlen;
		memcpy(sa, rp->ai_addr, rp->ai_addrlen);
		*sas = *sa;

	  return sockfd;   
	} 
	else
	{
		return -1;
	}
}