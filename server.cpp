//Steven Nguyen, Brandon No, Mason Kam
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <queue>
#include <signal.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "serverFunction.h"
#define PORT 10080

using namespace std;

void *dealWithMsg(void *_arg);

void wait_connection();

void handler_SIGINT();

const int BUFLEN = 24; //8 unsigned per line
                        //6 lines in a message
const int SUB_SERVER_SIZE = 20;

pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;

pthread_t subServer[SUB_SERVER_SIZE];
bool running[SUB_SERVER_SIZE];

//variable for semaphore
queue<int> socketWait;
queue<struct sockaddr_in> clientAddr;
sem_t socket_count;

bool done = false;

int main(int argc, char *argv[])
{
  struct sigaction handler;
  handler.sa_handler = (void(*)(int))handler_SIGINT;
  sigaction(SIGINT, &handler, NULL);
  
  //Initialize job_count
  sem_init(&socket_count, 0, 0);
  //creating the thread pool
  for(int i = 0; i < SUB_SERVER_SIZE; i++)
	if (-1 == pthread_create(&(subServer[i]), NULL, dealWithMsg, NULL)){
	  perror("pthread_create");
	  return -1;
	  }
  
  //ask about how to take care of when one thread cannot be created

  wait_connection();
	 
  return 0;
}

void *dealWithMsg(void *_arg)
{
  struct sockaddr_in cli_addr;
  int bcount = 0,
	rc,
	threadSocket;
  char buf[BUFLEN] = {0};
  char *pc;
  
  memset(buf, 0, BUFLEN);
  pc = buf;

  sem_wait(&socket_count);
  /*access the criticle section*/
  //if(!done){
	pthread_mutex_lock(&mutexLock);
	//Get the socket
	threadSocket = socketWait.front();
	socketWait.pop();
	//Get the client address
	cli_addr = clientAddr.front();
	clientAddr.pop();
	
	pthread_mutex_unlock(&mutexLock);
	//}

  /*Keep reading in until finish*/
  while(bcount < BUFLEN){
	if ( (rc = read(threadSocket, pc, BUFLEN - (pc -buf))) > 0){
	  pc += rc;
	  bcount += rc;
	} else
	  return 0;
  }

  readIn(buf);

  pc = buf;
  bcount = 0;
  while(bcount < BUFLEN){
	if ( (rc = write(threadSocket, pc, BUFLEN - (pc -buf))) > 0){
	  pc += rc;
	  bcount += rc;
	} else
	  return 0;
  }
  
  close(threadSocket);
  return 0;
}

void wait_connection()
{
  int mainSocket, /*this is for server socket*/
	rc,           /*this is for binding*/
	newSocket;

  unsigned size;
  
  /* Address resolution stage */
  struct sockaddr_in serv_addr, cli_addr;

  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  //any IP addr. Is accepted

  /*Initialize the socket*/
  mainSocket = socket(AF_INET,SOCK_STREAM, 0);
  assert(mainSocket >= 0); /*check if can allocate the socket*/

  /*Bind the socket */
  if( (rc = bind(mainSocket, (struct sockaddr *)&serv_addr,
				 sizeof(serv_addr))) < 0)
	return;

  while(!done){
	
	rc = listen(mainSocket, 10); //allow 10 connection to wait on the list
	
	size = sizeof(cli_addr);
	newSocket = accept(mainSocket, (struct sockaddr*)&cli_addr, &size);
	
	pthread_mutex_lock(&mutexLock);
	socketWait.push(newSocket);
	clientAddr.push(cli_addr);

	sem_post(&socket_count);
	pthread_mutex_unlock(&mutexLock);
	

  }
  close(mainSocket);
}

void handler_SIGINT()
{
  done = true;
  //--------------------------------------------------------------//
  //main thread maybe asleep, so no other thread can wake it up   //
  //--------------------------------------------------------------//

  printStats();  
  
  for(int i = 0; i < SUB_SERVER_SIZE; i++){
	pthread_join(subServer[i], NULL);
  }
}

//Prints statistics

//-----------------------------------------------------------------//
//struct hostent {                                                 //
//   char* h_name;                                                 //
//   /* official name of host */                                   //
//                                                                 //
//   char** h_aliases;                                             //
//   /* alias list */                                              //
//                                                                 //
//   int h_addrtype;                                               //
//   /* host address type */                                       //
//                                                                 //
//   int h_length;                                                 //
//   /* length of address */                                       //
//                                                                 //
//   char** h_addr_list;                                           //
//   /* list of addresses from name server */                      //
//                                                                 //
//   #define h_addr h_addr_list[0]                                 //
//   /* address, for backward compatibility */                     //
//-----------------------------------------------------------------//

//-----------------------------------------------------------------//
//structure of sockaddr_in                                         //
//struct sockaddr_in {                                             //
//   short sin_family;                                             //
//   u_short sin_port;                                             //
//   struct in_addr sin_addr;                                      //
//   char sin_zero[8];                                             //
//};                                                               //
//-----------------------------------------------------------------//

//-----------------------------------------------------------------//
//memset(void *ptr, int value, size_t num);                        //
//Memset is used to the first num bytes of the block of memory     //
//   pointed by ptr to the specified value                         //
//-----------------------------------------------------------------//

//Initialize sa
//sin_family = AF_INET         //specifies which address family
                               //is being used
//sin_port: port # (0-65535)   /*port number*/
//sin_addr: IP-address
//sin_zero: unused

//-----------------------------------------------------------------//
//int status = listen(sock, queuelen);                             //
//   status: 0 if listening, -1 if error                           //
//   sock: integer, socket descriptor                              //
//   queuelen: integer, # of active participants that              //
//             can “wait”ection                         //
//   listen is non-blocking: returns immediately                   //
//-----------------------------------------------------------------//

//-----------------------------------------------------------------//
//int s = accept(sock, &name, &namelen);                           //
//   s: integer, the new socket (used for data-transfer)           //
//   sock: integer, the orig. socket (being listened on)           //
//   name: struct sockaddr, address of the active participant      //
//   namelen: sizeof(name): value/result parameter                 //
//       -must be set appropriately before call                    //
//       -adjusted by OS upon return                               //
//   accept is blocking: waits for connection before returning     //
//-----------------------------------------------------------------//
