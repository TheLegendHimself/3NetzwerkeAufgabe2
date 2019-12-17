#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<signal.h>
//#include	<sys/socket.h>
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
			sysErr(	"Server Fault: writing file to socket",	-8	);
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

int	writeToSocket(	int	connfd,	const	char	*	message,	int flag	)
{
	printf(	"Trying to send: \n\"%s\" \n",	message	);
	if(	send(	connfd,	message,	strlen(	message	),	flag	)	<	0	)
	{
		sysErr(	"Server Fault: writing to socket",	-7	);
	}
	return	0;
}



int	main(	int	argc,	char	**argv	)
{	
	//----------------------------------------------------------------------------------------------
	//										SERVER START
	//----------------------------------------------------------------------------------------------											// 			//int for socket and connection number
	struct	sockaddr_in	server_addr,	client_addr;		// Struct for server & client ip & port
	socklen_t	addrLen	=	sizeof(	struct sockaddr_in	);	// length of server or client struct
	const	char	*serverName	=	"SERVER: IHopeYouKnowTheWay\r\n";
	const	char	*Header200	=	"HTTP/1.0 200 OK\r\nContent-type: text/html\r\n";
	const	char	*Header501	=	"HTTP/1.0 501 Not Implemented \r\nContent-type: text/html\r\n";
	const	char	*Header404	=	"HTTP/1.0 404 File Not Found \r\nContent-type: text/html\r\n";
	const	char	*emptyLine	=	"\n";
	const	char	*endHeader	=	"\r\n";
	int	sockfd,	connfd;
	if(	argc	<	2	)									// Check for right number of arguments
	{
		usage(	argv[0]	);
	}
	if(	signal(	SIGCHLD,	SIG_IGN	)	==	SIG_ERR	)
	{
		sysErr(	"Server Fault: Ignore Childreturns",	-23	);
	}
	
	
	memset(	&server_addr,	0,	addrLen	);					// writes 0's to server ip 	 
	server_addr.sin_addr.s_addr	=	htonl(	INADDR_ANY	);	// sets server ip adress to own ip adress	
	server_addr.sin_family	=	AF_INET;					// The ip adress is a iPv4 adress	
	server_addr.sin_port	=	htons(	(	u_short	)	atoi(	argv[1]	)	);	// gets port from arguments

	
	
	if(	(	sockfd	=	socket(	AF_INET,	SOCK_STREAM,	0	)	)	==	-1	)	// Creates Socket (TCP)
	{
		sysErr(	"Server Fault: SOCKET",	-1	);
	}else{
		printf(	"TCP Socket Created\n"	);
	}
	
	
	if(	bind(	sockfd,	(	struct	sockaddr	*	)	&server_addr,	addrLen	)	==	-1	)	// Bind The Socket to a connection connfd
	{
		sysErr(	"Server Fault: BIND",	-2	);
	}else{
		printf(	"Socket Bound\n"	);
	}
	//----------------------------------------------------------------------------------------------
	//										SERVER START ENDED	
	//----------------------------------------------------------------------------------------------	
		
	while(	true	)										// Start to accept new TCP connection until [CTRL]+C
	{
		if(	(	listen(	sockfd,	1	)	)	!=	0	)			// listen on this connection
		{
			sysErr(	"Server Fault: Listen failed...",	-3	);
		}else{
			printf(	"Listen started\n"	);
		}
	
		printf(	"Waiting for new connection\n"	);		// wait for incoming TCP-Connection	
		if(	(	connfd	=	accept(	sockfd,	(	struct	sockaddr	*)	&client_addr,	&addrLen	)	)	<	0	)	// Accept TCP connection
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
					printf(	"change token to index.htm\n"	);
					strcpy(	token,	"index.htm"	);
				}
				char	tokken[strlen(	token	)];
				strcpy(	tokken,	token	);
				char	tokkken[2];
				tokkken[0]	=	tokken[0];
				if(	strcmp(tokkken,	"/"	)	==	0	)
				{
					char	inputstr[strlen(	tokken	)];//	=	(	char*	)	malloc(	strlen(	token	)	);
					int	i	=	0;
					while(	i	<	(	int	)	strlen(	token	)	)
					{
						inputstr[i]	=	tokken[i	+	1];
						i++;
					}
					if(	strchr(	inputstr,	'/'	)	!=	NULL	)		// Sets page to index.htm if / is found (security)
					{
						printf(	"Could have tried to escape\n"	);
						strcpy(	token,	"index.htm"	);
					}
				}
	
		//----------------------------------------------------------	

				FILE	*fp;
				if(	(	fp	=	getFile(	token	)	)	==	NULL	)
				{
		
					writeToSocket(	connfd,	Header404,	0	);
					writeToSocket(	connfd,	serverName,	0	);
					//writeToSocket(	connfd,	emptyLine,	0	);
					writeToSocket(	connfd,	endHeader,	0	);
					if(	(	fp	=	fopen(	"microwww/404.htm",	"r"	)	)	==	NULL	)
					{
						sysErr(	"Server Fault: opening File",	-10	);
					}
					sendFile(	fp,	connfd	);	
					//writeToSocket(	connfd,	emptyLine,	0	);
					if(	fclose(	fp	)	==	EOF	)
					{
						sysErr(	"Server Fault: closing File",	-11	);
					}
				}else{
					writeToSocket(	connfd,	Header200,	0	);
					writeToSocket(	connfd,	serverName,	0	);
					//writeToSocket(	connfd,	emptyLine,	0	);
					writeToSocket(	connfd,	endHeader,	0	);
					sendFile(	fp,	connfd	);
					//writeToSocket(	connfd,	emptyLine,	0	);
					if(	fclose(	fp	)	==	EOF	)
					{
						sysErr(	"Server Fault: closing File",	-15	);
					}
				}
						
			}else{	// Not Implemented Message		
				writeToSocket(	connfd,	Header501,	0	);
				writeToSocket(	connfd,	serverName,	0	);
				//writeToSocket(	connfd,	emptyLine,	0	);
				writeToSocket(	connfd,	endHeader,	0	);
				FILE	*fp;
				if(	(	fp	=	fopen(	"microwww/501.htm",	"r"	)	)	==	NULL	)
				{
					sysErr(	"Server Fault: opening File",	-19	);
				}
				sendFile(	fp,	connfd	);
				//writeToSocket(	connfd,	emptyLine,	0	);
				if(	fclose(	fp	)	==	EOF	)
				{
					sysErr(	"Server Fault: closing File",	-20	);
				}
				close(	connfd	);
			}

			printf(	"Connection closing\n"	);					// Close Sockfd
			
			
			exit(	0	);	
		}
		close(	connfd	);
	}
	if(	close(	sockfd	)	<	0	)
	{
		sysErr(	"Server Fault: closing Connection",	-22	);
	}	
	
	return 0;
}