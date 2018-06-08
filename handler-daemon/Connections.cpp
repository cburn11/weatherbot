#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <sstream>
#include <thread>

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
    
    Connection * pConnection = new Connection{std::move(strPostData)};

    if( ProcessConnection(pConnection) < 0 )
      return false;
  }
  
  return true;
}

long ConnectionHandler::ProcessConnection(const Connection * pConnection) {

  std::string connection_command = pConnection->GetValue("command");

  // Need to handle exception here
  const Command * pCommand = nullptr;
  try {

    pCommand =  m_pCommands->at(connection_command).get();

  } catch( const std::out_of_range& e) {

    std::cerr << "No command: " << connection_command << " loaded." << std::endl;
    
  }
  
  if( pCommand ) {

    // check tokens
    std::string connection_token  = pConnection->GetValue("token");
    std::string command_token = pCommand->GetValue("token");
    if( connection_token != command_token && command_token != ""  ) {
      std::cout << "Connection token: " << connection_token << " does not match command token." << std::endl;
      return 1;
    }
    
    // Need to dump this off onto another thread

    std::thread t{pCommand->m_func, pConnection};
    t.detach();
    
    //pCommand->m_func(pConnection);
  }

  // Responsibility of the command function.
  // To do: shared_ptr?
  // delete pConnection;
  
  return 1;
}

void ConnectionHandler::LoadConfig() {

  std::cout << "Loading config from: " << m_configPath << std::endl;
  m_pCommands = LoadConfigFromJSON(m_configPath, m_fifoPath);
}

void ConnectionHandler::OpenPipe() {

  if( m_fifoPath == "" )
    return;

  std::cout << "Opening fifo pipe: " << m_fifoPath << std::endl;
  
  if ( mkfifo(m_fifoPath.c_str(), S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP) == -1 && errno != EEXIST ) {
    syslog(LOG_ERR, "mkfifo(%s) returned -1, errno =  %d", m_fifoPath.c_str(), errno);  
  }

  m_fifoFd = open(m_fifoPath.c_str(), O_RDONLY);
  if( m_fifoFd == -1) {
    syslog(LOG_ERR, "open(%s) returned -1, errno =  %d", m_fifoPath.c_str(), errno);
  }

  m_dummyFd = open(m_fifoPath.c_str(), O_WRONLY);
}

Connection::Connection(std::string&& strPostData) :

  m_strPostData{std::move(strPostData)} {

  std::cout << "Creating connection (\"" << m_strPostData  << "\")" << std::endl;
  
  std::istringstream ss{m_strPostData};

  std::string token;
  while( getline(ss, token, '&') ) {

    std::string key, value;
    auto i = token.find_first_of('=');
    key = DecodePercentEncodedString(token.substr(0, i));
    value = DecodePercentEncodedString(token.substr(i + 1, std::string::npos));

    std::cout << key << " = " << value << std::endl;
    
    m_kv_pairs.insert(std::make_pair(key, value));
    
  }
}

Connection::~Connection() {

  std::cout << "Deleting connection" << std::endl;
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

