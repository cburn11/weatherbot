#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

#include <syslog.h>

#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "utils.h"
#include "daemon.h"
#include "weatherbot.h"
#include "rest.h"

void handle_msg(char * qs);

int main(int argc, char * argv[]) {

  becomeDaemon(0);

  umask(0);

  auto pPasswd = getpwnam("weatherbot");
  setgid(pPasswd->pw_gid);
  setuid(pPasswd->pw_uid);
  
  if ( mkfifo(szServerFifoPath, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP) == -1 && errno != EEXIST ) {
    
    return 1;
  }

  int serverFd = open(szServerFifoPath, O_RDONLY);
  if( serverFd == -1) {

    return 1;
  }

  int dummyFd = open(szServerFifoPath, O_WRONLY);

  openlog(NULL, 0, LOG_DAEMON);
  syslog(LOG_INFO, "started.");
  
  for( ; ; ) {

    char * szMsg;
    int cbRead;
    if( (cbRead = read_fifo_msg(serverFd, &szMsg)) > 0 ) {

      syslog(LOG_INFO, "received %i bytes (%s).", cbRead, szMsg);
      
      handle_msg(szMsg);
      free(szMsg);
    }
  }

  close(serverFd);
  close(dummyFd);
  
  closelog();
  
  return 0;
}

void handle_msg(char * qs) {

  std::string text;
  std::string user_name;
  std::string response_url;

  
  if( qs ) {

    char * token = strtok(qs, "&");

    while( token ) {

      char * var = token;
      char * val = NULL;

      char * amp = strchr(token, '=');
      if( amp ) {
        *amp = '\0';
        val = amp + 1;
	
        if( strcmp(var, "text") == 0 ) {
          text = val;
        } else if( strcmp(var, "user_name") == 0 ) {
          user_name = val;
	} else if( strcmp(var, "response_url") == 0 ) {
	  response_url = val;
	}
	
      }

      token = strtok(NULL, "&");
    }
  }

  syslog(LOG_INFO, "text = %s, user_name = %s, response_url = %s",
	 text.c_str(), user_name.c_str(), response_url.c_str());
  
  if( response_url == "" )
    return;

  std::string response;
  
  if( text != "" ) {

    std::string loc_address;
    double latitude, longitude;

    GetCoords(text, loc_address, latitude, longitude);

    auto forecast_url = GetForecastUrl(latitude, longitude);

    auto forecast =  GetForecast(forecast_url);

    response = "#### ";
    response += loc_address;
    response += "\n";
    response += forecast;
    
  } else {

    const char * szForecast = "/tmp/forecast";
    
    int forecastId = open(szForecast, O_RDONLY);
    struct stat st;
    fstat(forecastId, &st);
    char * response_buffer = new char[st.st_size + 1]{0};
    if( response_buffer) {
      read(forecastId, response_buffer, st.st_size);
      response = response_buffer;
      delete[] response_buffer;
    } else {
      response = "Error loading forecast.";
    }
    
    close(forecastId);

    
  }

  SendResponse(response_url, response);
  
}
