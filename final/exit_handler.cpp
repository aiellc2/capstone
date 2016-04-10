#include "header.hpp"

void exit_handler(int s){
           printf("Caught signal %d\n",s);
           system("killall speech");
           usleep(500000);
           std::terminate();
           //exit(0);

}
