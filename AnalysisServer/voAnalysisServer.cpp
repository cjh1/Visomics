#include "voAnalysisServer.h"

// ProtoCall headers
#include "RemoteAnalysisService_Dispatcher.pb.h"
#include <google/protobuf/stubs/common.h>
#include <protocall/runtime/servicemanager.h>
#include <protocall/runtime/vtkcommunicatorchannel.h>

// Service implementation
#include "voRemoteAnalysisService.h"

// VTK headers
#include <vtkSocketController.h>
#include <vtkClientSocket.h>
#include <vtkServerSocket.h>
#include <vtkSocketCommunicator.h>

#include <algorithm>
#include <vector>
#include <list>
#include <iostream>

using std::vector;
using std::list;
using ProtoCall::Runtime::vtkCommunicatorChannel;
using ProtoCall::Runtime::RpcChannel;

voAnalysisServer::voAnalysisServer()
{
 // It's essential to initialize the socket controller to initialize sockets on
 // Windows.
 vtkSocketController* controller =  vtkSocketController::New();
 controller->Initialize();
 controller->Delete();
}

voAnalysisServer::~voAnalysisServer()
{
// TODO Auto-generated destructor stub
}

void voAnalysisServer::listen(int port)
{
  m_socket =  vtkServerSocket::New();
  if (m_socket->CreateServer(port) != 0)
  {
     std::cerr << "Failed to set up server socket.\n";
     m_socket->Delete();
     return;
  }

  while (true)
    processConnectionEvents();
}

void voAnalysisServer::accept()
{
  vtkCommunicatorChannel *channel = NULL;

  while (!channel) {
    vtkClientSocket* client_socket = NULL;
    client_socket = m_socket->WaitForConnection(100);

    if (!client_socket)
      return;

    vtkSocketController* controller = vtkSocketController::New();
    vtkSocketCommunicator* comm = vtkSocketCommunicator::SafeDownCast(
        controller->GetCommunicator());
    // Silent unecessary error reporting
    comm->SetReportErrors(0);
    comm->SetSocket(client_socket);
    client_socket->FastDelete();
    channel = new vtkCommunicatorChannel(comm);

    comm->ServerSideHandshake();
  }

  std::cout << "Client connected to server" << std::endl;

  m_clientChannels.push_back(channel);


}

void voAnalysisServer::processConnectionEvents()
{
  int timeout = 200;
  vector<int> socketsToSelect;
  vector<vtkCommunicatorChannel *> channels;

  for (list<vtkCommunicatorChannel *>::iterator it
    = m_clientChannels.begin(); it != m_clientChannels.end(); ++it) {

    vtkCommunicatorChannel *channel = *it;

    vtkSocketCommunicator *comm = channel->communicator();
    vtkSocket* socket = comm->GetSocket();
    if (socket && socket->GetConnected()) {
      socketsToSelect.push_back(socket->GetSocketDescriptor());
      channels.push_back(channel);
    }
  }

  // Add server socket first, we are looking for incoming connections
  socketsToSelect.push_back(m_socket->GetSocketDescriptor());

  int selectedIndex = -1;
  int result = vtkSocket::SelectSockets(&socketsToSelect[0],
                                        socketsToSelect.size(), timeout,
                                        &selectedIndex);
  if (result < 0) {
    std::cerr << "Socket select failed with error code: " << result << std::endl;
    return;
  }

  if (selectedIndex == -1)
    return;

  // Are we dealing with an incoming connection?
  if (selectedIndex == socketsToSelect.size()-1) {
    accept();
  }
  // We have a message waiting from a client
  else {
    RpcChannel *channel = channels[selectedIndex];
    if (!channel->receive(true)) {
      // Connection lost remove channel from list
      std::cout << "Client disconnected from server" << std::endl;
      list<vtkCommunicatorChannel *>::iterator it
        = std::find(m_clientChannels.begin(), m_clientChannels.end(), channel);
      m_clientChannels.erase(it);
    }

  }
}

int main(int argc, char *argv[])
{
  // Register the RPC service
  ProtoCall::Runtime::ServiceManager *mgr
    = ProtoCall::Runtime::ServiceManager::instance();
  voRemoteAnalysisService service;
  voRemoteAnalysisService::Dispatcher dispatcher(&service);
  mgr->registerService(&dispatcher);

  voAnalysisServer server;
  server.listen(8888);
}
