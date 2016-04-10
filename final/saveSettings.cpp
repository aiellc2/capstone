#include "header.hpp"

void saveSettings(int a, int b, int c,\
                  int d, int e, int f,\
                  int g, int h, int i,\
                  int j, int k, int l,\
                  int m, int n, int o,\
                  int p, int q, int r){

  std::ofstream myfile;
  myfile.open ("parameters.txt");
  //myfile << "Writing this to a file.\n";
  myfile << a;
  myfile << "\n";
  myfile << b; //low1
  myfile << "\n";
  myfile << c;
  myfile << "\n";

  myfile << d;
  myfile << "\n";
  myfile << e;//high1
  myfile << "\n";
  myfile << f;
  myfile << "\n";

  myfile << g;
  myfile << "\n";
  myfile << h;//low2
  myfile << "\n";
  myfile << i;
  myfile << "\n";

  myfile << j;
  myfile << "\n";
  myfile << k;//high2
  myfile << "\n";
  myfile << l;
  myfile << "\n";

  myfile << m;
  myfile << "\n";
  myfile << n;//high2
  myfile << "\n";
  myfile << o;
  myfile << "\n";

  myfile << p;
  myfile << "\n";
  myfile << q;//high2
  myfile << "\n";
  myfile << r;
  myfile << "\n";



  myfile.close();

}
