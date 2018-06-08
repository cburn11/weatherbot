#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include "json_settings.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Command>>>
LoadConfigFromJSON(const std::string& path, std::string& fifoPath) {

  if( path == "" ) {
    std::cout << "path null" << std::endl;
    return nullptr;
  }

  auto command_map = std::make_unique<std::unordered_map<std::string, std::unique_ptr<Command>>>();
  
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
      
      auto pcommand = std::make_unique<Command>(std::move(strCommand), std::move(strLibPath),
						std::move(strFuncName), std::move(strToken));
      
      strCommand = command.as_object()["command"].as_string();

      auto cmd_pair = std::make_pair(strCommand, std::move(pcommand));

      command_map->insert(std::move(cmd_pair));      
    }

    fifoPath = config_json["fifo"].as_string();
    
    delete[] json_buffer;
  }
  
  close(fd);
  
  return command_map;
}
