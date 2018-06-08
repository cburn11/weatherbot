#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <string>
#include <iostream>

#include "utils.h"
#include "GenericHandler.h"

int main(int argc, char * argv[], char * env[]) {

  if( argc < 2 )
    std::cout << "Need string as second arg." << std::endl;
  
  int serverFd = open(szServerFifoPath, O_WRONLY);

  std::cout << "Sending: " << argv[1] << std::endl;
  
  write_fifo_msg(serverFd, argv[1]);

  close(serverFd);  
  
  return 0;
}
