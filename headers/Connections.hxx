#ifndef __CONNECTIONS_HEADERS_DEF_
#define __CONNECTIONS_HEADERS_DEF_

#include <string>
#include <sstream>
#include <utility>
#include <unordered_map>
#include <memory>

#include "Command.hxx"

class Connection;
class Integration;
class Command;
class Action;

class ConnectionHandler { 
  
public:

  template <typename T>
  ConnectionHandler(T&& strConfigPath);
  
  virtual ~ConnectionHandler();

  bool ListenForConnections();

  const Integration * GetCommand(const std::string& command) const;
  const Integration * GetAction(const std::string& action) const;
  
private:

  enum IntegrationType {
    command = 1,
    action
  };
  
  std::string m_fifoPath;
  std::string m_configPath;
  
  int m_fifoFd = -1;
  int m_dummyFd;
  
  std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Integration>>> m_pCommands;
  std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<Integration>>> m_pActions;

  void LoadConfig();
  void OpenPipe();

  void ProcessConnection(std::string&& strPostData);

  const Integration * GetIntegration(const std::string& name, IntegrationType _type) const;
};

class Connection {
 
public:

  Connection(std::string&& strPostData) : m_strPostData{std::move(strPostData)} { };
  virtual ~Connection() { }
  
  virtual void Process(const ConnectionHandler * pConnectionHandler) = 0;

  std::string GetValue(const std::string &strKey) const;

protected:

  std::string m_strPostData;
  std::unordered_map<std::string, std::string> m_kv_pairs;
  
};

class FormUrlEncodedConnection : public Connection {

public:

  FormUrlEncodedConnection(std::string&& strPostData);
  ~FormUrlEncodedConnection();
  
  void Process(const ConnectionHandler * pConnectionHandler) override; 

};

class JSONConnection : public Connection {

public:
  
  JSONConnection(std::string&& strPostData);
  ~JSONConnection();

  void Process(const ConnectionHandler * pConnectionHandler) override;

private:

  std::string m_clientFifoPath;
  int m_clientFifoFd = -1;
  
};


#endif // __CONNECTIONS_HEADERS_DEF_
