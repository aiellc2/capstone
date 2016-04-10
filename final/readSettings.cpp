#include "header.hpp"

int readSettings(int index){
  int line;
  std::ifstream myfile ("parameters.txt");
  if (myfile.is_open())
  {
    for(int i = 0; i<18; i++)
    {
      //getline (myfile,line);
      myfile >> params[i];
      //std::cout << i<<" "<<params[i] << '\n';

    }

    myfile.close();
    std::cout<<"Imported previous parameters\n";
  }


  return line;
  //return 0;

}
