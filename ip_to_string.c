#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

char *get_ip_str(struct sockaddr *sa)
{
	char *s = NULL;
  switch(sa->sa_family) 
  {
    case AF_INET:
    {
    	struct sockaddr_in *addr_in = (struct sockaddr_in *)sa;
    	s = malloc(INET_ADDRSTRLEN);
    	inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
    	break;
    }
    case AF_INET6: {
      struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&sa;
      s = malloc(INET6_ADDRSTRLEN);
      inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
      break;
    }
		default:
		{
      strcpy(s,"Unknown IP");
		}
  }

  return s;
}