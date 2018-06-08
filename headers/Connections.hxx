#ifndef __CONNECTIONS_HEADERS_DEF_
#define __CONNECTIONS_HEADERS_DEF_

#include <string>
#include <utility>
#include <unordered_map>
#include <memory>

#include "Command.hxx"

class Connection;
class Command;

class ConnectionHandler { 
  
public:

  template <typename T>
  ConnectionHandler(T&& strConfigPath);
  
  virtual ~ConnectionHandler();

  bool ListenForConnections();

private:

  std::string m_fifoPath;
  std::string m_configPath;
  
  int m_fifoFd = -1;
  int m_dummyFd;
  
  std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Command>>> m_pCommands;

  void LoadConfig();
  void OpenPipe();

  long ProcessConnection(const Connection * pConnection);
};

class Connection {

public:

  Connection(std::string&& strPostData);
  virtual ~Connection();
  
  std::string GetValue(const std::string& strKey) const;

  
private:

  std::string m_strPostData;
  
  std::unordered_map<std::string, std::string> m_kv_pairs;
};


#endif // __CONNECTIONS_HEADERS_DEF_
