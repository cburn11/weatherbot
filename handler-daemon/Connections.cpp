#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <algorithm>

#include "utils.h"
#include "http.h"
#include "json_settings.h"

#include "Connections.hxx"

template ConnectionHandler::ConnectionHandler(std::string&&);

template <typename T>
ConnectionHandler::ConnectionHandler(T&& strConfigPath) :
    m_configPath{std::forward<T>(strConfigPath)} {

  std::cout << "Creating ConnectionHandler." << std::endl;
  
  LoadConfig();

  OpenPipe();
}

ConnectionHandler::~ConnectionHandler() {

  std::cout << "Stopping" << std::endl;
  
  if( m_fifoFd > 0 ) close(m_fifoFd);

  if( m_dummyFd > 0 ) close(m_dummyFd);
}

bool ConnectionHandler::ListenForConnections() {

  if( m_fifoFd < 0 )
    return false;
  
  std::cout << "Listening for connections." << std::endl;
  
  char * szMsg;
  int cbRead;
  if( (cbRead = read_fifo_msg(m_fifoFd, &szMsg)) > 0 ) {

    std::string strPostData = szMsg;
    free(szMsg);

    if( strPostData == "STOP" )
      return false;
    
    ProcessConnection(std::move(strPostData));	
  }
  
  return true;
}

const Integration * ConnectionHandler::GetCommand(const std::string& command) const {

  auto pintegration = GetIntegration(command, ConnectionHandler::IntegrationType::command);
  if( !pintegration )
    std::cout << "Command: " << command << " not loaded." << std::endl;

  return pintegration;
}

const Integration * ConnectionHandler::GetAction(const std::string& action) const {
  
  auto pintegration =  GetIntegration(action, ConnectionHandler::IntegrationType::action);
  if( !pintegration )
    std::cout << "Action: " << action << " not loaded." << std::endl;

  return pintegration;
}

const Integration * ConnectionHandler::GetIntegration(const std::string& name, IntegrationType _type) const {

  const auto * pMap = _type == ConnectionHandler::IntegrationType::command ?
    m_pCommands.get() : m_pActions.get();

  try {
    
    auto& pIntegration =pMap->at(name);
    
    return pIntegration.get();
    
  } catch( std::range_error e) {
    
    return nullptr;
    
  } 
}


void ConnectionHandler::ProcessConnection(std::string&& strPostData) {

  const ConnectionHandler * pHandler = this;
  
  std::thread t{[&strPostData, &pHandler]() {
  
      auto substr_begin = strPostData.find_first_of('/');
      auto substr_end = strPostData.find_first_of('&', substr_begin);
      std::string content_type =  strPostData.substr(substr_begin + 1, substr_end - substr_begin - 1);
      std::string content = strPostData.substr(substr_end + 1, std::string::npos);
      
      std::unique_ptr<Connection> pConnection;
  
      if( content_type == "x-www-form-urlencoded" ) {

	pConnection =  std::make_unique<FormUrlEncodedConnection>(std::move(content));
    
      } else if( content_type == "json" ) {

	pConnection = std::make_unique<JSONConnection>(std::move(content));
	
      }

      if( pConnection ) {
	pConnection->Process(pHandler);
      }
      
    }};

  t.detach();    
}

void ConnectionHandler::LoadConfig() {

  std::cout << "Loading config from: " << m_configPath << std::endl;
  m_pCommands = LoadConfigFromJSON(m_configPath, m_fifoPath, &m_pActions);
}

void ConnectionHandler::OpenPipe() {

  if( m_fifoPath == "" )
    return;

  std::cout << "Opening fifo pipe: " << m_fifoPath << std::endl;
  
  if ( mkfifo(m_fifoPath.c_str(), S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP) == -1 && errno != EEXIST ) {
    syslog(LOG_ERR, "mkfifo(%s) returned -1, errno =  %d", m_fifoPath.c_str(), errno);  
  }

  m_fifoFd = open(m_fifoPath.c_str(), O_RDONLY | O_NONBLOCK);
  if( m_fifoFd == -1) {
    syslog(LOG_ERR, "open(%s) returned -1, errno =  %d", m_fifoPath.c_str(), errno);
  }

  int flags;
  flags = fcntl(m_fifoFd, F_GETFL); /* Fetch open files status flags */
  flags = flags & ~O_NONBLOCK; /* Enable O_NONBLOCK bit */
  fcntl(m_fifoFd, F_SETFL, flags); /* Update open files status flags */
  
  m_dummyFd = open(m_fifoPath.c_str(), O_WRONLY);
}

std::string Connection::GetValue(const std::string &strKey) const {

  if( strKey == "" )
    return strKey;

  try {  

    return m_kv_pairs.at(strKey);

  } catch( const std::out_of_range& e ) {

    std::cerr << "Connection does not contain key: " << strKey << std::endl;

    return "";
  }
}


FormUrlEncodedConnection::FormUrlEncodedConnection(std::string&& strPostData) :
  Connection(std::move(strPostData)) {
 
  std::istringstream ss{m_strPostData};
  std::string token;

  while( getline(ss, token, '&') ) {
    std::cout << token << std::endl;
    std::string key, value;
    auto i = token.find_first_of('=');
    key = DecodePercentEncodedString(token.substr(0, i));
    value = DecodePercentEncodedString(token.substr(i + 1, std::string::npos));
    std::replace(value.begin(), value.end(), '+', ' ');
    
    m_kv_pairs.insert(std::make_pair(key, value));
  }
}

FormUrlEncodedConnection::~FormUrlEncodedConnection() {

  std::cout << "Deleting FormUrlencodedconnectionn" << std::endl;
}


void FormUrlEncodedConnection::Process(const ConnectionHandler * pConnectionHandler) {

  std::string command = GetValue("command");
  if( command == "" )
    return;
  
  const Integration * pCommand = pConnectionHandler->GetCommand(command);
  
  if( pCommand ) {

    // check tokens
    std::string connection_token  = GetValue("token");
    std::string command_token = GetValue("token");
    if( connection_token != command_token && command_token != ""  ) {
      std::cout << "Connection token: " << connection_token << " does not match command token." << std::endl;
      return;
    }

    pCommand->m_func(this);
  }
}

JSONConnection::JSONConnection(std::string&& strPostData) :
  Connection(std::move(strPostData)) {

  auto amp = m_strPostData.find_first_of('&');
  m_clientFifoPath = "/var/spool/cgi/client_fifo_" + m_strPostData.substr(0, amp);

  m_clientFifoFd = open(m_clientFifoPath.c_str(), O_WRONLY);
  if( m_clientFifoFd == -1 && errno != EEXIST ) {
    std::cout << "Error (" << std::to_string(errno) << ") opening " << m_clientFifoPath << std::endl;
  }

  std::string json = m_strPostData.substr(amp + 1, std::string::npos);

  ParseJSONConnection(json, m_kv_pairs);
  
}

JSONConnection::~JSONConnection() {

  if( m_clientFifoFd > 0 )
    close(m_clientFifoFd);
  
  std::cout << "Deleting JSONConnectionn" << std::endl;
}


void JSONConnection::Process(const ConnectionHandler * pConnectionHandler) {

  std::string action = GetValue("action");
  if( action == "" )
    return;
  
  const Integration * pAction = pConnectionHandler->GetAction(action);
  
  if( pAction ) {

    const char * szBody = (char *) pAction->m_func(this);
    if( szBody ) {
      write_fifo_msg(m_clientFifoFd, szBody);
      delete[] szBody;
    }

    szBody = "{\"response-type\":\"ephemeral\",\"text\":\"Error\"}";

    write_fifo_msg(m_clientFifoFd, szBody);
  }
}
