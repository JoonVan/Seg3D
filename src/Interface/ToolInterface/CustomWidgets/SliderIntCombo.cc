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


//Core Includes - for logging
#include <Utils/Core/Log.h>

#include <Interface/ToolInterface/CustomWidgets/SliderIntCombo.h>
#include "ui_SliderIntCombo.h"

namespace Seg3D
{

class SliderIntComboPrivate
{
public:
    Ui::SliderIntCombo ui_;
};

SliderIntCombo::SliderIntCombo( QWidget* parent, bool edit_range ) :
    private_( new SliderIntComboPrivate ), QWidget( parent )
{
    this->private_->ui_.setupUi( this );
    
    this->private_->ui_.decrease_range_button_->hide();
    this->private_->ui_.increase_range_button_->hide();
    
    if( !edit_range )
    {
        this->private_->ui_.edit_button_->setEnabled( edit_range );
        this->private_->ui_.edit_button_->hide();
    }
    
    connect( this->private_->ui_.edit_button_, SIGNAL(clicked( bool )), this, SLOT(edit_ranges(bool)));
    connect( this->private_->ui_.decrease_range_button_, SIGNAL(clicked()), this, SLOT(half_range()));
    connect( this->private_->ui_.increase_range_button_, SIGNAL(clicked()), this, SLOT(double_range()));
    
    connect( this->private_->ui_.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slider_signal( int )));
    connect( this->private_->ui_.spinBox, SIGNAL(valueChanged(int)), this, SLOT(spinner_signal( int )));
}

SliderIntCombo::~SliderIntCombo()
{
}

void SliderIntCombo::slider_signal( int value )
{
    this->private_->ui_.horizontalSlider->blockSignals( true );
    this->private_->ui_.horizontalSlider->setValue( value );
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.horizontalSlider->blockSignals( false );
}

void SliderIntCombo::spinner_signal( int value )
{
    this->private_->ui_.spinBox->blockSignals( true );
    this->private_->ui_.spinBox->setValue( value );
    Q_EMIT valueAdjusted( value );
  this->private_->ui_.spinBox->blockSignals( false );
}


void SliderIntCombo::setStep(int step)
{
    this->private_->ui_.horizontalSlider->setSingleStep( step );
    this->private_->ui_.spinBox->setSingleStep( step );
}
void SliderIntCombo::setRange( int min, int max)
{
    this->private_->ui_.horizontalSlider->setRange( min, max );
    this->private_->ui_.spinBox->setRange( min, max );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );
    
    int tick = (max - min)/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
}
void SliderIntCombo::setCurrentValue( int value )
{
    this->private_->ui_.horizontalSlider->setValue( value );
    this->private_->ui_.spinBox->setValue( value );
}

void SliderIntCombo::edit_ranges( bool edit )
{
    if( edit )
    {
        this->private_->ui_.decrease_range_button_->show();
        this->private_->ui_.increase_range_button_->show();
    }
    else
    {
        this->private_->ui_.decrease_range_button_->hide();
        this->private_->ui_.increase_range_button_->hide();
    }
}

void SliderIntCombo::change_min( int new_min )
{
    this->private_->ui_.horizontalSlider->setMinimum( new_min );
    this->private_->ui_.spinBox->setMinimum( new_min );
    this->private_->ui_.min_->setNum(new_min);
    int tick = (this->private_->ui_.max_->text().toInt() - this->private_->ui_.min_->text().toInt())/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
}

void SliderIntCombo::change_max( int new_max )
{
    this->private_->ui_.horizontalSlider->setMaximum( new_max );
    this->private_->ui_.spinBox->setMaximum( new_max );
    this->private_->ui_.max_->setNum( new_max );
    int tick = (this->private_->ui_.max_->text().toInt() - this->private_->ui_.min_->text().toInt())/10;
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
}

void SliderIntCombo::set_all( int min, int max, int value )
{
    int tick = (max - min)/10;

    this->private_->ui_.horizontalSlider->setRange(min, max);
    this->private_->ui_.horizontalSlider->setValue( value );
    this->private_->ui_.horizontalSlider->setTickInterval( tick );
    this->private_->ui_.spinBox->setRange(min, max);
    this->private_->ui_.spinBox->setValue( value );
    this->private_->ui_.min_->setNum( min );
    this->private_->ui_.max_->setNum( max );

}

void SliderIntCombo::double_range()
{
    int new_max = this->private_->ui_.max_->text().toInt() * 2;
    change_max( new_max );
    rangeChanged( new_max );
}
void SliderIntCombo::half_range()
{
    int new_max = this->private_->ui_.max_->text().toInt() / 2;
    change_max( new_max );
    rangeChanged( new_max );
}



}  // end namespace Seg3D