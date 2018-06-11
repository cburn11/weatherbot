#include <string>
#include <memory>
#include <utility>
#include <iostream>

#include <dlfcn.h>

#include "Command.hxx"


template <typename T>
Integration::Integration(T&& lib_path, T&& func_name) :
  m_LibPath{std::forward<T>(lib_path)}, m_FuncName{std::forward<T>(func_name)} {

    dlerror();
    m_hLib = dlopen(m_LibPath.c_str(), RTLD_NOW);
    if( !m_hLib ) {
      std::cout << "Error loading " << m_LibPath << ": " << dlerror() << std::endl;
      return ;
    }

    using init_func = int (*)(void);
    auto need_init = (init_func) dlsym(m_hLib, "NeedInit");
    auto lib_init = (init_func) dlsym(m_hLib, "Init");

    if( need_init && lib_init ) {

      if( need_init() > 0 )
	lib_init();
    }

    m_func = (cmd_func) dlsym(m_hLib, m_FuncName.c_str());
    if( !m_func ) {
      std::cout << "Error resolving " << m_FuncName << " from " << m_LibPath << ": " << dlerror() << std::endl;
      return ;
    }
}

Integration::~Integration() {
 
  if( m_hLib )
    dlclose(m_hLib);
}

std::string Integration::GetValue(const std::string& key) const {

  if( key == "library" )
    return m_LibPath;
  
  else if( key == "func" )
    return m_FuncName;

  return "";
}

template Command::Command(std::string&&, std::string&&, std::string&&, std::string&&);

template <typename T>
Command::Command(T&& command, T&& lib_path, T&& func_name, T&& token) :
  Integration{std::forward<T>(lib_path), std::forward<T>(func_name)} {

  m_command = std::forward<T>(command);
  m_Token = std::forward<T>(token);
  
  std::cout << "Command(" << m_command << ", " << m_Token << ") created" << std::endl;    
     
}

std::string Command::GetValue(const std::string& key) const {

  if( key == "command" )
    return m_command;
  
  else if( key == "token" )
    return m_Token;

  return Integration::GetValue(key);
}

template Action::Action(std::string&&, std::string&&, std::string&&);

template <typename T>
Action::Action(T&& action, T&& lib_path, T&& func_name) :
  Integration{std::forward<T>(lib_path), std::forward<T>(func_name)} {

  m_action = std::forward<T>(action);
  
  std::cout << "Action(" << m_action << ") created" << std::endl;    
     
}

std::string Action::GetValue(const std::string& key) const {

  if( key == "action" )
    return m_action;
  
  return Integration::GetValue(key);
}
