#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<signal.h>

// Define Buffer length
const	size_t	REQ_LEN	=	1024;
const	char	*NotImpl	=	"HTTP/1.0 501 Not Implemented \\r\\n\nContent-type: text/html\\r\\n\n";
const	char	*NotImplFile	=	"<html><body><b>501</b> Operation not supported</body></html>\\r\\n\n";
const	char	*NotImpl1	=	"\\r\\n\n";
const	char	*ok	=	"HTTP/1.0 200 OK\\r\\n\n";
const	char	*ok1	=	"Content-type: text/html\\r\\n\n";
const	char	*ok3	=	"\\r\\n\n";
const	char	*okFile	=	"<html><body>Dies ist die eine Fake Seite des Webservers!</body></html>\\r\\n\n";
const	char	*serverName	=	"SERVER: IHopeYouKnowTheWay\\r\\n\n";

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

int	sendAnswer(	int	code,	char	*filename	)
{
	if(	code	==	200	)
	{
		
	}
	
	return	0;
}

int	getRequest();



int	main(	int	argc,	char	**argv	)
{	
	//----------------------------------------------------------------------------------------------
	//										SERVER START
	//----------------------------------------------------------------------------------------------
	int	connfd,	sockfd;										// int for socket and connection number
	struct	sockaddr_in	server_addr,	client_addr;		// Struct for server & client ip & port
	socklen_t	addrLen	=	sizeof(	struct sockaddr_in	);	// length of server or client struct
	
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
	
	signal(	SIGCHLD,	SIG_IGN	);
	//----------------------------------------------------------------------------------------------
	//										SERVER START ENDED	
	//----------------------------------------------------------------------------------------------	
		
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
		
		char	request[REQ_LEN];
		char	*fullRequest;
		size_t	RequestLength	=	1;
		int	numchars	=	2;
		char	method[25]	=	"";
		size_t	i,	j;
		
		fullRequest	=	(	char*	)	malloc(	RequestLength	);
		
		while(	numchars	>	1	)
		{
			numchars	=	get_line(	connfd,	request,	sizeof(	request	)	);
			printf(	"Received: %s\n",	request	);
			RequestLength	+=	strlen(	request	);
			fullRequest	=	(	char*	)	realloc(	fullRequest,	RequestLength	);
			strcat(	fullRequest,	request	);			
		}
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
				break;
			}
		}
		printf(	"Method : %s\n",	method	);
		
		if(	strcmp(	method,	"GET"	)	==	0	)
		{			
			const	char	s[2]	=	" ";
			char	*token;
			char	pathToFile[REQ_LEN];
			FILE	*fp;
			
			token	=	strtok(	fullRequest,	s	);
			token	=	strtok(	NULL,	s	);
			token[strlen(	token	)	]	=	'\0';		
			printf(	"File: \"%s\"\n",	token	);			
			strcpy(	pathToFile,	"microwww/"	);
			strcat(	pathToFile,	token	);
			printf(	"FullPath \"%s\"\n",	pathToFile	);
										
			fp	=	fopen(	pathToFile,	"r"	);	// TO-DO: check for / in token so you cant leave
			
			if(	fp	==	NULL	)
			{
				printf(	"Not Found 404\n"	);
			}else{
				char	fileout[REQ_LEN];
				
				write(	connfd,	ok,	strlen(	ok	)	);
				write(	connfd,	ok1,	strlen(	ok1	)	);
				write(	connfd,	serverName,	strlen( serverName	)	);
				write(	connfd,	ok3,	strlen(	ok3	)	);				
				//printf(	"Size fileout: %d\n",	sizeof(	fileout	)	);
				//printf(	"Size fileout: %d\n",	strlen(	fileout	)	);
				
				while(	fgets(	fileout,	sizeof(	fileout	),	fp	)	!=	NULL	)
				{
					printf(	"Read and send: %s\n",	 fileout	);
					write(	connfd,	fileout,	strlen(	fileout	)	);
				}
				fclose(	fp	);
			}
			/* How to use strtok 
			while(	token	!=	NULL	)
			{
				token	=	strtok(	NULL,	s	);
				//printf(	"accepted part %s\n",	token	);				
			}
			*/
			
		}else{	
			write(	connfd,	NotImpl,	strlen(	NotImpl	)	);
			write(	connfd,	serverName,	strlen( serverName	)	);
			write(	connfd,	ok3,	strlen(	ok3	)	);
			write(	connfd,	NotImplFile,	strlen(	NotImplFile	)	);
			write(	connfd,	NotImpl1,	strlen(	NotImpl1	)	);
		}

		
		printf(	"Connection closing\n"	);					// Close Sockfd
		close(	connfd	);									
	}	
	close(	sockfd	);										// Before exit close the initial socket
	return 0;
}