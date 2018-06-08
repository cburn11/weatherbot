#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#ifdef _DEBUG

#define DBG_OUT(x) std::cout << x << std::endl;

#else

#define DBG_OUT(x)

#endif // _DEBUG

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams


void GetCoords(std::string loc_str, std::string& loc_address, double& latitude, double& longitude) {

  http_client client{U("https://maps.googleapis.com")};

  uri_builder uri{U("maps/api/geocode/json")};
  uri.append_query(U("address"), U(loc_str.c_str()));                                                                              
  uri.append_query(U("key"), U("AIzaSyDdK6122Wu2rLtg5Wy0JWNgTS6T0YdA7zE"));

  http_request req{methods::GET};
  req.set_request_uri(uri.to_uri());
 
  http_response resp = client.request(req).get();
  json::value geocode_json = resp.extract_json(true).get();
 
  auto result_array = geocode_json["results"].as_array();

  auto first_result_obj = result_array[0].as_object();

  auto formatted_address = first_result_obj["formatted_address"].as_string();
  loc_address = formatted_address;
	
  auto geometry_obj = first_result_obj["geometry"].as_object();
  auto location_obj = geometry_obj["location"].as_object();
  latitude = location_obj["lat"].as_double();
  longitude = location_obj["lng"].as_double();
}

std::string GetForecastUrl(double lat, double lng) {

  std::string forecast_url;

  http_client client{U("https://api.weather.gov/")};

  std::string endpoint = "/points/";

  std::stringstream lat_stream;
  lat_stream << std::fixed << std::setprecision(4) << lat;
  std::string lat_str = lat_stream.str();
  lat_str.erase( lat_str.find_last_not_of('0') + 1, std::string::npos );
  
  std::stringstream lng_stream;
  lng_stream << std::fixed << std::setprecision(4) << lng;
  std::string lng_str = lng_stream.str();
  lng_str.erase( lng_str.find_last_not_of('0') + 1, std::string::npos );
  
  endpoint += lat_str;
  endpoint += ",";
  endpoint += lng_str;
  
  uri_builder uri{endpoint.c_str()};
  http_request req{methods::GET};
  req.set_request_uri(uri.to_uri());
  req.headers().add(U("Accept"), "application/json;version=1");

  DBG_OUT(req.to_string());
  
  http_response resp = client.request(req).get();
  json::value point_json = resp.extract_json(true).get();
      
  auto properties_obj = point_json.as_object()["properties"].as_object();

  forecast_url = properties_obj["forecast"].as_string();
	
  return forecast_url;
 
}

std::string GetForecast(const std::string& forecast_url) {

  std::string forecast;

  http_client client{U(forecast_url.c_str())};
  http_request req{methods::GET};
  
  req.headers().add(U("Accept"), "application/json;version=1");

  http_response resp = client.request(req).get();

  auto status_code = resp.status_code();
  
  if( status_code != 200 ) {

    forecast = forecast_url + " returned " + std::to_string(status_code);
    
  } else {
    
    json::value forecast_json = resp.extract_json(true).get();

    auto properties = forecast_json["properties"];
    auto periods = properties["periods"];

    int size = periods.as_array().size();
    int limit = std::min(4, size);

    for( int i = 0; i < limit; ++i ) {
	  
      auto elem = periods.as_array()[i].as_object();

      std::string name = elem["name"].as_string();
      std::string detailedForecast = elem["detailedForecast"].as_string();

      forecast += "#### ";
      forecast += name;
		  
      forecast += "\n";

      forecast += detailedForecast;
      forecast += "\n\n";
    }
  }
  
  return forecast;
}

bool GetForecastCurrentTempAndIcon(const std::string& forecast_url,
				   std::string& temp, std::string& icon_url) {

  std::string lTemp, lIcon_url;
  bool ret = false;

  temp = "";
  icon_url = "";
  
  http_client client{U((forecast_url + "/hourly").c_str())};
  http_request req{methods::GET};
  
  req.headers().add(U("Accept"), "application/json;version=1");

  http_response resp = client.request(req).get();
  
  auto status_code = resp.status_code();
  
  if( status_code == 200 ) {
    
    json::value forecast_json = resp.extract_json(true).get();

    auto properties = forecast_json["properties"];
    auto periods = properties["periods"];
    auto period = periods.at(0);	   

    lTemp = std::to_string(period["temperature"].as_integer()) +
      " " + period["temperatureUnit"].as_string();
	
    lIcon_url = period["icon"].as_string();

    ret = true;
  }

  temp = lTemp;
  icon_url = lIcon_url;
  
  return ret;
}

std::string BuildJSONAttachment(const std::string& header, const std::string& forecast,
				const std::string& icon) {

  json::value attachment1 = json::value::object();
  attachment1["text"] = json::value::string(header);
  attachment1["thumb_url"] = json::value::string(icon);

  json::value attachment2 = json::value::object();
  attachment2["text"] = json::value::string(forecast);
  
  json::value attachments_array = json::value::array(2);
  attachments_array[0] = attachment1;
  attachments_array[1] = attachment2;
  
  json::value payload_json = json::value::object();
  payload_json["attachments"] = attachments_array;
  payload_json["response_type"] = json::value::string("in_channel");

  payload_json["username"] = json::value::string("Weather Bot");

  return payload_json.serialize();
  
}

std::string DecodePercentEncodedString(const std::string& strEncoded) {

  return web::uri::decode(strEncoded);
}

void SendJSON(const std::string& url, const std::string& json,
	      const std::string& channel = std::string{""}) {

  /*
  auto url_decoded = web::uri::decode(url_encoded);
    DecodePercentEncodedString(url);
    */
  
  http_client client{url};
  
  http_request req{methods::POST};

  auto payload_json = web::json::value::parse(json);
  if( channel != "" )
    payload_json["channel"] = json::value::string(channel);
  
  req.set_body(payload_json);

  syslog(LOG_INFO, "Response url: %s", url.c_str());
  syslog(LOG_INFO, "Request: %s", req.to_string().c_str());
  
  client.request(req).wait();
}

void SendResponse(const std::string& url, const std::string& text) {
  
  json::value text_json = json::value::object();
  text_json["text"] = json::value::string(U(text.c_str()));

  SendJSON(url, text_json.serialize());
  
}

void SendAttachment(const std::string& url, const std::string& header,
		    const std::string& forecast, const std::string& icon) {

  auto json_str = BuildJSONAttachment(header, forecast, icon);
  
  SendJSON(url, json_str);

}
