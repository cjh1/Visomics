#ifndef CONNECTIONLISTENER_H_
#define CONNECTIONLISTENER_H_

#include <string>
#include <list>

namespace ProtoCall {
namespace Runtime {
class vtkCommunicatorChannel;
}
}

class vtkServerSocket;

class voAnalysisServer
{
public:
  voAnalysisServer();
  virtual ~voAnalysisServer();

  void listen(int port);

private:
  vtkServerSocket *m_socket;
  std::list<ProtoCall::Runtime::vtkCommunicatorChannel *> m_clientChannels;

  void processConnectionEvents();
  void accept();

};

#endif /* CONNECTIONLISTENER_H_ */
