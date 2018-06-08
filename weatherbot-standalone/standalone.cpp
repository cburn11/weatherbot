#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include <unistd.h>
#include <errno.h>

#include <time.h>

#include <string>
#include <iostream>

#include "http.h"

int main(int argc, char * argv[]) {

  std::string default_url = "https://api.weather.gov/gridpoints/VEF/122,97/forecast";
  std::string url;
  std::string location;
  std::string cache_path;
  std::string request_url;
  std::string in_path;
  std::string channel;
  
  bool fSilent = false;
  
  extern char * optarg;
  extern int optind, opterr, optopt;
  const char * szOpt = ":u:l:o:w:c:i:s"; 
  int opt;
  while( (opt = getopt(argc, argv, szOpt)) > 0 ) {

    switch(opt) {

    case 'u':
      url = optarg;
      break;

    case 'l':
      location = optarg;
      break;
      
    case 'o':
      cache_path = optarg;
      break;

    case 'w':
      request_url = optarg;
      break;

    case 'i':
      in_path = optarg;
      break;

    case 'c':
      channel = optarg;
      break;

    case 's':
      fSilent = true;
      break;
    }
  }

  std::string loc_address{"Las Vegas, NV"};

  std::string json_str;
  
  if( in_path == "" ) {
  
    if( location != "" && url == "" ) {

      double latitude, longitude;

      GetCoords(location, loc_address, latitude, longitude);

      url = GetForecastUrl(latitude, longitude);
    
    } else if( location == "" && url == "" ) {

      url = default_url;
    }
 
    std::string forecast = GetForecast(url);
    std::string temperature, icon_url;
    GetForecastCurrentTempAndIcon(url, temperature, icon_url);
 
    std::string header = "#### " + loc_address + "\n";
    if( temperature != "" ) {
    header += "Current temperature: " + temperature;
  
    json_str = BuildJSONAttachment(header, forecast, icon_url);

    }
    
  } else {

    int fdIn = open(in_path.c_str(), O_RDONLY );
    if( fdIn != -1 ) {
      struct stat st{0};
      fstat(fdIn, &st);
      auto buffer = new char[st.st_size + 1]{0};
      if( buffer ) {
	int cbRead = read(fdIn, buffer, st.st_size);
	json_str = buffer;
	delete[] buffer;
      }
      close(fdIn);
    }
  }

  if( !fSilent )
    std::cout << json_str << std::endl;
 
  if( request_url != "" )
    SendJSON(request_url, json_str, channel);

  if( cache_path != "" ) {

    int fdCache = open(cache_path.c_str(), O_WRONLY | O_CREAT,
		     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if( fdCache != -1 ) {
      
      write(fdCache, json_str.c_str(), json_str.size());
      close(fdCache);

      syslog(LOG_INFO, "Saving forecast from %s to %s", url.c_str(), cache_path.c_str());

    } else {

      syslog(LOG_ERR, "Error saving forecast from %s to %s", url.c_str(), cache_path.c_str());
    
    }
  }
  
  return 0;
}
