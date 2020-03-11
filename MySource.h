#ifndef __MY_SOURCE_H__
#define __MY_SOURCE_H__

#include <stdint.h>
#include <string>
#include <queue>
#include <random>

#include "DlgSubscriber.h"
#include "DlgMessage.h"

class MySource
{
  std::string          m_server;
  std::string          m_service;
  std::queue<uint32_t> m_data;
  DlgSubscriber*        m_subscriber;
 public:
  MySource(const char* server_name, const char* service_name);
  ~MySource();

  bool     Connect();
  bool     IsConnected();
  bool     HasData();
  uint32_t GetValue();
  void     GenerateData();
};


#endif
