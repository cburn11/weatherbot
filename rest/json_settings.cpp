#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include "json_settings.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

PIntegration_map_type  LoadConfigFromJSON(const std::string& path, std::string& fifoPath, PIntegration_map_type * paction_map) {

  if( path == "" ) {
    std::cout << "path null" << std::endl;
    return nullptr;
  }

  auto command_map = std::make_unique<std::unordered_map<std::string, std::unique_ptr<Integration>>>();
  *paction_map = std::make_unique<std::unordered_map<std::string, std::unique_ptr<Integration>>>();
  
  int fd = open(path.c_str(), O_RDONLY);
  if( fd < 0 ) {
    std::cout << "Error: " <<  std::to_string(errno) << std::endl;
    return nullptr;
  }

  struct stat s{0};
  fstat(fd, &s);
  auto json_buffer = new char[s.st_size + 1]{0};
  if( json_buffer ) {

    int cbRead = read(fd, json_buffer, s.st_size);
    std::cout << "Read: " << json_buffer << std::endl;
    
    json::value config_json = web::json::value::parse(json_buffer);
    auto commands = config_json["commands"].as_array();
    for( auto command : commands ) {
      std::string strCommand = command.as_object()["command"].as_string();
      std::string strLibPath = command.as_object()["library"].as_string();
      std::string strFuncName = command.as_object()["func"].as_string();
      std::string strToken = command.as_object()["token"].as_string();
      
      /*auto pcommand = std::make_unique<Command>(std::move(strCommand), std::move(strLibPath),
	std::move(strFuncName), std::move(strToken));*/
      std::unique_ptr<Integration> pcommand{new Command{std::move(strCommand), std::move(strLibPath),
	    std::move(strFuncName), std::move(strToken)}};
      
      
      strCommand = command.as_object()["command"].as_string();

      auto cmd_pair = std::make_pair(strCommand, std::move(pcommand));

      command_map->insert(std::move(cmd_pair));      
    }

    auto actions = config_json["actions"].as_array();
    for( auto action : actions ) {
      std::string strAction = action.as_object()["action"].as_string();
      std::string strLibPath = action.as_object()["library"].as_string();
      std::string strFuncName = action.as_object()["func"].as_string();

      auto paction = std::make_unique<Action>(std::move(strAction), std::move(strLibPath),
						std::move(strFuncName));
      
      strAction = action.as_object()["action"].as_string();

      auto action_pair = std::make_pair(strAction, std::move(paction));

      (*paction_map)->insert(std::move(action_pair));      
    }    

    fifoPath = config_json["fifo"].as_string();
    
    delete[] json_buffer;
  }
  
  close(fd);
  
  return command_map;
}

void ParseJSONConnection(const std::string& strJSON, std::unordered_map<std::string, std::string>& kv_pairs) {

  std::cout << strJSON << std::endl;
  
  json::value json = web::json::value::parse(strJSON);

  kv_pairs.insert(std::make_pair("user_id", json["user_id"].as_string()));

  auto context = json["context"].as_object();
  for( auto& elem : context ) {
    std::string key = elem.first;
    std::cout << key << " = ";
    std::string value = context[key].as_string();
    std::cout << value << std::endl;
    kv_pairs.insert(std::make_pair(std::move(key), std::move(value)));
  }
}
