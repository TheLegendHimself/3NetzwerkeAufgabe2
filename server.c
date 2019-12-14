#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdbool.h>

// Define Buffer length
const	size_t	BUF_LEN	=	128;
const	size_t	REQ_LEN	=	1024;

// Something unexpected happened. Report error and terminate.
void	sysErr(	char	*msg,	int	exitCode	)
{
	fprintf(	stderr,	"%s\n\t%s\n",	msg,	strerror(	errno	)	);
	exit(	exitCode	);
}

// The user entered something stupid. Tell him.
void	usage(	char	*argv0	)
{
	printf(	"usage : %s portnumber\n",	argv0	);
	exit(	0	);
}






int	get_line(	int	sock,	char	*buf,	int	size	)
{
    int	i	=	0;
    char	c	=	'\0';
    int	n;
    
    while(	(	i	<	size	-	1	)	&& (	c	!=	'\n'	)	)
    {
        n	=	recv(	sock,	&c,	1,	0	);
        /* DEBUG printf("%02X\n", c); */
        if(	n	>	0	)
        {
            if(	c	==	'\r'	)
            {
                n	=	recv(	sock,	&c,	1,	MSG_PEEK	);
                /* DEBUG printf("%02X\n", c); */
                if(	(	n	>	0	)	&&	(	c	==	'\n'	)	)
				{
                    recv(	sock,	&c,	1,	0	);
				}else{
                    c	=	'\n';// end While
				}
            }
            buf[i]	=	c;
            i++;
        }else{
            c = '\n';// End While
		}
    }
    buf[i]	=	'\0';// show end of string
    return(	i	);
}






int	main(	int	argc,	char	**argv	)
{
	// int for socket and connection number
	int	connfd,	sockfd;

	// Struct for server & client ip & port
	struct	sockaddr_in	server_addr,	client_addr; 

	// length of server or client struct
	socklen_t	addrLen	=	sizeof(	struct sockaddr_in	); 

	// RevBuff for incomming Message
	char	revBuff[BUF_LEN];

 	// For Error checking from Read
	//size_t	len; 

	// Check for right number of arguments
	if(	argc	<	2	)
	{
		usage(	argv[0]	);
	}

	// Creates Socket (TCP)
	if(	(	connfd	=	socket(	AF_INET,	SOCK_STREAM,	0	)	)	==	-1	)
	{
		sysErr(	"Server Fault : SOCKET",	-1	);
	}
	
	// writes 0's to server ip 
	memset(	&server_addr,	0,	addrLen	);
	//sets server ip adress to own ip adress 
	server_addr.sin_addr.s_addr	=	htonl(	INADDR_ANY	);
	// The ip adress is a iPv4 adress
	server_addr.sin_family	=	AF_INET;
	// sets port:
	server_addr.sin_port	=	htons(	(	u_short	)	atoi(	argv[1]	)	);

	// Bind The Socket to a connection connfd
	if(	bind(	connfd,	(	struct	sockaddr	*	)	&server_addr,	addrLen	)	==	-1	) 
	{
		sysErr(	"Server Fault : BIND",	-2	);
	}else{
		printf(	"ServerSocket Binded"	);
	}

	// listen on this connection
	if(	(	listen(	connfd,	1	)	)	!=	0	)
	{
		sysErr(	"Server Fault : Listen failed...",	-3	);
	}
		
	while(	true	)
	{
		// wait for incoming TCP-Connection

		// writes 0 to revBuff
		memset(	revBuff,	0,	BUF_LEN	);
		
		// Accept TCP connection
		sockfd	=	accept(	connfd,	(	struct	sockaddr	*	)	&client_addr,	&addrLen	);
		if(	sockfd	<	0	)
		{
			sysErr(	"Server Fault: server accept failed",	-4	);
		}
		
		int	lenRequest	=	1;
		char	*request	=	"\n";
		char	*fullRequest[10];
		int	lineCount=0;
		while(	(	lenRequest	>	0	)	&&	strcmp(	"\n",	request	)	)
		{
			lenRequest	=	get_line(	sockfd,	request,	REQ_LEN	-	1	);
			strcpy(	fullRequest[lineCount],	request	);
			
			printf(	"%s",	fullRequest[lineCount]	);
			lineCount++;
		}
		
		
		/*
		// Read from connection
		len	=	read(	sockfd,	revBuff,	BUF_LEN	-	1	);
		printf(	"Received from %s: \n",	inet_ntoa(	client_addr.sin_addr	)	);

		// Send TCP Packet back
		write(	sockfd,	revBuff,	len	);

		// Write TCP Packet to stdout (console)
		printf(	"Packet erhalten: %s\n",	revBuff	);
		*/
		// Close Sockfd
		close(	sockfd	);

		// Start to accept new TCP connection until [CTRL]+C
	}
	// Before exit close the initial socket
	close(	connfd	);
	return 0;
}
