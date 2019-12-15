#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<signal.h>
//#define	REQ_LEN	1024
// Define Buffer length
const	size_t	REQ_LEN	=	1024;

int	get_line(	int	sock,	char	*buf,	int	size	)	// I didnt change Any code from this so here is no error checking :)
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

FILE	*getFile(	char	*token	)
{
	char	pathToFile[REQ_LEN];
	FILE	*fp;
	strcpy(	pathToFile,	"microwww/"	);
	strcat(	pathToFile,	token	);
	printf(	"FullPath \"%s\"\n",	pathToFile	);
	fp	=	fopen(	pathToFile,	"r"	);						// Maybe error check, but if NULL then it should return NULL so i dont think it is necassary
	return	fp;
}
int	sendFile(	FILE	*fp,	int	connfd	)
{
	char	fileout[REQ_LEN];
	while(	fgets(	fileout,	sizeof(	fileout	),	fp	)	!=	NULL	)
	{
		printf(	"Read and send: %s\n",	 fileout	);
		if(	write(	connfd,	fileout,	strlen(	fileout	)	)	<	0	)
		{
			sysErr(	"Server Fault: writing file to socket",	-24	);
		}
	}
	return	0;
}
char*	getRequest(	int	connfd	)
{
	char	request[REQ_LEN];
	char	*fullRequest;
	size_t	RequestLength	=	1;
	int	numchars	=	2;
	
	fullRequest	=	(	char*	)	malloc(	RequestLength	);
	while(	numchars	>	1	)
	{
		numchars	=	get_line(	connfd,	request,	sizeof(	request	)	);
		//printf(	"Received: %s\n",	request	);
		RequestLength	+=	strlen(	request	);
		fullRequest	=	(	char*	)	realloc(	fullRequest,	RequestLength	);
		strcat(	fullRequest,	request	);			
	}
	return fullRequest;
}

char*	getMethod(	char*	fullRequest	)
{
	char	*method	=	(	char*	)malloc(	sizeof(	char	)	*	21	);
	size_t	i,	j;
	
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
	return	method;
}

int	main(	int	argc,	char	**argv	)
{	
	//----------------------------------------------------------------------------------------------
	//										SERVER START
	//----------------------------------------------------------------------------------------------
	int	connfd,	sockfd;										// int for socket and connection number
	struct	sockaddr_in	server_addr,	client_addr;		// Struct for server & client ip & port
	socklen_t	addrLen	=	sizeof(	struct sockaddr_in	);	// length of server or client struct
	const	char	*serverName	=	"SERVER: IHopeYouKnowTheWay\\r\\n\n";
	const	char	*Header200	=	"HTTP/1.0 200 Header200\\r\\n\nContent-type: text/html\\r\\n\n";
	const	char	*Header501	=	"HTTP/1.0 501 Not Implemented \\r\\n\nContent-type: text/html\\r\\n\n";
	const	char	*Header404	=	"HTTP/1.0 404 File Not Found \\r\\n\nContent-type: text/html\\r\\n\n";
	const	char	*emptyLine	=	"\\r\\n\n";
	
	if(	argc	<	2	)									// Check for right number of arguments
	{
		usage(	argv[0]	);
	}
	
	if(	(	sockfd	=	socket(	AF_INET,	SOCK_STREAM,	0	)	)	==	-1	)	// Creates Socket (TCP)
	{
		sysErr(	"Server Fault: SOCKET",	-1	);
	}else{
		printf(	"TCP Socket Created\n"	);
	}
	
	memset(	&server_addr,	0,	addrLen	);					// writes 0's to server ip 	 
	server_addr.sin_addr.s_addr	=	htonl(	INADDR_ANY	);	// sets server ip adress to own ip adress	
	server_addr.sin_family	=	AF_INET;					// The ip adress is a iPv4 adress	
	server_addr.sin_port	=	htons(	(	u_short	)	atoi(	argv[1]	)	);	// gets port from arguments

	if(	bind(	sockfd,	(	struct	sockaddr	*	)	&server_addr,	addrLen	)	==	-1	)	// Bind The Socket to a connection connfd
	{
		sysErr(	"Server Fault: BIND",	-2	);
	}else{
		printf(	"Socket Bound\n"	);
	}

	if(	(	listen(	sockfd,	1	)	)	!=	0	)			// listen on this connection
	{
		sysErr(	"Server Fault: Listen failed...",	-3	);
	}else{
		printf(	"Listen started\n"	);
	}
	
	if(	signal(	SIGCHLD,	SIG_IGN	)	==	SIG_ERR	)
	{
		sysErr(	"Server Fault: Ignore Childreturns",	-23	);
	}
	//----------------------------------------------------------------------------------------------
	//										SERVER START ENDED	
	//----------------------------------------------------------------------------------------------	
		
	while(	true	)										// Start to accept new TCP connection until [CTRL]+C
	{
		printf(	"Waiting for new connection\n"	);		// wait for incoming TCP-Connection		
		if(	(connfd	=	accept(	sockfd,	(	struct	sockaddr	*)	&client_addr,	&addrLen	)	)	<	0	)	// Accept TCP connection
		{
			sysErr(	"Server Fault:	Accepting TCP Connection",	-6	);
		}
		pid_t	newchild	=	fork();
		if(	newchild	<	0	)
		{
			sysErr(	"Server Fault: Creating Child failed",	-5	);
		}
		if(	newchild	==	0	)
		{
			
			if(	connfd	<	0	)
			{
				sysErr(	"Server Fault: server accept failed",	-4	);
			}else{
				printf(	"Connection accepted\n"	);
			}
			
			char	*fullRequest;	
			fullRequest	=	getRequest(	connfd	);				// Gets Request From Socket
			printf(	"Complete Request: \n %s \n",	fullRequest	);	
			
			char	*method;
			method	=	getMethod(	fullRequest	);				// Extracts the method from fullRequest		
			printf(	"Method : %s\n",	method	);
			
			
			if(	strcmp(	method,	"GET"	)	==	0	)			// GET command
			{			
				const	char	s[2]	=	" ";
				char	*token;	
		//----------------------------------------------------------
				token	=	strtok(	fullRequest,	s	);
				token	=	strtok(	NULL,	s	);				
				token[strlen(	token	)]	=	'\0';
				printf(	"File: \"%s\"\n",	token	);				
				if(	strcmp(	token,	"/"	)	==	0	)
				{
					printf(	"change token to index.html\n"	);
					strcpy(	token,	"index.html"	);
				}
				if(	strchr(	token,	'/'	)	!=	NULL	&&	strchr(	NULL,	'/'	)	!=	NULL	)// Sets page to index.html if / is found (security)
				{
					printf(	"Could have tried to escape\n"	);
					strcpy(	token,	"index.html"	);
				}
				
		//----------------------------------------------------------	
		
				FILE	*fp;
				if(	(	fp	=	getFile(	token	)	)	==	NULL	)
				{
					
					if(	write(	connfd,	Header404,	strlen(	Header404	)	)	<	0	)
					{
						sysErr(	"Server Fault: writing to socket",	-7	);
					}
					if(	write(	connfd,	serverName,	strlen(	serverName	)	)	<	0	)
					{
						sysErr(	"Server Fault: writing to socket",	-8	);
					}
					if(	write(	connfd,	emptyLine,	strlen(	emptyLine	)	)	<	0	)
					{
						sysErr(	"Server Fault: writing to socket",	-9	);
					}
					if(	(	fp	=	fopen(	"microwww/404.html",	"r"	)	)	==	NULL	)
					{
						sysErr(	"Server Fault: opening File",	-10	);
					}
					sendFile(	fp,	connfd	);	
					if(	fclose(	fp	)	==	EOF	)
					{
						sysErr(	"Server Fault: closing File",	-11	);
					}
				}else{
					if(	write(	connfd,	Header200,	strlen(	Header200	)	)	<	0	)
					{
						sysErr(	"Server Fault: writing to socket",	-12	);
					}
					if(	write(	connfd,	serverName,	strlen(	serverName	)	)	<	0	)
					{
						sysErr(	"Server Fault: writing to socket",	-13	);
					}
					if(	write(	connfd,	emptyLine,	strlen(	emptyLine	)	)	<	0	)
					{
						sysErr(	"Server Fault: writing to socket",	-14	);
					}
					sendFile(	fp,	connfd	);
					if(	fclose(	fp	)	==	EOF	)
					{
						sysErr(	"Server Fault: closing File",	-15	);
					}
				}
									
			}else{	// Not Implemented Message		
				if(	write(	connfd,	Header501,	strlen(	Header200	)	)	<	0	)
				{
					sysErr(	"Server Fault: writing to socket",	-16	);
				}
				if(	write(	connfd,	serverName,	strlen(	serverName	)	)	<	0	)
				{
					sysErr(	"Server Fault: writing to socket",	-17	);
				}
				if(	write(	connfd,	emptyLine,	strlen(	emptyLine	)	)	<	0	)
				{
					sysErr(	"Server Fault: writing to socket",	-18	);
				}
				FILE	*fp;
				if(	(	fp	=	fopen(	"microwww/501.html",	"r"	)	)	==	NULL	)
				{
					sysErr(	"Server Fault: opening File",	-19	);
				}
				sendFile(	fp,	connfd	);
				if(	fclose(	fp	)	==	EOF	)
				{
					sysErr(	"Server Fault: closing File",	-20	);
				}
			}
			
			printf(	"Connection closing\n"	);					// Close Sockfd
			if(	close(	connfd	)	<	0	)
			{
				sysErr(	"Server Fault: closing Connection",	-21	);
			}
			exit(	0	);
		}			
	}	
	if(	close(	sockfd	)	<	0	)
	{
		sysErr(	"Server Fault: closing Connection",	-22	);
	}										// Before exit close the initial socket
	return 0;
}
