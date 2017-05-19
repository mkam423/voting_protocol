//Server Work

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>
#include <queue>
#include <utility>
#include <iostream>
#include <stdlib.h>
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

unordered_map<unsigned int, unsigned int> votes; // KEY = CandidateNum, VALUE = voteCount
queue<unsigned int> candKeys; // QUEUE of KEYs that exist in HASHTABLE (unordered_map)
queue<unsigned int> cookies; // QUEUE of cookies that have been created

int stats[3] = {0, 0, 0}; //0 == voteMessages, 1 == InquiryMessages, 2 ==malformedMessages

///////////////////////////////////////////////// Function Definitions
void readIn(char* buf2);
void processMess(VoteMsg &msg);
bool isMagicEqual(VoteMsg &msg);
bool isCheckCalc(unsigned char *flagIn);
bool verifyCheck(VoteMsg &msg);
void isRSet(VoteMsg &msg);
void setReserve(VoteMsg &msg);
void isTypeValid(VoteMsg &msg);
void decideResponse(VoteMsg &msg);
void convertToBuffer(VoteMsg &msg);
void doVote(VoteMsg &msg);
void doInq(VoteMsg &msg);
unsigned int compute_csum(VoteMsg &msg);
void printStats();
unsigned int bakeCookie();
//////////////////////////////////////////////// End of Function Definitions

void readIn(char* buf2) {

  VoteMsg *msgIn = (VoteMsg*)buf2;
  VoteMsg temp;
  //Convert to Struct
    
  //check C-Bit
  temp.flag = msgIn->flag;
  if((msgIn->flag >> 7) & 1) {//c bit is 1
	//do checksum stuff
	temp.checkSum = compute_csum(*msgIn);
  } else { //C-Bit is zero
	temp.flag &= ~(1 << 7); // c-bit to zero
	temp.checkSum = 0; // checksum = 0
  }
  

  temp.magic = ntohs(msgIn->magic);
  temp.req_id = ntohl(msgIn->req_id);
  temp.checkSum = ntohl(temp.checkSum);
  temp.candidateNum = ntohl(msgIn->candidateNum);
  temp.voteCount = ntohl(msgIn->voteCount);
  temp.cookie = ntohl(msgIn->cookie);
  temp.type = msgIn->type;

  if(temp.checkSum != 0) {
	if(temp.checkSum != 0xdeadbeef) {
	  temp.flag |= (1 << 1); // S bit = 1
	}
  }
  
  
  processMess(temp); //This function updates error bits(3, 2, 1, 0) and checking
  decideResponse(temp); //Determines if well structured, then vote or inquiry depending on type bit
  
  //Convert struct back to char* buffer
  msgIn->magic = htons(temp.magic);
  msgIn->req_id = htonl(temp.req_id);
  msgIn->checkSum = htonl(temp.checkSum);
  msgIn->candidateNum = htonl(temp.candidateNum);
  msgIn->voteCount = htonl(temp.voteCount);
  msgIn->cookie = htonl(temp.cookie);
  
  msgIn->flag = temp.flag;
  msgIn->type = temp.type;

  msgIn->checkSum = compute_csum(*msgIn);



}

//This function updates error bits(3,2,1,0)
void processMess(VoteMsg &msg) {
	
	//Check R bit and update P bit
	isRSet(msg);
	//Set Reserve bits (4,5) to ZERO
	setReserve(msg);
	//Check M bit
	isMagicEqual(msg);
	//Check T bit
	isTypeValid(msg);
}

//Decide if there are errors or not. If no errors, either vote or inquiry depending on TYPE bit
void decideResponse(VoteMsg &msg) {
  msg.flag |= (1 << 6); //Set R bit to RESPONSE (1)
  if(((msg.flag >> 3) & 1 ) == 1 || ((msg.flag >> 2) & 1 ) == 1
	 || ((msg.flag >> 1) & 1 ) == 1 || ((msg.flag >> 0) & 1 ) == 1) {

	//msg.checkSum = compute_csum(msg); //Recalculate checkSum
	stats[2]++; //malfunction++
  } else {

	if(msg.type == 0x18) {
	  doVote(msg);
	  stats[0]++; //vote++
	}
	if(msg.type == 0x08) {
	  doInq(msg);
	  stats[1]++; //inquiry++
	}
  }
}


void doVote(VoteMsg &msg) {
  unsigned int tempCand = msg.candidateNum;
  unsigned int finVoteCount;
  
  if(::votes.count(tempCand) >= 1) {  //If candidate has at least one vote
	unsigned int tempVote = ::votes[tempCand];
	tempVote++;
	::votes[tempCand] = tempVote;
	finVoteCount = tempVote;
  } else { //if candidate doesn't have a vote yet
	::votes.insert(std::make_pair(tempCand, 1));
	finVoteCount = 1;
	::candKeys.push(tempCand); //adds the candidate key into queue
  }
  //Response work to be sent later to client
  msg.voteCount = finVoteCount; //update voteCount
  msg.cookie = bakeCookie(); //set cookie
}

//Do Inquiry 
void doInq(VoteMsg &msg) {
  unsigned int tempCand = msg.candidateNum;
  if(::votes.count(tempCand) == 1) { //if candidate exists in hashtable
	msg.voteCount = ::votes[tempCand]; //retrieve from map value and set in message
  } else {
	msg.voteCount = 0; //if not in map, set voteCount in message to 0 (no votes yet)
  }
  msg.cookie = bakeCookie(); //set cookie
}

unsigned int bakeCookie(){
  srand(time(NULL));
  unsigned int cookieNew = rand(); // generate random cookie
  ::cookies.push(cookieNew); //Tracks cookies in queue
  return cookieNew;
}

//Return true if C bit is set (means we have to verify checksum thru the compute_csum function)
bool isCheckCalc(unsigned char *flagIn) {
	if((*flagIn >> 7) & 1) {
		return true;
	}
	return false;
  }

  //Check if magic = 0x023B (validity check)
bool isMagicEqual(VoteMsg &msg) {
  if(msg.magic != 0x023B) {
	msg.flag |= (1 << 2); //M bit = 1
	if((msg.flag >> 2) & 1)

	  
	return false;
  }
  return true;
}

  //Check if CheckSum calculation = 0xdeadbeef
bool verifyCheck(VoteMsg &msg) {
	if(msg.checkSum == 0xdeadbeef) {
		return true;
	} else {
	  return false;
	}
}  

//Check R bit to see if request or response
void isRSet(VoteMsg &msg) {
	if((msg.flag >> 6) & 1) {
		msg.flag |= (1 << 3); //P bit = 1
	}


}
  
//Sets Reserve Bits to 0
void setReserve(VoteMsg &msg) {
	msg.flag |= (0 << 5);
	msg.flag |= (0 << 4);
}

//Check Type Validity
void isTypeValid(VoteMsg &msg) {

	if((msg.type != 0x18) && (msg.type != 0x08)) {
		msg.flag |= (1 << 0);//T bit = 1 if type is invalid
	}
}

//Prints statistics
void printStats() {
	cout << "# of Vote Messages: " << ::stats[0] << endl;
	cout << "# of Inquiry Messages: " << ::stats[1] << endl;
	cout << "# of Malformed Requests: " << ::stats[2] << endl;
	cout << "Candidate # | Total Votes" << endl;
	
	while(!::candKeys.empty()) {
	  cout << ::candKeys.front() << " | " <<
		::votes[::candKeys.front()] << endl;
		::candKeys.pop();
	}
	cout << "End of Server Statistics." << endl << endl;
}

//Compute CheckSum
unsigned int compute_csum(VoteMsg &msg)
{
	unsigned int *buf2 = (unsigned int*) &msg;
	unsigned int newCheckSum = buf2[0];

	for(int i = 1; i < 6; i++) {
		newCheckSum = newCheckSum ^ buf2[i];
	}
	return newCheckSum;
}
