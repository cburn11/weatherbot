#ifndef __JSON_DEFS_
#define __JSON_DEFS_

#include <string>
#include <unordered_map>
#include <memory>

#include "Command.hxx"

//using PCommand_map_type = std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Command>>>;
//using PAction_map_type = std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Action>>>;
using PIntegration_map_type = std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Integration>>>;

PIntegration_map_type  LoadConfigFromJSON(const std::string& path, std::string& fifoPath, PIntegration_map_type * paction_map);

void ParseJSONConnection(const std::string& strJSON, std::unordered_map<std::string, std::string>& kv_pairs);

#endif // __JSON_DEFS_
