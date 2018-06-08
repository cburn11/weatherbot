#include <string>
#include <memory>
#include <utility>
#include <iostream>

#include <dlfcn.h>

#include "Command.hxx"

template Command::Command(std::string&&, std::string&&, std::string&&, std::string&&);

template <typename T>
Command::Command(T&& command, T&& lib_path, T&& func_name, T&& token) :
  m_command{std::forward<T>(command)}, m_LibPath{std::forward<T>(lib_path)},
  m_FuncName{std::forward<T>(func_name)}, m_Token{std::forward<T>(token)} {

    std::cout << "Command(" << m_command << ", " << m_LibPath <<
      ", " << m_FuncName << ", " << m_Token << ") created" << std::endl;

    
    dlerror();
    m_hLib = dlopen(m_LibPath.c_str(), RTLD_NOW);
    if( !m_hLib ) {
      std::cout << "Error loading " << m_LibPath << ": " << dlerror() << std::endl;
      return ;
    }

    m_func = (cmd_func) dlsym(m_hLib, m_FuncName.c_str());
    if( !m_func ) {
      std::cout << "Error resolving " << m_FuncName << " from " << m_LibPath << ": " << dlerror() << std::endl;
      return ;
    } 
}

Command::~Command() {

  std::cout << m_command << " unloading." << std::endl;
  
  if( m_hLib )
    dlclose(m_hLib);
}

std::string Command::GetValue(const std::string& key) const {

  if( key == "command" )
    return m_command;
  
  else if( key == "library" )
    return m_LibPath;

  else if( key == "func" )
    return m_FuncName;

  else if( key == "token" )
    return m_Token;

  return "";
}
