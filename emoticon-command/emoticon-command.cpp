#include <string.h>

#include <string>
#include <iostream>
#include <thread>
#include <memory>

#include "http.h"
#include "json_settings.h"
#include "Connections.hxx"

PEmote_map_type g_pEmote_Map;

extern "C" void * emoticon_command(const Connection * pConnection) {

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

  std::cout << "Emote func" << std::endl;
  
  if( !g_pEmote_Map )
    return nullptr;
  
  std::string user_name = pConnection->GetValue("user_name");
  std::string text = pConnection->GetValue("text");
  std::string url = pConnection->GetValue("response_url");

  if( text == "list" ) {

    std::string emote_list;
    for( auto pair : *g_pEmote_Map ) {

      emote_list += pair.first + " = " + DecodePercentEncodedString(pair.second) + "\n";
    }

    auto strJSON = BuildEmoticonResponse("", emote_list);

    SendJSON(url, strJSON);

    return nullptr;
  }
  
  try {
    
    std::string encoded_emote = g_pEmote_Map->at(text);  // throws if key not found
      
    std::string decoded_emote = DecodePercentEncodedString(encoded_emote);

    auto strJSON = BuildEmoticonResponse(user_name, decoded_emote);

    SendJSON(url, strJSON);
    
  } catch(std::exception e) {

    std::cout << "Emote " << text << " not loaded." << std::endl;

  }
  
  return nullptr;
}

extern "C" int NeedInit(void * pvoid) {
 
  static int fInit = 1;

  if( fInit == 1 ) {
    fInit = 0;
    return 1;
  }

  return fInit;
}

extern "C" int Init(void * pvoid) {
    
  auto pstrInit = static_cast<std::string *>(pvoid);

  if( pstrInit->length() > 0 )  
    g_pEmote_Map = LoadEmoteFromJSON(*pstrInit);
  
  return 1;
}
