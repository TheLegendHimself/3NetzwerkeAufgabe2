#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdbool.h>

// Define Buffer length
//const	size_t	BUF_LEN	=	128;
const	size_t	REQ_LEN	=	1024;
const	char	*NotImpl	=	"HTTP/1.0 501 Not Implemented \\r\\n\nContent-type: text/html\\r\\n\n\\r\\n\n";
const	char	*NotImplFile	=	"<html><body><b>501</b> Operation not supported</body></html>\\r\\n\n";
const	char	*ok	=	"HTTP/1.0 200 OK\\r\\n\n\\r\\n\n";
const	char	*okFile	=	"<html><body>Dies ist die eine Fake Seite des Webservers!</body></html>\\r\\n\n";

int	get_line(	int	sock,	char	*buf,	int	size	)
{
    int	i	=	0;
    char	c	=	'\0';
    int	n;
    
    while(	(	i	<	size	-	1	)	&&	(	c	!=	'\n'	)	)
    {
        n	=	recv(	sock,	&c,	1,	0	);
        /* DEBUG printf("%02X\n", c); */
        if(	n	>	0	)
        {
            if	(	c	==	'\r'	)
            {
                n	=	recv(	sock,	&c,	1,	MSG_PEEK	);
                /* DEBUG printf("%02X\n", c); */
                if(	(	n	>	0	)	&&	(	c	==	'\n'	)	)
                    recv(	sock,	&c,	1,	0	);
                else
                    c	=	'\n';
            }
            buf[i]	=	c;
            i++;
        }
        else
            c	=	'\n';
    }
    buf[i]	=	'\0';
    return(	i	);
}

void	sysErr(	char	*msg,	int	exitCode	)			// Something unexpected happened. Report error and terminate.
{
	fprintf(	stderr,	"%s\n\t%s\n",	msg,	strerror(	errno	)	);
	exit(	exitCode	);
}


void	usage(	char	*argv0	)							// The user entered something stupid. Tell him.
{
	printf(	"usage : %s portnumber\n",	argv0	);
	exit(	0	);
}

int	main(	int	argc,	char	**argv	)
{	
	int	connfd,	sockfd;										// int for socket and connection number
	struct	sockaddr_in	server_addr,	client_addr;		// Struct for server & client ip & port
	socklen_t	addrLen	=	sizeof(	struct sockaddr_in	);	// length of server or client struct
	//char	revBuff[BUF_LEN];								// RevBuff for incomming Message
	//size_t	len;											// For Error checking from Read
	
	if(	argc	<	2	)									// Check for right number of arguments
	{
		usage(	argv[0]	);
	}

	
	if(	(	sockfd	=	socket(	AF_INET,	SOCK_STREAM,	0	)	)	==	-1	)	// Creates Socket (TCP)
	{
		sysErr(	"Server Fault : SOCKET",	-1	);
	}else{
		printf(	"TCP Socket Created\n"	);
	}
	
	memset(	&server_addr,	0,	addrLen	);					// writes 0's to server ip 	 
	server_addr.sin_addr.s_addr	=	htonl(	INADDR_ANY	);	// sets server ip adress to own ip adress	
	server_addr.sin_family	=	AF_INET;					// The ip adress is a iPv4 adress	
	server_addr.sin_port	=	htons(	(	u_short	)	atoi(	argv[1]	)	);	// gets port from arguments

	if(	bind(	sockfd,	(	struct	sockaddr	*	)	&server_addr,	addrLen	)	==	-1	)	// Bind The Socket to a connection connfd
	{
		sysErr(	"Server Fault : BIND",	-2	);
	}else{
		printf(	"Socket Bound\n"	);
	}

	if(	(	listen(	sockfd,	1	)	)	!=	0	)			// listen on this connection
	{
		sysErr(	"Server Fault : Listen failed...",	-3	);
	}else{
		printf(	"Listen started\n"	);
	}
		
	while(	true	)										// Start to accept new TCP connection until [CTRL]+C
	{
		printf(	"Waiting for new connection\n"	);			// wait for incoming TCP-Connection
		connfd	=	accept(	sockfd,	(	struct	sockaddr	*)	&client_addr,	&addrLen	);	// Accept TCP connection
		if(	connfd	<	0	)
		{
			sysErr(	"Server Fault: server accept failed",	-4	);
		}else{
			printf(	"Connection accepted\n"	);
		}
		
		//now there is that boi
		char	request[REQ_LEN];
		char	*fullRequest;
		size_t	RequestLength=1;
		fullRequest	=	(	char*	)	malloc(	RequestLength	);
		int	numchars	=	2;
		char	method[25]	=	"";
		size_t	i,	j;
		while(	numchars	>	1	)
		{
			numchars	=	get_line(	connfd,	request,	sizeof(	request	)	);
			//printf(	"numchars: %d\n",	numchars	);
			printf(	"Received: %s\n",	request	);
			RequestLength	+=	strlen(	request	);
			fullRequest	=	(	char*	)	realloc(	fullRequest,	RequestLength	);
			strcat(	fullRequest,	request	);			
		}
		//printf(	"Request complete \n"	);
		printf(	"Complete Request: \n %s \n",	fullRequest	);
		
		i	=	0;
		j	=	0;
		while(	fullRequest[i]	!=	' '	&&	(	j	<	strlen(	fullRequest	)	)	)
		{
			method[i]	=	fullRequest[i];
			i++;
			j++;
			if(	i	>=	20	)
			{
				printf(	"Method not found\n"	);
			}
		}
		printf(	"Method : %s\n",	method	);
		
		if(	strcmp(	method,	"GET"	)	==	0	)
		{			
			const	char	s[2]	=	" ";
			char	*token;
			token	=	strtok(	fullRequest,	s	);
			token	=	strtok(	NULL,	s	);
			printf(	"looking for this file: %s\n",	token	);
			
			/* How to use strtok 
			while(	token	!=	NULL	)
			{
				token	=	strtok(	NULL,	s	);
				//printf(	"accepted part %s\n",	token	);				
			}
			*/
			
			write(	connfd,	ok,	strlen(	ok	)	);
			write(	connfd,	okFile,	strlen(	okFile	)	);
		}else{	
			write(	connfd,	NotImpl,	strlen(	NotImpl	)	);
			write(	connfd,	NotImplFile,	strlen(	NotImplFile	)	);
		}

		// strtok( string der gesplittet wird, mit welchem zeichen )

		/*
		memset(	revBuff,	0,	BUF_LEN	);					// writes 0 to revBuff	
		len	=	read(	sockfd,	revBuff,	BUF_LEN	-	1	);	// Read from connection
		printf(	"Received from %s: \n",	inet_ntoa(	client_addr.sin_addr	)	);		
		write(	sockfd,	revBuff,	len	);						// Send TCP Packet back		
		printf(	"Packet erhalten: %s\n",	revBuff	);		// Write TCP Packet to stdout (console)
		*/
		
		printf(	"Connection closing\n"	);					// Close Sockfd
		close(	connfd	);									
	}	
	close(	sockfd	);										// Before exit close the initial socket
	return 0;
}