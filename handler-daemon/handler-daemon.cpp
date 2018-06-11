#include <unistd.h>
#include <sys/stat.h>

#include <string>
#include <utility>

#include "Connections.hxx"
#include "GenericHandler.h"

int main(int argc, char * argv[]) {

  umask(0);
  
  // Parse command line;

  std::string strConfigPath{"config/config.json"};
  
  extern char * optarg;
  extern int optind, opterr, optopt;
  const char * szOpt = ":c:"; 
  int opt;

  while( (opt = getopt(argc, argv, szOpt)) > 0 ) {

    switch(opt) {

    case 'c':
      strConfigPath = optarg;
      break;
    }
  }


  
  // Load handler
  
  ConnectionHandler handler{std::move(strConfigPath)};

  for( ; handler.ListenForConnections(); )  {

  }

  return 0;
}
