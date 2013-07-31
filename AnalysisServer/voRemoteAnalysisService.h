#ifndef VOREMOTEANALYSISSERVICE_H_
#define VOREMOTEANALYSISSERVICE_H_

#include "RemoteAnalysisService.pb.h"

// VTK includes
#include <vtkRCalculatorFilter.h>
#include <vtkSmartPointer.h>

class voRemoteAnalysisService : public RemoteAnalysisService {
public:
  voRemoteAnalysisService();
  virtual ~voRemoteAnalysisService();
  void execute(const AnalysisRequest* input,
      AnalysisResponse* output, ::google::protobuf::Closure* done);
private:
  vtkSmartPointer<vtkRCalculatorFilter> m_RCalc;
};

#endif /* VOREMOTEANALYSISSERVICE_H_ */
