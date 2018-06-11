#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <string>
#include <iostream>
#include <thread>

#include "http.h"
#include "Connections.hxx"

extern "C" void * weatherbot_command(const Connection * pConnection) {

  /*
   * channel_id
   * channel_name
   * command
   * response_url
   * team_domain
   * team_id
   * text
   * token
   * user_id
   * user_name
   */

  std::cout << "Begin thread" << std::endl;
  
  if( !pConnection )
    return nullptr;
  
  auto response_url = pConnection->GetValue("response_url");
  if( response_url == "" )
    return nullptr;

  //std::cout << "Response url: " << response_url << std::endl;
  
  auto text = pConnection->GetValue("text");
  std::cout << "Text: (" << text << ")" << std::endl;
  if( text != "" ) {

    std::string loc_address;
    double latitude, longitude;

    GetCoords(text, loc_address, latitude, longitude);
    //std::cout << loc_address << ", " << std::to_string(latitude) << ", " << std::to_string(longitude) << std::endl;

    std::string strLatitude = FormatCoordinate(latitude);
    std::string strLongitude = FormatCoordinate(longitude);

    std::string humidity;
    std::thread humidity_thread{[&humidity, &strLatitude, &strLongitude]() {
	humidity = GetHumidity(strLatitude, strLongitude);
      }};
    
    //auto forecast_url = GetForecastUrl(latitude, longitude);
    auto forecast_url = GetForecastUrl(strLatitude, strLongitude);
    //std::cout << forecast_url << std::endl;
    
    std::string forecast;
    std::thread forecast_thread{[&forecast, &forecast_url]() {
	forecast = GetForecast(forecast_url);
	//std::cout << forecast << std::endl;
      }};
     
    std::string temperature, icon_url;
    std::thread temp_thread{[&forecast_url, &temperature, &icon_url]() {
      GetForecastCurrentTempAndIcon(forecast_url, temperature, icon_url);
      //std::cout << temperature << ", " << icon_url << std::endl;
      }};
  
    forecast_thread.join();
    temp_thread.join();
    humidity_thread.join();
    
    std::string header = "#### ";
    header += loc_address + "\n";

    if( temperature.length() > 0 )
      header += "Current temperature: " + temperature + "\n";

    if( humidity.length() > 0 )
      header += "Current humidity: " + humidity + "%\n";

    SendAttachment(response_url, header, forecast, icon_url);  
    
  } else {

    std::string response;
    
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

    SendJSON(response_url, response);
  }

  //std::cout << "End thread" << std::endl;
    
  return nullptr;
}
