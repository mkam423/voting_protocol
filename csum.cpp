

#include <iostream>
using namespace std;

unsigned short fun(unsigned short checkSum, unsigned short stuff,
				   unsigned short l);


int main()
{
  unsigned short checkSum = 1;
  unsigned short stuff = 1;
  unsigned short l = 2;
  
  for(int i = 0; i < 16; i++){
	if(((checkSum >> i) & 1) == 1)
	  cout << "bit " << i << ": 1" << endl;
	else
	  cout << "bit " << i << ": 0" << endl;
  }

  cout << endl << endl;
  
  for(int i = 0; i < 16; i++){
	if(((stuff >> i) & 1) == 1)
	  cout << "bit " << i << ": 1" << endl;
	else
	  cout << "bit " << i << ": 0" << endl;
  }

  cout << endl << endl;
  
  for(int i = 0; i < 16; i++){
	if(((l >> i) & 1) == 1)
	  cout << "bit " << i << ": 1" << endl;
	else
	  cout << "bit " << i << ": 0" << endl;
  }

  cout << endl << endl << endl;

  checkSum = fun(checkSum, stuff, l);

  cout << "result:" << endl;
  for(int i = 0; i < 16; i++){
	if(((checkSum >> i) & 1) == 1)
	  cout << "bit " << i << ": 1" << endl;
	else
	  cout << "bit " << i << ": 0" << endl;
  }
  
	return 0;

}


unsigned short fun(unsigned short checkSum, unsigned short stuff,
				   unsigned short l)
{
  return checkSum ^ stuff ^ l;
}
