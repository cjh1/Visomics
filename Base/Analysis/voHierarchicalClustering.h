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

#ifndef __voHierarchicalClustering_h
#define __voHierarchicalClustering_h

// Qt includes
#include <QScopedPointer>

// Visomics includes
#include "voAnalysis.h"

class voHierarchicalClustering : public voAnalysis
{
  Q_OBJECT
public:
  typedef voAnalysis Superclass;
  voHierarchicalClustering();
  virtual ~voHierarchicalClustering();

protected:
  virtual void setOutputInformation();
  virtual void setParameterInformation();
  virtual QString parameterDescription()const;

  virtual bool execute();

private:
  Q_DISABLE_COPY(voHierarchicalClustering);
};

#endif
