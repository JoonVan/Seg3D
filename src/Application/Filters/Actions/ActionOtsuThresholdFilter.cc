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


#include <Application/LayerManager/LayerManager.h>
#include <Application/Filters/Actions/ActionOtsuThreshold.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, OtsuThreshold )

namespace Seg3D
{
  
bool ActionOtsuThreshold::validate( Core::ActionContextHandle& context )
{
  if( !( Core::StateEngine::Instance()->is_statealias( this->layer_alias_ ) ) )
  {
    context->report_error( std::string( "LayerID '" ) + this->layer_alias_ + "' is invalid" );
    return false;
  }
  if( this->order_ < 0 )
  {
    return false;
  }
  return true;
}

bool ActionOtsuThreshold::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  if( Core::StateEngine::Instance()->is_statealias( this->layer_alias_ ) )
  {
    // TODO: run filter
    context->report_message( "The Otsu Threshold Filter has been triggered "
      "successfully on layer: "  + this->layer_alias_ );
    return true;
  }
    
  return false;
}


void ActionOtsuThreshold::Dispatch( std::string layer_alias, int order )
{
  ActionOtsuThreshold* action = new ActionOtsuThreshold;
  action->layer_alias_ = layer_alias;
  action->order_ = order;
  
  Core::Interface::PostAction( Core::ActionHandle( action ) );
}
  
} // end namespace Seg3D