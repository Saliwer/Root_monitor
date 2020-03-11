#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include <string>
#include "MySource.h"

#include "TMyRint.h"

CsThread::CsThread(ThreadRoutineType routine, void* param)
{
  isError = false;
  if(pthread_create(&m_thread,NULL,routine, param) != 0)
    {
      fprintf(stderr,"Thread cannot be created\n");
    }
}

TSemaphore::TSemaphore(int initValue)
{
  if(sem_init(&m_semaphore,0,initValue) != 0)
    {
      fprintf(stderr,"Semaphore initialization error\n");
    }
}

TSemaphore::~TSemaphore()
{
  Unlock();
  if(sem_destroy(&m_semaphore) != 0)
  {
    fprintf(stderr,"Semaphore cannot be destroyed");
  }
}

int TSemaphore::Lock()
{
  return sem_wait(&m_semaphore);
}

int TSemaphore::Unlock()
{
  int errCode = sem_post(&m_semaphore);
  if(errCode)
    fprintf(stderr,"Semaphore posting error\n");
  return (int)errCode;
}

void MonitorBuffer::Clear()
{
  semaphore.Lock();
  while(!buffer.empty())
    {
      MonitorData* d = buffer.front();
      delete d;
      buffer.pop();
    }
  semaphore.Unlock();
}

MonitorData* MonitorBuffer::GetData()
{
  semaphore.Lock();
  MonitorData* d = buffer.front();
  buffer.pop();
  semaphore.Unlock();
  return d;
}

void MonitorBuffer::PutData(MonitorData* d)
{
  semaphore.Lock();
  buffer.push(d);
  semaphore.Unlock();
}

//////*************************** TMyRint class ********************///////

TMyRint* TMyRint::instance = NULL;

TMyRint::TMyRint(char* appClassName, int* argc, char** argv) : 
TApplication(appClassName,argc,argv)
{
  assert(instance == NULL);
  m_isModified = true;
  instance = this;
  m_pSemaphore = new TSemaphore(SM_UNLOCKED);
  CreateSources(argc,argv);
  CreateHistograms();
  m_pUpdateTimer = new TTimer("UpdateMonitorWindows()",10);
  m_pUpdateTimer->Start();
  m_pMonitorThread = new CsThread(MonitorFunc,this);
}

void TMyRint::Terminate(int status)
{
  // Terminate the application. Reset the terminal to sane mode and call
  // the logoff macro defined via Rint.Logoff environment variable.
  m_isRunning = false;
  usleep(10000);
  delete m_pMonitorThread;
  delete m_pSemaphore;
  for(unsigned i = 0; i < m_sources.size(); i++)
    delete m_sources[i];
  TApplication::Terminate(status);
}

void TMyRint::CreateHistograms()
{
  TCanvas* c  = new TCanvas("root_test","ROOT test", 0, 0, 800,600);
  
  m_canvas.push_back(c);

  TH1F* h = new TH1F("h0","Normal Distribution",1000,0,1000);
  m_histos.push_back(h);
  h = new TH1F("h1","Gaussian Distribution",1000,0,1000);
  m_histos.push_back(h);
  h = new TH1F("h2","Exponential Distribution", 1000, 0, 1000);
  m_histos.push_back(h);
  h = new TH1F("h3","Total Distribution", 1000, 0, 1000);
  m_histos.push_back(h);
  c->Divide(2,2);
  c->cd(1);
  m_histos[0]->Draw();
  c->cd(2);
  m_histos[1]->Draw();
  c->cd(3);
  m_histos[2]->Draw();
  c->cd(4);
  m_histos[3]->Draw();

  UpdateHistograms();
}

void TMyRint::ResetHistograms()
{
  TThread::Lock();
  for(unsigned i = 0; i < m_histos.size(); i++)
    {
      m_histos[i]->Reset();
    }
  TThread::UnLock();
}

void TMyRint::UpdateHistograms()
{
  if(!m_isModified)
    return;
  TThread::Lock();
  for(unsigned i = 0; i < m_canvas.size(); i++)
    {
      m_canvas[i]->Paint();
      m_canvas[i]->Update();
    }
  m_isModified = false;
  TThread::UnLock();
}

void TMyRint::FillHistograms()
{
  if(m_buffer.IsEmpty())
    return;
  m_pSemaphore->Lock();
  m_isModified = true;

  while(!m_buffer.IsEmpty())
    {
      MonitorData* d = m_buffer.GetData();
      if(d->data_type == 0)
	m_histos[0]->Fill(d->data_value);
      if(d->data_type == 1)
	m_histos[1]->Fill(d->data_value);
      if(d->data_type == 2)
	m_histos[2]->Fill(d->data_value);
      m_histos[3]->Fill(d->data_value);
    }
  m_pSemaphore->Unlock();
}

void* TMyRint::MonitorFunc(void* p)
{
  TMyRint* rint = (TMyRint*)p;
  rint->m_isRunning = true;
  while(rint->m_isRunning)
    {
      int  status = rint->GetEvent();
      if(status > 0)
	{
	  rint->FillHistograms();
	}
      else if(status == 0)
	usleep(1000);
      if(status < 0)
	break;
      // for debugging:
      // for(unsigned i = 0; i < rint->m_sources.size(); i++)
      // 	rint->m_sources[i]->GenerateData();
      // usleep(100000);
    }
  printf("Monitoring thread is stoped.\n");
  return NULL;
}

bool TMyRint::CreateSources(int* argc, char** argv)
{
  if(*argc < 2)
    return false;
  std::string server_name = argv[1];

  MySource* norm = new MySource(server_name.c_str(),"NORM");
  m_sources.push_back(norm);
  MySource* gaus = new MySource(server_name.c_str(),"GAUS");
  m_sources.push_back(gaus);
  MySource* exp = new MySource(server_name.c_str(),"EXP");
  m_sources.push_back(exp);


  for(int i = 0; i < 3; i++)
    {
      if(!m_sources[i]->Connect())
	return false;
    }

  return true;
}

int TMyRint::GetEvent()
{
  int n_data = 0;
  for(unsigned i = 0; i < m_sources.size(); i++)
    {
      if(m_sources[i]->IsConnected())
	{
	  while(m_sources[i]->HasData())
	    {
	      n_data++;
	      MonitorData* d = new MonitorData;
	      d->data_type = i;
	      d->data_value = m_sources[i]->GetValue();
	      m_buffer.PutData(d);
	    }
	}
      else
	return (-1-i);
    }
  return n_data;
}
