#ifndef __T_MY_RINT_H__
#define __T_MY_RINT_H__

#include <vector>
#include <queue>
#include <pthread.h>
#include <semaphore.h>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TTimer.h>
#include <TThread.h>

typedef void* (*ThreadRoutineType)(void*);

class CsThread
{
  bool isError;
  pthread_t m_thread;
 public:
  CsThread(ThreadRoutineType routine, void* param = NULL);
  virtual ~CsThread() { Cancel(); };
 public:
  void Cancel() { pthread_cancel(m_thread); }
  bool IsError() const { return isError; }
};

const int SM_LOCKED   = 0;
const int SM_UNLOCKED = 1;

class TSemaphore
{
  sem_t m_semaphore;
 public:
  TSemaphore(int initValue = SM_LOCKED);
  virtual ~TSemaphore();
 public:
  int Lock();
  int Unlock();
};

struct MonitorData
{
  uint32_t data_type;
  uint32_t data_value;
};

class MonitorBuffer
{
  std::queue<MonitorData*> buffer;
  TSemaphore semaphore;
public:
  MonitorBuffer() : semaphore(SM_UNLOCKED) {};
  ~MonitorBuffer() { Clear(); }

  void Clear();
  void PutData(MonitorData*);
  MonitorData* GetData();
  bool IsEmpty() const { return buffer.empty(); }
};

class MySource;

class TMyRint : public TApplication
{
  CsThread*              m_pMonitorThread;
  bool                   m_isRunning;
  std::vector<TCanvas*>  m_canvas;
  std::vector<TH1*>      m_histos;
  TSemaphore*            m_pSemaphore;
  TTimer*                m_pUpdateTimer;
  bool                   m_isModified;
  MonitorBuffer          m_buffer;
  static TMyRint*        instance;
  std::vector<MySource*> m_sources;
public:
  TMyRint(char* appClassName, int* argc, char** argv);
  virtual ~TMyRint() {};
  virtual void Terminate(int status);
  void ResetHistograms();
  void UpdateHistograms();
  void FillHistograms();
  TSemaphore* GetSemaphore() { return m_pSemaphore; }
  static TMyRint* Instance() { return instance; }
 private:
  static void* MonitorFunc(void*);
  int  GetEvent();
  void CreateHistograms();
  bool CreateSources(int* argc, char** argv);
};

#endif
