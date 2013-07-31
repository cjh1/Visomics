/*=========================================================================

  Program: Visomics

  Copyright (c) Kitware, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QDebug>

// QtPropertyBrowser includes
#include <QtVariantProperty>
#include <QtVariantPropertyManager>

// Visomics includes
#include "voRemoteCustomAnalysis.h"
#include "voDataObject.h"
#include "voInputFileDataObject.h"
#include "voTableDataObject.h"
#include "voUtils.h"
#include "vtkExtendedTable.h"
#include "voCustomAnalysisInformation.h"

// VTK includes
#include <vtkArrayData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkDoubleArray.h>
#include <vtkGraph.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkNew.h>
#include <vtkRCalculatorFilter.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTableToGraph.h>
#include <vtkTree.h>
#include <vtkInformation.h>
#include <vtkInformationKey.h>
#include <vtkInformationStringKey.h>
#include <vtkInformationIntegerKey.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkSocketCommunicator.h>
#include <vtkSocketController.h>

// ProtoCall
#include "RemoteAnalysisService.pb.h"
#include <protocall/runtime/vtkcommunicatorchannel.h>
#include <google/protobuf/stubs/common.h>

using namespace google::protobuf;

// --------------------------------------------------------------------------
// voRemoteCustomAnalysis methods

// --------------------------------------------------------------------------
voRemoteCustomAnalysis::voRemoteCustomAnalysis(QObject* newParent):
    Superclass(newParent)
{

}

// --------------------------------------------------------------------------
voRemoteCustomAnalysis::~voRemoteCustomAnalysis()
{

}
// --------------------------------------------------------------------------
bool voRemoteCustomAnalysis::execute()
{

  AnalysisRequest remoteAnalysis;
  vtkSmartPointer<vtkExtendedTable> extendedTable;
  vtkNew<vtkMultiBlockDataSet> inputs;

  inputs->SetNumberOfBlocks(this->information()->inputs().size());

  int index = 0;
  foreach(voCustomAnalysisData *input, this->information()->inputs())
    {
    remoteAnalysis.add_inputnames(input->name().toStdString());

    vtkDataObject *data;

    if (input->type() == "Table")
      {
      extendedTable = this->getInputTable(index);
      data = extendedTable->GetInputData();
      }
    else if(input->type() == "Tree")
      {
      data =
        vtkTree::SafeDownCast(this->input(index)->dataAsVTKDataObject());
      if (!data)
        {
        qCritical() << "Input Tree is Null";
        return false;
        }
      }
    else
      {
      qCritical() << "Unsupported input type:" << input->type();
      return false;
      }
    inputs->SetBlock(index, data);
    ++index;
    }

  vtkNew<vtkMultiBlockDataSet> outputs;

  index = 0;
  outputs->SetNumberOfBlocks(this->information()->outputs().length());
  foreach(voCustomAnalysisData *output, this->information()->outputs())
    {

    remoteAnalysis.add_outputnames(output->name().toStdString());

    if (output->type() == "Table")
      {
      remoteAnalysis.add_outputtypes(VTK_TABLE);
      }
    else if(output->type() == "Tree")
      {
      remoteAnalysis.add_outputtypes(VTK_TREE);
      }
    else
      {
      qCritical() << "Unsupported output type:" << output->type();
      return false;
      }
    }

  // replace each parameter in the script with its actual value
  QString script = this->information()->script();
  foreach(voCustomAnalysisParameter *parameter, this->information()->parameters())
    {
    QString type = parameter->type();
    QString name = parameter->name();
    QString parameterValue;
    if (type == "Integer")
      {
      parameterValue = QString::number(this->integerParameter(name));
      }
    else if (type == "Double")
      {
      parameterValue = QString::number(this->doubleParameter(name));
      }
    else if (type == "Enum")
      {
      parameterValue = this->enumParameter(name);
      }
    else if (type == "String")
      {
      parameterValue = QString("\"%1\"").arg(this->stringParameter(name));
      }
    else
      {
      qCritical() << "Unsupported parameter type in voCustomAnalysis:" << type;
      }
    script.replace(name, parameterValue);
    }

  remoteAnalysis.mutable_inputs()->set(inputs.GetPointer());
  remoteAnalysis.set_script(script.toStdString());

  vtkNew<vtkSocketController> con;
  vtkNew<vtkSocketCommunicator> com;
  con->SetCommunicator(com.GetPointer());
  con->Initialize();

  if(!com->ConnectTo("localhost", 8888)) {
    cerr << "Connection failure" << endl;
    return false;
  }

  ProtoCall::Runtime::vtkCommunicatorChannel channel(com.GetPointer());
  RemoteAnalysisService::Proxy proxy(&channel);

  AnalysisResponse analysisResult;
  Closure *callback = NewCallback(this, &voRemoteCustomAnalysis::handleResult,
      &analysisResult);

  proxy.execute(&remoteAnalysis, &analysisResult, callback);

  // Wait for the response
  channel.receive();

  com->CloseConnection();

  if (analysisResult.hasError())
    return false;

  vtkMultiBlockDataSet *outputDataSet = analysisResult.outputs().get();

  vtkCompositeDataIterator* iter = outputDataSet->NewIterator();
  index = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal();
       iter->GoToNextItem())
  {
    voCustomAnalysisData *output = this->information()->outputs().at(index);
    if (output->type() == "Table")
      {
      vtkTable *outputTable =
          vtkTable::SafeDownCast(iter->GetCurrentDataObject());
      this->setOutput(output->name(),
                      new voTableDataObject(output->name(), outputTable, true));
      }
    else // tree is the only other possibility atm
      {
      vtkTree *outputTree = vtkTree::SafeDownCast(iter->GetCurrentDataObject());
      this->setOutput(output->name(),
                      new voInputFileDataObject(output->name(), outputTree));
      }
      ++index;
  }
  iter->Delete();


  return true;
}

// This provides an example of how the result can be processed in asyn mode,
// currently just prints out any errors ...
void voRemoteCustomAnalysis::handleResult(AnalysisResponse *analysisResult)
{
  if (analysisResult->hasError())
    {
    qCritical() << "[Remote Custom Analysis Error] "
        << analysisResult->errorString().c_str();
    qCritical() << analysisResult->errorCode();
    }
}
