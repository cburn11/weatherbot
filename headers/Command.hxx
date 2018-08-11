#ifndef __COMMAND_HEADERS_DEF_
#define __COMMAND_HEADERS_DEF_

#include <string>

#include "Connections.hxx"

class Connection;

class Integration {

public:

  using cmd_func = void * (*)(const Connection * pConn);
  
  template <typename T>
  Integration(T&& lib_path, T&& func_name, T&& init);
  virtual ~Integration();

  cmd_func m_func = nullptr;

  virtual std::string GetValue(const std::string& key) const;
  
private:  
  
  void * m_hLib = nullptr;

  std::string m_LibPath;
  std::string m_FuncName;

  std::string m_init;
};

class Command : public Integration {

public:

  template <typename T>
  Command(T&& command, T&& lib_path, T&& func_name, T&& token, T&& init);

  std::string GetValue(const std::string& key) const override;
  
private:  
  
  std::string m_command;
  std::string m_Token;
  
};

class Action : public Integration {

public:

  template <typename T>
  Action(T&& action, T&& lib_path, T&& func_name);

  std::string GetValue(const std::string& key) const override;
  
private:  
  
  std::string m_action;
};


#endif // __COMMAND_HEADERS_DEF_
