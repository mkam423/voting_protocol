#include <iostream>
#include <netinet/in.h>
using namespace std;

struct VoteMsg
{
  unsigned short magic;
  unsigned char flag;
  unsigned char type;
  unsigned int req_id;
  unsigned int checkSum;
  unsigned int candidateNum;
  unsigned int voteCount;
  unsigned int cookie;
};

int main()
{
  /*
  uint32_t some_long = 10;
  uint32_t network_byte_order;
  network_byte_order = htonl(some_long);

  cout << network_byte_order << endl;
  cout << &network_byte_order << endl;

  network_byte_order = ntohl(some_long);
  cout << network_byte_order << endl;
  cout << &network_byte_order << endl;
  */

  VoteMsg msg;
  char *buf2;

  msg.magic = 571;
  msg.flag = 0;
  msg.type = 0;
  msg.req_id = 1;
  msg.checkSum = 0;
  msg.candidateNum = 0;
  msg.voteCount = 0;
  msg.cookie = 0;
  
  buf2 = (char*)&msg;
  //VoteMsg msg2 = (VoteMsg)&buf2;
  
  cout << buf2 << endl;
  cout << *buf2 << endl;
  cout << &buf2 << endl;
  //buf = (VoteMsg)*buf2;
  /*
  for(int a = 0; a < 1; a++){
	cout << "clear byte #: " << a << endl;
	for(int i = 0; i < 8; i++){
	buf.magic &= ~(1 << i);
	}
  }
	buf[0] |= 1 << 0;
	buf[0] |= 1 << 1;
	buf[0] |= 1 << 3;
	buf[1] |= 1 << 0;
  */
  for(int a = 0; a < 24; a++){
	cout << "byte #: " << a << endl;
	for(int i = 0; i < 8; i++){
	  if(((buf2[a] >> i) & 1) == 1)
		cout << "bit " << i << ": 1" << endl;
	  else
	    cout << "bit " << i << ": 0" << endl;
	}
	cout << endl << endl;
  }

  
  return 0;

}
