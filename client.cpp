// Mason Kam, Steven Nguyen, Brandon No
// client.cpp

#include <iostream> /* cout function */
#include <cctype> /* tolower funtion */
#include <time.h> /* time function */
#include <cstdlib> /* srand and rand function */
#include "clientConnection.h"
#include <unordered_map>
#include <utility>
#include <queue>
using namespace std;

struct VoteMsg
{
  unsigned short magic;
  char flag;
  char type;
  unsigned int req_id;
  unsigned int checkSum;
  unsigned int candidateNum;
  unsigned int voteCount;
  unsigned int cookie;
};


//Welcome Screen
void welcome();
char menuChoice();

//Setting up message fields
unsigned short setMagic(int mal);
void setFlag(VoteMsg &buf, int mal);
void setType(VoteMsg &buf, int num, int mal);
unsigned int setID();
unsigned int setCheckSum(VoteMsg buf, int mal);
unsigned int setCandidateNum();

//Message handling: send/receive
void voteRequest();
void voteInquiry();
void setupMessage(VoteMsg &buf);
void convertHToN(VoteMsg &buf);
void convertNToH(VoteMsg *receive);

//Result of message
bool checkGoodResponse(VoteMsg *receive);
void printResults(VoteMsg *receive);

//Vote again and result of session
char voteAgain();
void statistics();
void goodbye();


const char YES = 'y';
const char NO = 'n';
const char CHOICE_A = 'a';
const char CHOICE_B = 'b';
const char CHOICE_C = 'c';
const int CLEAR = 50;
const int MISBEHAVING_CHANCE = 2;
const int MAX_ROLL = 10;
const int MIN_ROLL = 1;
const unsigned short MAGIC = 571;
const unsigned int CHECK_SUM = 3735928559;
const unsigned int MAX_SIZE = 4294967295;
const int CHAR_BITS = 8;
const int R_BIT = 6;
const int C_BIT = 7;
const int BAD_MESSAGE = 1;
const int HALF = 5;
const char VOTE_REQUEST = 0x18;
const char INQUIRY_REQUEST = 0x08;

int goodVotes = 0; //Votes requests with cookie
int badVotes = 0; //Votes requests without cookie
int goodInquiries = 0; //Inquiry with cookie
int badInquiries = 0; //Inquiry without cookie
int totalMessages = 0; //Total requests
int goodMessages = 0; //Total good requests
int badMessages = 0; //Total bad requests
int port;
string hostName;
unordered_map<unsigned int, unsigned int> inquiries;
queue<unsigned int> candKeys;

int main(int argc, char *argv[])
{
  char choice;
  bool done = false;

  hostName = argv[1]; //first argument
  port = atoi(argv[2]);     //second argument
  
  for (int i = 0; i < CLEAR; i++)
    cout << endl;

  welcome();
  srand(time(0));  

  while(!done){
	choice = menuChoice();
	
    if(choice == CHOICE_A)
	  voteRequest();
	else if (choice == CHOICE_B)
	  voteInquiry();
    else 
	  done = true;
	
    if(done == false)
	  if(voteAgain() == NO)
		done = true;
  }
  
  statistics();
  goodbye();  
  cout << endl << endl << endl;
  
  return 0;
}


void welcome()
{
  cout << "Welcome to iVote!" << endl;
  cout << "Here you may handle voting options for candidates."
	   << endl << endl;
}


char menuChoice()
{
  char selection;
  
  cout << "You may perform one of the following options:" << endl;
  cout << CHOICE_A << ") Vote for a candidate " << endl;
  cout << CHOICE_B << ") See vote count for a candidate " << endl;
  cout << CHOICE_C << ") Exit iVote " << endl << endl;

  cout << "Input the letter of the option you would like to perform: ";
  cin >> selection;

  while(tolower(selection) != CHOICE_A && tolower(selection) != CHOICE_B &&
		tolower(selection) != CHOICE_C){
	cout << "Please enter a valid option: ";
	cin >> selection;
  }
  
  return selection;
}


unsigned short setMagic(int mal = 0)
{
  return MAGIC + mal;
}


void setFlag(VoteMsg &buf, int mal = 0)
{
  //Clear bits before setting
  for(int i = 0; i < CHAR_BITS; i++)
	buf.flag &= ~(1 << i);

  //Set checksum bit
  buf.flag |= 1 << C_BIT;

  if(mal != 0)
	buf.flag |= 1 << R_BIT; //Bad message
}


void setType(VoteMsg &buf, int num = 0, int mal = 0)
{
  //Clear bits before setting
  for(int i = 0; i < CHAR_BITS; i++)
	buf.type &= ~(1 << i);

  if(num == 1)
	buf.type = VOTE_REQUEST;
  else
	buf.type = INQUIRY_REQUEST;
  
  //Set for both types
  if(mal != 0)
    buf.type |= 1 << mal; //Bad message
}


unsigned int setID()
{
  unsigned int ID;

  cout << "Input an ID number to identify yourself between 0 and "
	   << MAX_SIZE << ": ";
  cin >> ID;

  while(ID < 0 || ID > MAX_SIZE){
	cout << "Please input a ID number between 0 and " << MAX_SIZE << ": ";
	cin >> ID;
  }
  return ID;
}


unsigned int setCheckSum(VoteMsg buf)
{
  unsigned int *buf2 = (unsigned int*)&buf;
  unsigned int newCheckSum = buf2[0];

  //XOR it with all the bits
  for(int i = 1; i < 6; i++)
	newCheckSum = newCheckSum^buf2[i];
  
  return newCheckSum;
}


unsigned int setCandidateNum()
{
  unsigned int candNum;

  cout << "Input a candidate ID number between 0 and " << MAX_SIZE << ": ";
  cin >> candNum;

  while(candNum < 0 || candNum > MAX_SIZE){
	cout << "Please input an ID number between 0 and " << MAX_SIZE << ": ";
	cin >> candNum;
  }
  return candNum;
}

void setupMessage(VoteMsg &buf)
{
  int roll = ((rand() % MAX_ROLL) + MIN_ROLL);
  
  setFlag(buf);
  buf.voteCount = 0;
  buf.cookie = 0;
  buf.req_id = setID();
  buf.candidateNum = setCandidateNum();
  buf.magic = 0;
  buf.magic = setMagic();
  
  buf.checkSum = CHECK_SUM;
  
  //Malicious Message
  if(roll <= 2){
	if(roll == 1){
	  roll = ((rand() % MAX_ROLL) + MIN_ROLL);
	  if(roll <= HALF)
		setType(buf, 1, BAD_MESSAGE);
	  else
		setFlag(buf, BAD_MESSAGE);
	} else {
	  roll = ((rand() % MAX_ROLL) + MIN_ROLL);
	  if(roll <= HALF)
		buf.checkSum = buf.checkSum + BAD_MESSAGE;
	  else
		buf.magic = setMagic(BAD_MESSAGE);
	}
  }

  convertHToN(buf);
  buf.checkSum = setCheckSum(buf);
}


void convertHToN(VoteMsg &buf)
{
  buf.magic = htons(buf.magic);
  buf.req_id = htonl(buf.req_id);
  buf.checkSum = htonl(buf.checkSum);
  buf.candidateNum = htonl(buf.candidateNum);
  buf.voteCount = htonl(buf.voteCount);
  buf.cookie = htonl(buf.cookie);
}


void convertNToH(VoteMsg *receive)
{
  receive->magic = ntohs(receive->magic);
  receive->req_id = ntohl(receive->req_id);
  receive->checkSum = ntohl(receive->checkSum);
  receive->candidateNum = ntohl(receive->candidateNum);
  receive->voteCount = ntohl(receive->voteCount);
  receive->cookie = ntohl(receive->cookie);
}


void voteRequest()
{
  VoteMsg buf;
  VoteMsg *receive;
  bool good;
  char *msg;

  //Setting up the voting message
  setType(buf, 1);
  setupMessage(buf);
  
  msg = (char*)&buf;
  connection(msg, hostName, port);
  receive = (VoteMsg*)msg;
  convertNToH(receive);
  
  good = checkGoodResponse(receive);
  totalMessages++;
  
  if(good){
	goodMessages++;
	goodVotes++;
	printResults(receive);
  } else {
	badVotes++;
	badMessages++;
  }
}


void voteInquiry()
{
  VoteMsg buf;
  VoteMsg *receive;
  bool good;
  char *msg;
  
  setType(buf);
  setupMessage(buf);  

  msg = (char*)&buf;
  
  //send msg
  connection(msg, hostName, port);
  //receive msg
  receive = (VoteMsg*)msg;
  convertNToH(receive);
  
  good = checkGoodResponse(receive);
  totalMessages++;
  
  if(good){
	goodMessages++;
	goodInquiries++;
	printResults(receive);
  } else {
	badMessages++;
	badInquiries++;
  }

  if(::inquiries.count(receive->candidateNum) >= 1) {
	unsigned int tempInq = ::inquiries[receive->candidateNum];
	tempInq++;
	::inquiries[receive->candidateNum] = tempInq;
  } else { //if candidate doesn't have an inquiry yet
	::inquiries.insert(std::make_pair(receive->candidateNum, 1));
	::candKeys.push(receive->candidateNum); //adds the candidate key into queue
  }
  
}


bool checkGoodResponse(VoteMsg *receive)
{
  bool goodMessage = true;

  // Check Flag Bits
  if(((receive->flag >> 0) & 1) == 1){ //Type bit
	goodMessage = false;
	cout << "Error...type field invalid." << endl;
  }
  if(((receive->flag >> 1) & 1) == 1){ //Checksum bit
	goodMessage = false;
	cout << "Error...checksum incorrect." << endl;
  }
  if(((receive->flag >> 2) & 1) == 1){ //Magic bit
	goodMessage = false;
	cout << "Error...magic number was incorrect." << endl;
  }
  if(((receive->flag >> 3) & 1) == 1){ //Request bit
	goodMessage = false;
	cout << "Error...R bit set to response in a request." << endl;
  }

  return goodMessage;
}


void printResults(VoteMsg *receive)
{
  cout << "Results of vote..." << endl;
  cout << "User ID: " << receive->req_id << endl;
  cout << "Candidate ID: " << receive->candidateNum << endl;
  cout << "Vote Count: " << receive->voteCount << endl;
  cout << "Receipt #: " << receive->cookie << endl;
}


char voteAgain()
{
  char input;

  cout << endl << endl;
  cout << "Do you want to see the menu again? (y/n): ";
  cin >> input;

  while(tolower(input) != YES && tolower(input) != NO){
	cout << "Please enter y or n: ";
	cin >> input;
  }

  if(tolower(input) == NO)
	return NO;
  
  return YES;
}



void statistics()
{
  cout << endl << endl;
  cout << "Activity for this session..." << endl;
  cout << "Number of vote requests that were valid: " << goodVotes << endl;
  cout << "Number of vote requests that were invalid: " << badVotes << endl;
  cout << "Number of inquiries that were valid: " << goodInquiries << endl;
  cout << "Number of inquiries that were invalid: " << badInquiries << endl;
  cout << "Candidate # | Total Inquiries:" << endl;
  while(!::candKeys.empty()) {
	cout << ::candKeys.front() << " | " <<
	  ::inquiries[::candKeys.front()] << endl;
	::candKeys.pop();
  }
  cout << "Total number of valid requests: " << goodMessages << endl;
  cout << "Total number of invalid requests: " << badMessages << endl;
  cout << "Total number of requests: " << totalMessages << endl;

  
}


void goodbye()
{
  cout << "Thank you for using iVote!" << endl;
  cout << "Have a nice day!" << endl;  
}
