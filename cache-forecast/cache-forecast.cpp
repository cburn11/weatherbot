#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include <unistd.h>
#include <errno.h>

#include <time.h>

#include <string>
#include <iostream>

#include "rest.h"

int main(int argc, char * argv[]) {

  std::string default_url = "https://api.weather.gov/gridpoints/VEF/122,97/forecast";
  std::string url;
  std::string location;
  std::string cache_path;
  bool fSilent = false;
  
  extern char * optarg;
  extern int optind, opterr, optopt;
  const char * szOpt = ":u:l:o:s"; 
  int opt;
  while( (opt = getopt(argc, argv, szOpt)) > 0 ) {

    switch(opt) {

    case 'u':
      url = optarg;
      break;

    case 'l':
      location = optarg;

    case 'o':
      cache_path = optarg;
      break;

    case 's':
      fSilent = true;
    }
  }

  std::string loc_address{"Las Vegas, NV"};
  
  if( location != "" && url == "" ) {

    double latitude, longitude;

    GetCoords(location, loc_address, latitude, longitude);

    url = GetForecastUrl(latitude, longitude);
    
  } else if( location == "" && url == "" ) {

    url = default_url;
  }
 
  std::string forecast = GetForecast(url);

  if( !fSilent ) {
    std::cout << loc_address << std::endl;
    std::cout << forecast << std::endl;
  }

  if( cache_path == "" ) {

    time_t t = time(nullptr);
    struct tm * pnow = gmtime(&t);
    
    cache_path = "/tmp/forecast-" + std::to_string(pnow->tm_year + 1900) +
      std::to_string(pnow->tm_mon + 1) + std::to_string(pnow->tm_mday) + "-" +
      std::to_string(pnow->tm_hour) + std::to_string(pnow->tm_min) + std::to_string(pnow->tm_sec);
  }
  
  int fdCache = open(cache_path.c_str(), O_WRONLY | O_CREAT,
		     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if( fdCache != -1 ) {
    std::string output = loc_address + "\n" + forecast;
    write(fdCache, output.c_str(), output.size());
    close(fdCache);

    syslog(LOG_INFO, "Saving forecast from %s to %s", url.c_str(), cache_path.c_str());

  } else {

    syslog(LOG_ERR, "Error saving forecast from %s to %s", url.c_str(), cache_path.c_str());
    
  }
  
  return 0;
}
