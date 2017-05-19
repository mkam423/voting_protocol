//Steven Nguyen, Brandon No, Mason Kam

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <cassert>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

const int BUFLEN = 24;

int connection(char *in, string hostName, int port)
{
  /*
  for(int a = 0; a < 24; a++){
	cout << "byte #: " << a << endl;
	for(int i = 0; i < 8; i++){
	  if(((in[a] >> i) & 1) == 1)
		cout << "bit " << i << ": 1" << endl;
	  else
		cout << "bit " << i << ": 0" << endl;
	}

	cout << endl << endl;
  }
  */

  int mainSocket,
	check = -1,
	rc,
	bcount = 0;

  char buf[BUFLEN];

  for(int i = 0; i < BUFLEN; i++)
	buf[i] = in[i];
  
  char *pc;
	
  /* Address resolution stage */
  struct hostent* server;
  struct sockaddr_in serv_addr;

  /*getting server name, which is cs1 server*/
  //cout << "Please enter server name"
  //   << "(Hint: cs1.seattleu.edu): ";
  //cin >> hostName;

  server = gethostbyname(&hostName[0]);
  if (!server) {
	perror("couldn't resolve host name");
  }
  
  /*now setting up the connection*/
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port); //server port number
  memcpy(&serv_addr.sin_addr.s_addr,
		 server->h_addr_list[0], server->h_length);

  /*now allocate a socket*/
  mainSocket = socket(AF_INET, SOCK_STREAM, 0);
  assert(mainSocket >= 0); //I am just lazy here!!

  /*now connect to socket*/
  while(check < 0){
	check = connect(mainSocket,
					(struct sockaddr *)&serv_addr, sizeof(serv_addr));
	//need to check return value!
  }

  pc = buf;

  //send
  while(bcount < BUFLEN){
	if ( (rc = write(mainSocket, pc, BUFLEN - (pc - buf))) > 0){
	  pc += rc;
	  bcount += rc;
	} else
	  return -1;
  }
  //send it to server, better use while loop

  pc = buf;
  //get
  bcount = rc = 0;
  while(bcount < BUFLEN){
	if ( (rc = read(mainSocket, pc, BUFLEN - (pc - buf))) > 0){
	  pc += rc;
	  bcount += rc;
	} else{
	  return -1;
	}
  }


  for(int i = 0; i < BUFLEN; i++)
	in[i] = buf[i];

  memset(buf, 0, BUFLEN);
  //pc = buf;
  //while( (rc = read(mainSocket, pc, BUFLEN - (pc - buf))) > 0 )
  //pc += rc;
  
  close(mainSocket);

  return 0;
}



	
