#include <string.h>

#include <string>
#include <iostream>
#include <thread>

#include "http.h"
#include "Connections.hxx"

extern "C" void * need_command(const Connection * pConnection) {

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

  std::cout << "Begin need-command thread" << std::endl;
  
  if( !pConnection )
    return nullptr;

  auto strAttachment = BuildNeedAttachment(pConnection->GetValue("text"));

  std::cout << strAttachment << std::endl;

  SendJSON(pConnection->GetValue("response_url"), strAttachment,
	   pConnection->GetValue("channel_name"));
  
  return nullptr;
}

extern "C" void * got_it_func(const Connection * pConnection) {

  std::string response = "{\"update\":{\"message\":\"~~" + pConnection->GetValue("text") + "~~\"}}";
  char * szResponse = new char[response.length() + 1]{0};
  if( szResponse ) {
    strcpy(szResponse, response.c_str());
    return szResponse;
  }
  
  return nullptr;
}

extern "C" void * delete_it_func(const Connection * pConnection) {

  std::string response = "{\"update\":{\"message\":\"\"}}";
  char * szResponse = new char[response.length() + 1]{0};
  if( szResponse ) {
    strcpy(szResponse, response.c_str());
    return szResponse;
  }
  
  return nullptr;
}
