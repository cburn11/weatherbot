#ifndef __COMMAND_HEADERS_DEF_
#define __COMMAND_HEADERS_DEF_

#include <string>

#include "Connections.hxx"

class Connection;

class Command {

public:

  using cmd_func = void * (*)(const Connection * pConn);
  
  template <typename T>
  Command(T&& command, T&& lib_path, T&& func_name, T&& token);
  virtual ~Command();

  cmd_func m_func = nullptr;

  std::string GetValue(const std::string& key) const;
  
private:  
  
  void * m_hLib = nullptr;

  std::string m_command;
  std::string m_LibPath;
  std::string m_FuncName;
  std::string m_Token;
  
};

#endif // __COMMAND_HEADERS_DEF_
