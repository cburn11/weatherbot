#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <syslog.h>

#include <string>
#include <iostream>
#include <memory>

#include "utils.h"
#include "GenericHandler.h"

std::string g_clientFifoPath;
int g_clientFd;
int g_dummyFd;

void ProcessPostBody(const char * szBody) {

  int serverFd = open(szServerFifoPath, O_WRONLY);

  write_fifo_msg(serverFd, szBody);

  close(serverFd);  
}

std::string GetResponseBody() {

  std::string Body;

  if( g_clientFd > 0 ) { 
  
    char * szMsg;
    int cbRead;
    if( (cbRead = read_fifo_msg(g_clientFd, &szMsg)) > 0 ) {

      Body = szMsg;
      free(szMsg);
    }

    close(g_clientFd);
    close(g_dummyFd);

  }
  
  unlink(g_clientFifoPath.c_str());
  
  return Body;
}

void InitClientFifo(pid_t pid) {

  g_clientFifoPath = "/var/spool/cgi/client_fifo_";
  g_clientFifoPath += std::to_string(pid);

  if ( mkfifo(g_clientFifoPath.c_str(), S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP) == -1 && errno != EEXIST ) {
    syslog(LOG_ERR, "mkfifo(%s) returned -1, errno =  %d", g_clientFifoPath.c_str(), errno);  
  }

  g_clientFd = open(g_clientFifoPath.c_str(), O_RDONLY | O_NONBLOCK);
  if( g_clientFd == -1) {
    syslog(LOG_ERR, "open(%s) returned -1, errno =  %d", g_clientFifoPath.c_str(), errno);
  }

  int flags;
  flags = fcntl(g_clientFd, F_GETFL); /* Fetch open files status flags */
  flags = flags & ~O_NONBLOCK; /* Enable O_NONBLOCK bit */
  fcntl(g_clientFd, F_SETFL, flags); /* Update open files status flags */
  
  g_dummyFd = open(g_clientFifoPath.c_str(), O_WRONLY);
}


int main(int argc, char * argv[], char * env[]) {

  umask(0);
  
  std::string Status_Line = "HTTP/1.1 200 OK\r\n";
  std::string Content_Header = "Content-Type: application/json\r\n";

  int cbLength = atoi(getenv("CONTENT_LENGTH"));

  std::string content_type = getenv("CONTENT_TYPE");

  std::string Body;
  
  if( content_type == "application/x-www-form-urlencoded" ) {

    Body = "{\"response-type\": \"ephemeral\"}";
    Body += "\r\n"; 
  }
    
  char * content_buffer = new char[cbLength + 1]{0};
  if( content_buffer) {

    int cbRead = read(0, content_buffer, cbLength);
    if( cbRead > 0 ) {
      std::string aggregate = content_type + "&";
      if( content_type == "application/json" ) {
	pid_t pid = getpid();
	aggregate += std::to_string(pid) + "&";
	InitClientFifo(pid);
      }

      aggregate += + content_buffer;
      ProcessPostBody(aggregate.c_str());

      if( content_type == "application/json" )
	Body += GetResponseBody();
    }

    delete[] content_buffer;
  }

  std::string Length_Header = "Content-Length: " + std::to_string(Body.size()) + "\r\n";

  std::string Response = Status_Line + Content_Header + Length_Header + "\r\n" + Body;

  std::cout << Response;


  return 0;
}
