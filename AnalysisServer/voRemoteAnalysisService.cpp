
#include "voRemoteAnalysisService.h"

// VTK includes
#include <vtkMultiBlockDataSet.h>
#include <vtkTable.h>
#include <vtkCompositeDataIterator.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkInformation.h>
#include <vtkType.h>

#include <sstream>

using std::string;
using std::ostringstream;

voRemoteAnalysisService::voRemoteAnalysisService()
{
  m_RCalc = vtkSmartPointer<vtkRCalculatorFilter>::New();
}

voRemoteAnalysisService::~voRemoteAnalysisService()
{
}

void voRemoteAnalysisService::execute(const AnalysisRequest* input,
    AnalysisResponse* output, ::google::protobuf::Closure* done)
{
  vtkMultiBlockDataSet *inputs = input->inputs().get();
  int numberOfOutputs = input->outputnames_size();

  m_RCalc->SetRoutput(0);

  bool multiInput = inputs->GetNumberOfBlocks() > 1;
  vtkCompositeDataIterator* iter = inputs->NewIterator();
  int index = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal();
       iter->GoToNextItem())
  {

    vtkDataObject *dataObj = iter->GetCurrentDataObject();
    const char *name = input->inputnames(index).c_str();
    int type = dataObj->GetDataObjectType();

    switch (type)
      {
      case VTK_TABLE: {
        vtkTable *table = vtkTable::SafeDownCast(iter->GetCurrentDataObject());
        if (!table)
          {
          iter->Delete();
          output->setErrorString("Input Table is Null");
          done->Run();
          return;
          }
        m_RCalc->PutTable(name);
        if (!multiInput)
          {
          m_RCalc->SetInputData(table);
          }
      }
        break;
      case VTK_TREE: {
        vtkTree* tree =
          vtkTree::SafeDownCast(iter->GetCurrentDataObject());
        if (!tree)
          {
          iter->Delete();
          output->setErrorString("Input Tree is Null");
          done->Run();
          return;
          }
        m_RCalc->PutTree(name);
        if (!multiInput)
          {
          m_RCalc->SetInputData(tree);
          }
      }
        break;
      default:
        iter->Delete();

        ostringstream msg;
        msg << "Unsupported input type: ";
        msg << type;
        output->setErrorString(msg.str());
        done->Run();
        return;
      }
    index++;
  }

  if (multiInput)
    {
    m_RCalc->SetInputData(inputs);
    }

  iter->Delete();

  for (index = 0; index <numberOfOutputs; index++ )
    {
    std::string name = input->outputnames(index);
    int type = input->outputtypes(index);

    switch (type)
      {
      case VTK_TABLE:
        m_RCalc->GetTable(name.c_str());
        break;
      case VTK_TREE:
        m_RCalc->GetTree(name.c_str());
        break;
      default:
        ostringstream msg;
        msg << "Unsupported output type: ";
        msg << type;
        output->setErrorString(msg.str());
        done->Run();
        return;
      }
    }

  // Set the script and update
  m_RCalc->SetRscript(input->script().c_str());
  m_RCalc->Update();

  vtkMultiBlockDataSet *results;

  // Get output(s) from R
  if (multiInput)
    {
    results =
      vtkMultiBlockDataSet::SafeDownCast(m_RCalc->GetOutput());
    if(!results)
      {
      output->setErrorString("Fatal error running analysis");
      done->Run();
      return;
      }

    output->mutable_outputs()->set(results);
    }
  else
    {
    results = vtkMultiBlockDataSet::New();
    results->SetNumberOfBlocks(1);
    vtkDataObject *outDataObj = m_RCalc->GetOutput();
    results->SetBlock(0, outDataObj);
    output->mutable_outputs()->set_allocated(results);
    }

  // Send result to client
  done->Run();
}

