#pragma once

#include "DpaTransaction.h"
#include "UdpChannel.h"
#include "IMessaging.h"
#include "TaskQueue.h"
#include "WatchDog.h"
#include <string>
#include <atomic>

typedef std::basic_string<unsigned char> ustring;

class UdpMessaging;
class MessagingController;

class UdpMessagingTransaction : public DpaTransaction
{
public:
  UdpMessagingTransaction(UdpMessaging* udpMessaging);
  virtual ~UdpMessagingTransaction();
  virtual const DpaMessage& getMessage() const;
  virtual void processConfirmationMessage(const DpaMessage& confirmation);
  virtual void processResponseMessage(const DpaMessage& response);
  virtual void processFinish(DpaRequest::DpaRequestStatus status);
  void setMessage(ustring message);
private:
  DpaMessage m_message;
  UdpMessaging* m_udpMessaging;
  bool m_success;
};

class UdpMessaging : public IMessaging
{
public:
  UdpMessaging() = delete;
  UdpMessaging(MessagingController* messagingController);
  virtual ~UdpMessaging();

  virtual void setDaemon(IDaemon* daemon);
  virtual void start();
  virtual void stop();

  void registerMessageHandler(MessageHandlerFunc hndl) override {}
  void unregisterMessageHandler() override {}
  void sendMessage(const ustring& msg) override {}
  const std::string& getName() const override { return m_name; }

  void sendDpaMessageToUdp(const DpaMessage&  dpaMessage);

  void getGwIdent(ustring& message);
  void getGwStatus(ustring& message);

  int handleMessageFromUdp(const ustring& udpMessage);
  void encodeMessageUdp(ustring& udpMessage, const ustring& message = ustring());
  void decodeMessageUdp(const ustring& udpMessage, ustring& message);

private:
  void setExclusiveAccess();
  void resetExclusiveAccess();

  std::atomic_bool m_exclusiveAccess = false;
  //std::mutex m_watcherMutex;
  //std::mutex m_conditionVariableMutex;
  //std::condition_variable m_conditionVariable;
  //std::thread m_watcherThread;
  //void exclusiveAccessWatcher();
  //std::atomic_bool m_watcherRunning;
  WatchDog<std::function<void()>> m_watchDog;

  MessagingController *m_messagingController;
  UdpMessagingTransaction* m_transaction;

  UdpChannel *m_udpChannel;
  TaskQueue<ustring> *m_toUdpMessageQueue;

  //configuration
  std::string m_name;
  unsigned long int m_remotePort;
  unsigned long int m_localPort;
};
