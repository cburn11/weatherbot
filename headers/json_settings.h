#ifndef __JSON_DEFS_
#define __JSON_DEFS_

#include <string>
#include <unordered_map>
#include <memory>

#include "Command.hxx"

std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Command>>>
  LoadConfigFromJSON(const std::string& path, std::string& fifoPath);

#endif // __JSON_DEFS_
