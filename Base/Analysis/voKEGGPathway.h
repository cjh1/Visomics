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

#ifndef __voKEGGPathway_h
#define __voKEGGPathway_h

// Qt includes
#include <QScopedPointer>

// Visomics includes
#include "voAnalysis.h"

class voKEGGPathwayPrivate;

class voKEGGPathway : public voAnalysis
{
  Q_OBJECT
public:
  typedef voAnalysis Superclass;
  voKEGGPathway();
  virtual ~voKEGGPathway();

protected:
  virtual void setOutputInformation();
  virtual void setParameterInformation();
  virtual QString parameterDescription()const;

  virtual bool execute();

protected:
  QScopedPointer<voKEGGPathwayPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(voKEGGPathway);
  Q_DISABLE_COPY(voKEGGPathway);
};

#endif
