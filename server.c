#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<signal.h>

const	size_t	REQ_LEN	=	1024;
const	char	*serverName	=	"SERVER: IHopeYouKnowTheWay\r\n";
const	char	*header200	=	"HTTP/1.0 200 OK\r\nContent-type: text/html\r\n";
const	char	*header501	=	"HTTP/1.0 501 Not Implemented \r\nContent-type: text/html\r\n";
const	char	*header404	=	"HTTP/1.0 404 File Not Found \r\nContent-type: text/html\r\n";
const	char	*endHeader	=	"\r\n";
const	char	*subdir	=	"microwww/";
/**************************************************/
/*
get_line gets a message char by char from the socket
and terminates when it hits the end of line 
the functions writes the hole request into the buffer
*buf returns the amount of chars read
*/
/**************************************************/
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
/************************************************/
/*
writes an Error Message to stdout
followed by the errno Message
then terminates the current programm with 
an exit code
Something unexpected happened. Report error and terminate.
*/
/************************************************/
void	sysErr(	char	*msg,	int	exitCode	)			
{
	fprintf(	stderr,	"%s\n\t%s\n",	msg,	strerror(	errno	)	);
	exit(	exitCode	);
}
/************************************************/
/*
The user entered something stupid. Tell him.
*/
/************************************************/
void	usage(	char	*argv0	)							
{
	printf(	"usage : %s portnumber\n",	argv0	);
	exit(	0	);
}
/************************************************/
/*
getFile takes a Filename and adds 
the subdirectory to it
then returns the Filepointer
if the fopen call fails,
because the File doesnt exist
or it has no read rights it
"fails" and returns NULL
-> 404
*/
/************************************************/
FILE	*getFile(	char	*token	)
{
	char	pathToFile[REQ_LEN];
	FILE	*fp;
	strcpy(	pathToFile,	subdir	);
	strcat(	pathToFile,	token	);
	printf(	"FullPath \"%s\"\n",	pathToFile	);
	fp	=	fopen(	pathToFile,	"r"	);						// Maybe error check, but if NULL then it should return NULL so i dont think it is necassary
	return	fp;
}
/************************************************/
/*
sendFile takes a filepointer and a connection
then reads all contents from the File and writes 
it on the socket
*/
/************************************************/
void	sendFile(	FILE	*fp,	int	connfd	)
{
	char	fileout[REQ_LEN];
	while(	fgets(	fileout,	sizeof(	fileout	),	fp	)	!=	NULL	)	// No Errorchecking for fgets ( only returns NULL if wrong )
	{
		printf(	"Read and send: %s\n",	 fileout	);
		if(	write(	connfd,	fileout,	strlen(	fileout	)	)	<	0	)
		{
			sysErr(	"Server Fault: writing file to socket",	-11	);
		}
	}
}
/************************************************/
/*
getRequest just uses get_line until
the HTTP request is complete
it then returns the pointer to the fullRequest
*/
/************************************************/
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
		RequestLength	+=	strlen(	request	);
		fullRequest	=	(	char*	)	realloc(	fullRequest,	RequestLength	);
		strcat(	fullRequest,	request	);			
	}
	return fullRequest;
}
/************************************************/
/*
getMethod returns a pointer to a string of 
the first chars until " " is found
*/
/************************************************/
char*	getMethod(	char*	fullRequest	)
{
	char	*method	=	(	char*	)malloc(	sizeof(	char	)	*	21	);
	size_t	i	=	0;
	size_t	j	=	0;	
		
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
/************************************************/
/*
writeToSocket takes a socket and a message. then
writes the message to the socket
*/
/************************************************/
void	writeToSocket(	int	connfd,	const	char	*	message	)
{
	printf(	"Sending: \"%s\" \n",	message	);
	if(	send(	connfd,	message,	strlen(	message	),	0	)	<	0	)
	{
		sysErr(	"Server Fault: writing to socket",	-12	);
	}
}
/************************************************/
/*
writes the hole headermessage to the socket
depending on const char * head
*/
/************************************************/
void	writeHeader(	int	connfd,	const	char	*	head)
{
	writeToSocket(	connfd,	head	);
	writeToSocket(	connfd,	serverName	);
	writeToSocket(	connfd,	endHeader	);	
}
/************************************************/
/*
closes a File with Errorchecking
*/
/************************************************/
void	closeFile(	FILE	*fp	)
{
	if(	fclose(	fp	)	==	EOF	)
	{
		sysErr(	"Server Fault: closing File",	-13	);
	}
}





int	main(	int	argc,	char	**argv	)
{	
	if(	argc	<	2	)									// Check for right number of arguments
	{
		usage(	argv[0]	);
	}
	//----------------------------------------------------------------------------------------------
	//										SERVER START
	//----------------------------------------------------------------------------------------------
	struct	sockaddr_in	server_addr,	client_addr;		// Struct for server & client ip & port
	socklen_t	addrLen	=	sizeof(	struct sockaddr_in	);	// length of server or client struct	
	int	sockfd,	connfd;
	memset(	&server_addr,	0,	addrLen	);					// writes 0's to server ip 	 
	server_addr.sin_addr.s_addr	=	htonl(	INADDR_ANY	);	// sets server ip adress to own ip adress	
	server_addr.sin_family	=	AF_INET;					// The ip adress is a iPv4 adress	
	server_addr.sin_port	=	htons(	(	u_short	)	atoi(	argv[1]	)	);	// gets port from arguments	
	
	
	if(	signal(	SIGCHLD,	SIG_IGN	)	==	SIG_ERR	)
	{
		sysErr(	"Server Fault: Ignore Childreturns",	-1	);
	}	
	if(	(	sockfd	=	socket(	AF_INET,	SOCK_STREAM,	0	)	)	==	-1	)	// Creates Socket (TCP)
	{
		sysErr(	"Server Fault: SOCKET",	-2	);
	}else{
		printf(	"TCP Socket Created\n"	);
	}	
	if(	bind(	sockfd,	(	struct	sockaddr	*	)	&server_addr,	addrLen	)	==	-1	)	// Bind The Socket to a connection connfd
	{
		sysErr(	"Server Fault: BIND",	-3	);
	}else{
		printf(	"Socket Bound\n"	);
	}
	//----------------------------------------------------------------------------------------------
	//										SERVER START ENDED	
	//----------------------------------------------------------------------------------------------	
		
	while(	true	)										// Start to accept new TCP connection until [CTRL]+C
	{
		/************************************ WHILE TRUE ***************************************************/
		if(	(	listen(	sockfd,	1	)	)	!=	0	)			// listen on this connection
		{
			sysErr(	"Server Fault: Listen failed...",	-4	);
		}else{
			printf(	"Waiting for new connection\n"	);// wait for incoming TCP-Connection	
		}
			
		if(	(	connfd	=	accept(	sockfd,	(	struct	sockaddr	*)	&client_addr,	&addrLen	)	)	<	0	)	// Accept TCP connection
		{
			sysErr(	"Server Fault:	Accepting TCP Connection",	-5	);
		}else{
			printf(	"Connection accepted\n"	);
		}
		pid_t	newchild	=	fork();
		if(	newchild	<	0	)
		{
			sysErr(	"Server Fault: Creating Child failed",	-6	);
		}
		if(	newchild	==	0	)
		{
			/***************************** START CHILD ***************************************************/
			char	*fullRequest;
			char	*method;
			FILE	*fp;
			fullRequest	=	getRequest(	connfd	);				// Gets Request From Socket
			method	=	getMethod(	fullRequest	);			// Extracts the method from fullRequest		
			printf(	"Complete Request: \n %s \n",	fullRequest	);
			printf(	"Method : %s\n",	method	);
				
			if(	strcmp(	method,	"GET"	)	==	0	)			// GET command
			{			
				const	char	s[2]	=	" ";
				char	*token;	
				token	=	strtok(	fullRequest,	s	);
				token	=	strtok(	NULL,	s	);				
				token[strlen(	token	)]	=	'\0';
				//printf(	"File: \"%s\"\n",	token	); /*DEBUG for correct file name*/	
				
				if(	strcmp(	token,	"/"	)	==	0	)
				{
					printf(	"change token to index.htm\n"	);
					strcpy(	token,	"index.htm"	);
				}
				//----------------------------------------------------------
				char	tokken[strlen(	token	)];				
				char	tokkken[2];
				tokkken[0]	=	tokken[0];
				strcpy(	tokken,	token	);
				if(	strcmp(	tokkken,	"/"	)	==	0	)
				{
					/******************************************/
					char	inputstr[strlen(	tokken	)];
					int	i	=	0;
					while(	i	<	(	int	)	strlen(	token	)	)
					{												// Removes "/" from the beginning of the filename
						inputstr[i]	=	tokken[i	+	1];
						i++;
					}
					/******************************************/
					if(	strchr(	inputstr,	'/'	)	!=	NULL	)		// Sets page to index.htm if / is found (security)
					{
						printf(	"Could have tried to escape\n"	);
						strcpy(	token,	"index.htm"	);
					}
				}
	
			//----------------------------------------------------------	
				
				if(	(	fp	=	getFile(	token	)	)	==	NULL	)
				{
					//----------------------------------------
					//			404 Message
					writeHeader(	connfd,	header404	);
					fp	=	getFile(	"404.htm"	);
					sendFile(	fp,	connfd	);	
					closeFile(	fp	);
					//----------------------------------------
				}else{	
					//----------------------------------------
					//			200 Message
					writeHeader(	connfd,	header200	);
					sendFile(	fp,	connfd	);
					closeFile(	fp	);
					//----------------------------------------
				}
						
			}else{	
				//----------------------------------------
				//			501 Message
				writeHeader(	connfd,	header501	);
				fp	=	getFile(	"501.htm"	);
				sendFile(	fp,	connfd	);
				closeFile(	fp	);
				//----------------------------------------
			}
			close(	connfd	);		
			exit(	0	);
			/***************************** END CHILD **************************************/
		}
		close(	connfd	);
		/********************************* END WHILE ******************************************/
	}
	// Usual way of closing the socket
	if(	close(	sockfd	)	<	0	)
	{
		sysErr(	"Server Fault: closing Connection",	-9	);
	}	
	return 0;
}