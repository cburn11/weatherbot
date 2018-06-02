#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <syslog.h>

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
  uri.append_query(U("key"), U("--XXXXX--"));

  http_request req{methods::GET};
  req.set_request_uri(uri.to_uri());
 
  client.request(req)
  .then([](http_response resp) {
      
      return resp.extract_json();
	     
	   })
    .then([&loc_address, &latitude, &longitude](json::value geocode_json) {

	auto result_array = geocode_json["results"].as_array();

	auto first_result_obj = result_array[0].as_object();

	auto formatted_address = first_result_obj["formatted_address"].as_string();
	loc_address = formatted_address;
	
	auto geometry_obj = first_result_obj["geometry"].as_object();
	auto location_obj = geometry_obj["location"].as_object();
	latitude = location_obj["lat"].as_double();
	longitude = location_obj["lng"].as_double();
	
      })
    .wait(); 
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
  
  client.request(req)
    .then([](http_response resp) {

	DBG_OUT(resp.to_string());
	
	return resp.extract_json(true);
      })
    .then([&forecast_url](json::value point_json) {

	auto properties_obj = point_json.as_object()["properties"].as_object();

	forecast_url = properties_obj["forecast"].as_string();
	
      })
    .wait();
  
  return forecast_url;
  
}

std::string GetForecast(const std::string& forecast_url) {

  std::string forecast;

  http_client client{U(forecast_url.c_str())};
  http_request req{methods::GET};
  
  req.headers().add(U("Accept"), "application/json;version=1");

  client.request(req)
    .then([](http_response resp) {

	return resp.extract_json(true);

      })
    .then([&forecast](json::value forecast_json) {

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
	
      })
    .wait();
  
  return forecast;
}

void SendResponse(const std::string& url, const std::string& text) {

  auto url_encoded = U(url.c_str());
  auto url_decoded = web::uri::decode(url_encoded);
  
  http_client client{url_decoded};
  
  http_request req{methods::POST};
    
  json::value text_json = json::value::object();
  text_json["text"] = json::value::string(U(text.c_str()));

  req.set_body(text_json);

  syslog(LOG_INFO, "Response url: %s", url_decoded.c_str());
  syslog(LOG_INFO, "Request: %s", req.to_string().c_str());
  
  client.request(req).wait();
}
