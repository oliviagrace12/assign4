/*-------------------------------------------------------------------------*
 *---									---*
 *---		babyFTPServer.cpp					---*
 *---									---*
 *---	    This file defines a very simple file-transfer server	---*
 *---	program.							---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1.0		2012 August 8		Joseph Phillips	---*
 *---									---*
 *---	Version 1.1		2013 August 15		Joseph Phillips	---*
 *---									---*
 *---	    Adapted to do more than just list remote files as the	---*
 *---	previous remoteFileListingServer.cpp.				---*
 *---									---*
 *-------------------------------------------------------------------------*/

//  Compile with:
//    $ g++ babyFTPServer.cpp -o babyFTPServer

#include	"babyFTPHeader.h"
#include	<sys/types.h>	// for open()
#include	<sys/stat.h>	// for open(), stat()
#include	<fcntl.h>	// for open()
#include	<dirent.h>	// for opendir(), readdir(), closedir()
#include	<wait.h>	// for wait()
#include	<signal.h>	// for sigaction()

using namespace std;

enum
{
  PORT_NUM_CMDLINE_INDEX			= 1,
  PASSWD_FILENAME_CMDLINE_INDEX
};

const int	MAX_NUM_QUEUING_CLIENTS		= 5;

#define		DIR_NAME			"."

#define		DEFAULT_PASSWORD_FILENAME	".babyFTPrc"


//  PURPOSE:  To serve as the SIGCHLD handler.  Ignores 'sig'.  No return
//	value.
void	sigchld_handler		(int	sig
				)
{
  int	status;
  pid_t	pid;

  //  YOUR CODE

  fflush(stdout);
}


//  PURPOSE:  To replace the first '\n' char appearing in the first 'maxLength'
//	chars of 'cPtr' with a '\0'.  If neither a '\n' nor a '\0' is found
//	in the first 'maxLength' chars then 'cPtr[maxLength-1]' is set to '\0'.
//	Returns 'cPtr', or NULL if 'maxLength==0'.
char*  	removeEndingNewline	(char*	cPtr,
				 uint	maxLength
				)
{
  //  I.  Applicability validity check:
  if  ( (cPtr == NULL) || (maxLength == 0) )
    return(NULL);

  //  II.  Remove ending newline char:
  for  (uint i = 0;  i < maxLength;  i++)
  {
    if  (cPtr[i] == '\0')
      return(cPtr);

    if  (cPtr[i] == '\n')
    {
      cPtr[i] = '\0';
      return(cPtr);
    }

  }

  //  III.  '\n' not found, end string and return:
  cPtr[maxLength-1] = '\0';
  return(cPtr);
}


//  PURPOSE:  To get the port number to use either from
//	'argv[PORT_NUM_CMDLINE_INDEX]' (if there are sufficient arguments
//	according to 'argc') or by asking the user.  Then to attempt to
//	open a listening socket on that port.  Returns file descriptor of
//	listening socket on success or '-1' otherwise.
int	createListeningSocket	(int		argc,
				 const char*	argv[]
				)
{
  //  I.  Applicability validity check:

  //  II.  Create listening socket:
  //  II.A.  Get desired port number:
  int 	     port;

  if  (argc > PORT_NUM_CMDLINE_INDEX)
    port = strtol(argv[PORT_NUM_CMDLINE_INDEX],NULL,0);
  else
  {
    char	text[MAX_STRING];

    printf("Desired port number? ");
    fgets(text,MAX_STRING,stdin);
    port = strtol(text,NULL,0);
  }

  //  II.B.  Create listening socket:
  int socketDescriptor;

  //  YOUR CODE HERE
  socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in socketInfo;
  memset(&socketInfo,'\0',sizeof(socketInfo));
  socketInfo.sin_family = AF_INET;
  socketInfo.sin_port = htons(port);
  socketInfo.sin_addr.s_addr = INADDR_ANY;

  int status = bind(socketDescriptor, (struct sockaddr*)&socketInfo, sizeof(socketInfo)); 
  
  if  (status < 0) {
    fprintf(stderr,"Could not bind to port %d\n",port);
    exit(EXIT_FAILURE);
  }

  listen(socketDescriptor,5);
  // done

  //  III.  Finished:
  return(socketDescriptor);
}


//  PURPOSE:  To get the password, either from the file specified in
//	'argv[PASSWD_FILENAME_CMDLINE_INDEX]' if there are sufficient
//	command line arguments according to 'argc', or from file
//	'DEFAULT_PASSWORD_FILENAME', or by asking user.  Up to 'maxLength-1'
//	chars of password written to 'password'.  Returns 'password'.
char*	obtainPassword	(int		argc,
			 const char*	argv[],
			 char* 		password,
			 uint		maxLength
			)
{
  //  I.  Applicability validity check:

  //  II.  Obtain password:
  const char*	passwordFilename= (argc > PASSWD_FILENAME_CMDLINE_INDEX)
  		 		  ? argv[PASSWD_FILENAME_CMDLINE_INDEX]
				  : DEFAULT_PASSWORD_FILENAME;

  FILE* passwordFilePtr;

  //  YOUR CODE HERE
  int pwFd = open(passwordFilename, O_RDONLY);

  if  (pwFd < 0) // CHANGE THIS -- done 
  {
    printf("Couldn't read %s, password for clients? ",passwordFilename);
    fgets(password,maxLength,stdin);
  }
  else
  {
    //  DEPENDING ON HOW YOU OPEN YOUR FILE, MAYBE SOMETHING HERE
    read(pwFd, password, maxLength-1);
  }

  removeEndingNewline(password,maxLength);

  //  III.  Finished:
  return(password);
}


//  PURPOSE:  To send 'GOOD_PASSWORD_RESPONSE' to the client over socket file
//	descriptor 'clientFD' and return 'true' if the password 'read()' from
//	'clientFD' matches 'password', or to send 'BAD_PASSWORD_RESPONSE' to
//	the client and return 'false' otherwise.
bool	didLogin	(int		clientFD,
			 const char*	password
			)
{
  //  I.  Application validity check:
  printf("Process %d authenticating user . . .\n",getpid());
  fflush(stdout);

  //  II.  See if user successfully logged-in:
  //  II.A.  Obtain user's password:
  char	buffer[MAX_LINE];

  //  YOUR CODE HERE
  read(clientFD, buffer, MAX_LINE);

  //  II.B.  Handle when user's password does NOT match:
  if (strncmp(removeEndingNewline(buffer,MAX_LINE), password, MAX_PASSWORD_LEN) != 0)
  {
    //  YOUR CODE HERE
    write(clientFD, BAD_PASSWORD_RESPONSE, strlen(BAD_PASSWORD_RESPONSE) + 1);
    printf("Process %d bad password.\n",getpid());
    return(false);
  }

  //  II.C.  If get here then user's password does match:
  //  YOUR CODE HERE
  write(clientFD, GOOD_PASSWORD_RESPONSE, strlen(GOOD_PASSWORD_RESPONSE) + 1);
  printf("Process %d good password.\n",getpid());

  //  III.  Finished:
  return(true);
}


//  PURPOSE:  To list the files in the server's current directory to the client
//	using file descriptor 'clientFD', followed by 'END_RESPONSE,MAX_LINE'.
//	No return value.
void	listDir		(int		clientFD
			)
{
  //  I.  Application validity check:
  printf("Process %d starting listing current directory.\n",getpid());
  fflush(stdout);

  //  II.  List files:
  //  II.A.  Attempt to open the directory:
  char	buffer[MAX_LINE];
  //  YOUR CODE HERE

  //  III.  Finished:
  printf("Process %d finished listing current directory.\n",getpid());
  fflush(stdout);
}


//  PURPOSE:  To send to the client using file descriptor 'clientFD' the file
//	whose name is 'read()' from 'clientFD'.  No return value.
void	getFile		(int		clientFD
			)
{
  //  I.  Applicability validity check:
  printf("Process %d attempting to read file.\n",getpid());
  fflush(stdout);

  //  II.  Attempt to get the file:
  //  II.A.  Attempt to 'read()' the filename from 'clientFD':
  char	buffer[MAX_LINE];
  char	filename[MAX_LINE];

  //  YOUR CODE HERE

  //  II.B.  Attempt to open file:
  //  YOUR CODE HERE

  //  II.C.  Attempt to get file's size:
  //  YOUR CODE HERE

  //  II.D.  Tell client about the file:
  printf("Process %d getting %s.\n",getpid(),buffer);
  fflush(stdout);

  //  II.D.1.  Tell client how many bytes the file has:
  //  YOUR CODE HERE

  //  II.D.2.  Tell client the chars of the file:
  //  YOUR CODE HERE

  //  II.E.  Close file:
  //  YOUR CODE HERE

  //  III.  Finished:
  printf("Process %d read file %s.\n",getpid(),<yourFileNameVar>);
  fflush(stdout);
}


//  PURPOSE:  To handle the client whom talking to over socket file descriptor
//	'clientFD' and that should authenticate with password 'password'.
//	Returns 'EXIT_SUCCESS' if session went well or 'EXIT_FAILURE'
//	otherwise.
int	handleClient	(int		clientFD,
			 const char*	password
			)
{
  //  I.  Application validity check:
  if  ( !didLogin(clientFD,password) )
  {
    close(clientFD);
    return(EXIT_FAILURE);
  }

  //  II.  Handle client:
  int	status	=	EXIT_SUCCESS;
  char	buffer[MAX_LINE];

  //  II.A.  Each iteration attempts to handle another
  //  	     command coming from the client:
  while  (true)
  {
    //  II.A.1.  'read()' command from client:
    int	numChars	= read(clientFD,buffer,MAX_LINE);

    if  (numChars < 0)
    {
      status = EXIT_FAILURE;
      break;
    }

    //  II.A.2.  Handle various commands:
    if  (strncmp(buffer,QUIT_CMD,QUIT_CMD_LEN) == 0)
    {
      printf("Process %d quitting.\n",getpid());
      fflush(stdout);
      break;
    }
    else
    if  (strncmp(buffer,LIST_CMD,LIST_CMD_LEN) == 0)
    {
      listDir(clientFD);
    }
    else
    if  (strncmp(buffer,GET_CMD,GET_CMD_LEN) == 0)
    {
      getFile(clientFD);
    }

  }

  //  III.  Finished:
  return(status);
}


//  PURPOSE:  To listen on socket 'listeningSocketId' for clients, let them
//	attempt to authenticate with password 'password', and to send list
//	of current directory entries to those that are successful.  No return
//	value.
void	doServer	(int		listeningSocketId,
			 const char*	password
			)
{
  //  I.  Applicability validity check:

  //  II.  Serve:

  //  II.A.  Each iteration serves one client
  bool	shouldServe = true;

  while  (shouldServe)
  {
    //  II.A.1.  Open communication with client:
    int		clientDescriptor	= accept(listeningSocketId,NULL,NULL);

    //  II.A.2.  Handle client:
    if  (fork() == 0)
    {
      exit(handleClient(clientDescriptor,password));
    }

    //  II.A.3.  Go back to listen for another client:
    close(clientDescriptor);
  }

  //  III.  Finished:
}


//  PURPOSE:  To get a port and function as a server on that port that sends
//	to clients a listing of the entries in the current directory.
//	First command line argument (if present) taken as port to use.
//	Second command line argument (if present) taken as filename containing
//	password to authenticate users.
int	main		(int		argc,
 			 const char*	argv[]
			)
{
  //  I.  Applicability validity check:

  //  II.  Do server:
  //  II.A.  Setup for server:
  char		password[MAX_PASSWORD_LEN];
  int		listeningSocketId;

  if  ( (listeningSocketId = createListeningSocket(argc,argv)) < 0 )
    return(EXIT_FAILURE);

  obtainPassword(argc,argv,password,MAX_PASSWORD_LEN);
  //  YOUR CODE HERE to make sigchld_handler() the handler for SIGCHLD
  struct sigaction act;
  memset(&act,'\0',sizeof(struct sigaction));
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
  act.sa_handler = sigchld_handler;
  sigaction(SIGCHLD,&act,NULL);
  // done
  
  //  II.B.  Do server:
  doServer(listeningSocketId,password);

  //  III.  Finished:
  close(listeningSocketId);
  return(EXIT_SUCCESS);
}

