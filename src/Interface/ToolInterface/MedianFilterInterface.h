/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2009 Scientific Computing and Imaging Institute,
 University of Utah.
 
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

#ifndef INTERFACE_TOOLINTERFACE_MEDIANFILTERINTERFACE_H
#define INTERFACE_TOOLINTERFACE_MEDIANFILTERINTERFACE_H

// Application includes
#include <Application/Tool/ToolFactory.h>

// Base class of the tool widget
#include <Interface/AppInterface/ToolWidget.h>
#include <Interface/ToolInterface/CustomWidgets/SliderSpinCombo.h>


namespace Seg3D {
  
class MedianFilterInterfacePrivate;

class MedianFilterInterface : public ToolWidget {
  Q_OBJECT

Q_SIGNALS:
  void activeChanged(int);
  void radiusSizeChanged(int);

public:
  MedianFilterInterface();
  virtual ~MedianFilterInterface();
  virtual bool build_widget(QFrame* frame);
  
public Q_SLOTS:
  void setActive(int);
  void addToActive(QStringList&);
  void setRadiusSize(int);

  
private:
  MedianFilterInterfacePrivate* private_;
  SliderSpinCombo *radiusSizeAdjuster;
  void makeConnections();
  
private Q_SLOTS:
  void senseActiveChanged(int);
  void senseRadiusSizeChanged(double);





};

} // namespace Seg3D

#endif