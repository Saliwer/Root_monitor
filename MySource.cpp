#include <stdlib.h>
#include <random>
#include "MySource.h"
#include "DlgSubscriber.h"
#include "Debug.h"
#include <iostream>
MySource::MySource(const char* server_name, const char* service_name)
{
  m_server = server_name;
  m_service = service_name;
  char subName[256];
  sprintf(subName, "%s_Subscriber", service_name);
  m_subscriber = new DlgSubscriber(std::string(subName), m_service, m_server);
  m_subscriber->Connect();
}

MySource::~MySource()
{
  delete m_subscriber;
}

bool     MySource::Connect()
{
  return m_subscriber->Subscribe();
}

bool     MySource::IsConnected()
{
  return m_subscriber->IsConnected();
}

bool     MySource::HasData()
{
  return m_subscriber->HasData();
}

uint32_t MySource::GetValue()
{
  DlgMessage *msg = nullptr;
  uint32_t value = 0;
  if (!m_subscriber->GetMessage(msg))
    {
      Print(DBG_LEVEL_ERROR, "MySource::GetValue(): Couldn't get DlgMessage.\n");
    }

  void *buf = nullptr;
  size_t size = 0;
  if (msg->GetMessageBuffer(buf, size))
    {
      buf = operator new(size);
      if (msg->GetMessageBuffer(buf, size))
	{
	  value = *static_cast<uint32_t*>(buf);
	}
      else
	{
	  Print(DBG_LEVEL_ERROR, "MySource::GetValue(): Couldn't get binary data.\n");
	}
      operator delete(buf);
    }
  else
    {
      Print(DBG_LEVEL_ERROR, "MySource::GetValue(): Couldn't get binary buffer.\n");
    }
  
  delete msg;
  return value;
}

/////////////////// Debug functions: 

void MySource::GenerateData()
{
  uint32_t value = 0;
  if(m_service == "NORM")
    {
      value = rand()%1000;
    }
  else if(m_service == "EXP")
    {
      std::random_device rd;
      std::mt19937 gen(rd());
      
      std::exponential_distribution<> dist(1);
      value = dist(gen) * 100;
    }
  else if(m_service == "GAUS")
    {    
      std::random_device rd;
      std::mt19937 gen(rd());
      
      std::normal_distribution<> dist(500, 100);
      value = dist(gen);
    }
  m_data.push(value);
}
