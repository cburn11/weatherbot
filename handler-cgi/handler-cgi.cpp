#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <string>
#include <iostream>

#include "utils.h"
#include "GenericHandler.h"

void ProcessPostBody(char * szBody) {

  int serverFd = open(szServerFifoPath, O_WRONLY);

  write_fifo_msg(serverFd, szBody);

  close(serverFd);  
}


int main(int argc, char * argv[], char * env[]) {

  std::string Status_Line = "HTTP/1.1 200 OK\r\n";
  std::string Content_Header = "Content-Type: application/json\r\n";

  std::string Body = "{\"response-type\": \"ephemeral\"}";
  Body += "\r\n";

  std::string Length_Header = "Content-Length: " + std::to_string(Body.size()) + "\r\n";

  std::string Response = Status_Line + Content_Header + Length_Header + "\r\n" + Body;

  std::cout << Response;

  int cbLength = atoi(getenv("CONTENT_LENGTH"));
  char * content_buffer = new char[cbLength + 1]{0};
  if( content_buffer) {

    int cbRead = read(0, content_buffer, cbLength);
    if( cbRead > 0 )
      ProcessPostBody(content_buffer);

    delete[] content_buffer;
  }

  return 0;
}
