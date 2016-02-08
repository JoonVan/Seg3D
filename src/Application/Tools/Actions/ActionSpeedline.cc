/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <boost/math/special_functions/round.hpp>

#include <Core/Action/ActionFactory.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Geometry/Path.h>

//#include <Core/ITKSpeedLine/itkSpeedFunctionToPathFilter.h>
//#include <Core/ITKSpeedLine/itkArrivalFunctionToPathFilter.h>

#include <Core/ITKLiveWire/itkLiveWireImageFunction.h>

#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/Tools/Actions/ActionSpeedline.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/ITKFilter.h>

#include <Application/Tools/SpeedlineTool.h>

//ITK Includes
//#include <itkRescaleIntensityImageFilter.h>
//#include <itkDiscreteGaussianImageFilter.h>
#include <itkExtractImageFilter.h>
//#include <itkLinearInterpolateImageFunction.h>
#include <itkPathIterator.h>
//#include <itkIndex.h>
#include <itkImageBase.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

CORE_REGISTER_ACTION( Seg3D, Speedline )

namespace Seg3D
{

class ActionSpeedlineAlgo : public  ITKFilter
{
  static const unsigned int VOL_DIM_C = 3;
  static const unsigned int SLICE_DIM_C = 2;
//  typedef double pixel_type;
//  typedef itk::Image< pixel_type, SLICE_DIM_C > image_type;
//  typedef itk::PolyLineParametricPath< SLICE_DIM_C > path_type;
//  typedef itk::SpeedFunctionToPathFilter< image_type, path_type > path_filter_type;
//  typedef path_filter_type::CostFunctionType::CoordRepType coord_rep_type; // double
//
//  typedef itk::ContinuousIndex< double, 3 >   continuous_index_type;
//  typedef itk::Point< double, 3 > itk_point_type;

//  Core::Path itk_paths_;
  Core::Path world_paths_;

//  continuous_index_type start_point_cindex_; // Continuous index for start point
//  std::vector< continuous_index_type > vertices_cindex_;

public:
  std::string target_layer_id_;
  std::string output_mask_layer_id_;
  std::string roi_mask_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector< Core::Point > vertices_;

//  std::vector< Core::Point > path_vertices_;
//  int current_vertex_index_;

  DataLayerHandle target_layer_;
  MaskLayerHandle roi_mask_layer_;
  MaskLayerHandle output_mask_layer_;

//  int iterations_;
//  double termination_;
  double grad_mag_weight_;
  double zero_cross_weight_;
  double grad_dir_weight_;
  bool image_spacing_;
  bool face_conn_;

//  bool update_all_paths_;
//  std::string itk_path_state_id_;
  std::string world_path_state_id_;
//  std::string path_vertices_state_id_;

//  long action_id_;
//  Core::AtomicCounterHandle action_handle_;

public:
  // When mouse is moved, clean the temporary paths
//  bool clean_paths( )
//  {
//    bool delete_outdated_path = false;
//    std::vector< Core::SinglePath > tmp_paths;
//
//    size_t paths_num = this->itk_paths_.get_path_num();
//    for ( unsigned int i = 0; i < paths_num; ++i )
//    {
//      Core::SinglePath single_path = this->itk_paths_.get_one_path( i );
//
//      Core::Point p0, p1;
//      single_path.get_point_on_ends( 0, p0 );
//      single_path.get_point_on_ends( 1, p1 );
//
//      std::vector< Core::Point >::iterator it0  =
//      std::find( this->vertices_.begin(), this->vertices_.end(), p0 );
//
//      std::vector< Core::Point >::iterator it1  =
//      std::find( this->vertices_.begin(), this->vertices_.end(), p1 );
//
//      if ( it0 == this->vertices_.end() || it1 == this->vertices_.end() )
//      {
//        delete_outdated_path = true;
//      }
//      else
//      {
//        tmp_paths.push_back( single_path );
//      }
//    }
//
//    this->itk_paths_.delete_all_paths();
//    for ( std::vector< Core::SinglePath >::iterator it = tmp_paths.begin(); it != tmp_paths.end(); ++it )
//    {
//      this->itk_paths_.add_one_path( *it );
//    }
//
//    //this->itk_paths_.set_all_paths( tmp_paths );
//
//    return delete_outdated_path;
//  }

  template< class TYPED_IMAGE_TYPE, class TYPED_IMAGE_TYPE_2D >
  typename TYPED_IMAGE_TYPE_2D::Pointer extract_2D_image( typename TYPED_IMAGE_TYPE::Pointer image )
  {
    // Create 2D itk image
    typedef itk::ExtractImageFilter< TYPED_IMAGE_TYPE, TYPED_IMAGE_TYPE_2D >  extract_filter_type;
    typename extract_filter_type::Pointer extract_filter = extract_filter_type::New();

    typename TYPED_IMAGE_TYPE::RegionType input_region = image->GetLargestPossibleRegion();
    typename TYPED_IMAGE_TYPE::SizeType size = input_region.GetSize();
    typename TYPED_IMAGE_TYPE::IndexType start = input_region.GetIndex();

    if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
    {
      // Collapse Dim X
      size[ 0 ] = 0;
      start[ 0 ] = this->slice_number_;
    }
    else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
    {
      // Collapse Dim Y
      size[ 1 ] = 0;
      start[ 1 ] = this->slice_number_;

    }
    else
    {
      // AXIAL_E
      // Collapse Dim Z
      size[ 2 ] = 0;
      start[ 2 ] = this->slice_number_;
    }

    typename TYPED_IMAGE_TYPE::RegionType desired_region;
    desired_region.SetSize( size );
    desired_region.SetIndex( start );

    extract_filter->SetExtractionRegion( desired_region );
    extract_filter->SetInput( image );
    extract_filter->Update();

    typename TYPED_IMAGE_TYPE_2D::Pointer input_image_2D = extract_filter->GetOutput();
    input_image_2D->DisconnectPipeline();

    return input_image_2D;
  }

  template< class TYPED_IMAGE_TYPE_2D >
  typename TYPED_IMAGE_TYPE_2D::IndexType extract_2D_point( const Core::Point& point )
  {
    typename TYPED_IMAGE_TYPE_2D::IndexType slice_point;

    if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
    {
      // Collapse Dim X (YZ plane)
      slice_point[0] = point[1];
      slice_point[1] = point[2];
    }
    else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
    {
      // Collapse Dim Y (XZ plane)
      slice_point[0] = point[0];
      slice_point[1] = point[2];
    }
    else
    {
      // AXIAL_E (XY plane)
      // Collapse Dim Z
      slice_point[0] = point[0];
      slice_point[1] = point[1];
    }
    return slice_point;
  }

  template< class PATH_TYPE >
  Core::Point build_3D_point( typename PATH_TYPE::ContinuousIndexType slice_point )
  {
    Core::Point point;

    if ( this->slice_type_ == Core::VolumeSliceType::SAGITTAL_E )
    {
      // Restore Dim X (YZ plane)
      point[0] = this->slice_number_;
      point[1] = slice_point[0];
      point[2] = slice_point[1];
    }
    else if ( this->slice_type_ == Core::VolumeSliceType::CORONAL_E )
    {
      // Restore Dim Y (XZ plane)
      point[0] = slice_point[0];
      point[1] = this->slice_number_;
      point[2] = slice_point[1];
    }
    else
    {
      // AXIAL_E (XY plane)
      // Restore Dim Z
      point[0] = slice_point[0];
      point[1] = slice_point[1];
      point[2] = this->slice_number_;
    }
    return point;
  }

  SCI_BEGIN_TYPED_ITK_RUN( this->target_layer_->get_data_type() )
  {
    Core::StateSpeedlinePathHandle world_path_state;
    Core::StateBaseHandle state_var_1;

    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

      if ( Core::StateEngine::Instance()->get_state( this->world_path_state_id_, state_var_1 ) )
      {
        world_path_state = boost::dynamic_pointer_cast< Core::StateSpeedlinePath > ( state_var_1 );
        if ( world_path_state )
        {
          this->world_paths_ = world_path_state->get();
        }
      }
    }

    this->world_paths_.delete_all_paths();

    typedef itk::Image< int, VOL_DIM_C> MASK_IMAGE_TYPE;
    typedef itk::Image< int, SLICE_DIM_C > MASK_IMAGE_TYPE_2D;
    typedef itk::Image< VALUE_TYPE, SLICE_DIM_C > TYPED_IMAGE_TYPE_2D;
    typedef itk::LiveWireImageFunction< TYPED_IMAGE_TYPE_2D > LiveWireType;
    typedef typename LiveWireType::OutputType PATH_TYPE;

    typename Core::ITKImageDataT< int >::Handle roi_mask_image;
    typename Core::ITKImageDataT< VALUE_TYPE >::Handle input_image;

    this->get_itk_image_from_layer< VALUE_TYPE >( this->target_layer_, input_image );

    typename TYPED_IMAGE_TYPE_2D::Pointer input_image_2D =
      extract_2D_image< TYPED_IMAGE_TYPE, TYPED_IMAGE_TYPE_2D >( input_image->get_image() );

    typename LiveWireType::Pointer livewire = LiveWireType::New();

    livewire->SetInputImage( input_image_2D );

    livewire->SetGradientMagnitudeWeight( this->grad_mag_weight_ );
    livewire->SetZeroCrossingWeight( this->zero_cross_weight_ );
    livewire->SetGradientDirectionWeight( this->grad_dir_weight_ );
    livewire->SetUseImageSpacing( this->image_spacing_ );
    livewire->SetUseFaceConnectedness( this->face_conn_ );

    if ( this->roi_mask_layer_id_ != "<none>" )
    {
      this->get_itk_image_from_layer< int >( this->roi_mask_layer_, roi_mask_image );
      typename MASK_IMAGE_TYPE_2D::Pointer roi_mask_image_2D =
        extract_2D_image< MASK_IMAGE_TYPE, MASK_IMAGE_TYPE_2D >( roi_mask_image->get_image() );

      livewire->SetMaskImage( roi_mask_image_2D );
      // TODO: not sure about pixel value...
      livewire->SetInsidePixelValue( 1 );
    }

    typename TYPED_IMAGE_TYPE_2D::IndexType anchor =
      extract_2D_point< TYPED_IMAGE_TYPE_2D >( this->vertices_[0] );
    livewire->SetAnchorSeed( anchor );

    size_t end_index = this->vertices_.size() - 1;
    Core::SinglePath new_path( this->vertices_[0], this->vertices_[end_index] );

    this->world_paths_.set_start_point( this->vertices_[0] );
    this->world_paths_.set_end_point( this->vertices_[end_index] );

    for (size_t i = 1; i < this->vertices_.size(); ++i)
    {
      typename TYPED_IMAGE_TYPE_2D::IndexType free =
        extract_2D_point< TYPED_IMAGE_TYPE_2D >( this->vertices_[i] );

      typename PATH_TYPE::Pointer path = livewire->EvaluateAtIndex( free );
      if ( path == nullptr )
      {
        // TODO: report point
        this->report_error( "Error computing path..." );
        return;
      }

      const typename PATH_TYPE::VertexListType* vertexList = path->GetVertexList ();
      for(size_t j = 0; j < vertexList->Size(); ++j)
      {
//        std::cout << vertexList->GetElement(j) << std::endl;
        Core::Point point = build_3D_point< PATH_TYPE >( vertexList->GetElement(j) );
        new_path.add_a_point( point );
      }
    }
    this->world_paths_.add_one_path( new_path );

    Core::Application::PostEvent( boost::bind( &Core::StateSpeedlinePath::set,
      world_path_state, this->world_paths_, Core::ActionSource::NONE_E ) );


  }
  SCI_END_TYPED_ITK_RUN()


  virtual std::string get_filter_name() const
  {
    return "SpeedLine Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const
  {
    return "Speed";
  }
};

class ActionSpeedlinePrivate
{
public:
  // == action parameters ==
  std::string target_layer_id_;
  std::string roi_mask_layer_id_;
  std::string output_mask_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector< Core::Point > vertices_;
  double grad_mag_weight_;
  double zero_cross_weight_;
  double grad_dir_weight_;
  bool image_spacing_;
  bool face_conn_;

//  int current_vertex_index_;
//  std::string itk_path_state_id_;
  std::string world_path_state_id_;
//  std::string path_vertices_state_id_;
//  bool update_all_paths_;

//  long action_id_;
//  Core::AtomicCounterHandle action_handle_;
};

ActionSpeedline::ActionSpeedline() :
  private_( new ActionSpeedlinePrivate )
{
  this->add_layer_id( this->private_->target_layer_id_ );
  this->add_layer_id( this->private_->output_mask_layer_id_ );
  this->add_parameter( this->private_->slice_type_ );
  this->add_parameter( this->private_->slice_number_ );
  this->add_parameter( this->private_->vertices_ );
  this->add_layer_id( this->private_->roi_mask_layer_id_ );
  this->add_parameter( this->private_->grad_mag_weight_ );
  this->add_parameter( this->private_->zero_cross_weight_ );
  this->add_parameter( this->private_->grad_dir_weight_ );
  this->add_parameter( this->private_->image_spacing_ );
  this->add_parameter( this->private_->face_conn_ );
//  this->add_parameter( this->private_->current_vertex_index_ );
//  this->add_parameter( this->private_->update_all_paths_ );
//  this->add_parameter( this->private_->itk_path_state_id_ );
  this->add_parameter( this->private_->world_path_state_id_ );
//  this->add_parameter( this->private_->path_vertices_state_id_ );
}

bool ActionSpeedline::validate( Core::ActionContextHandle& context )
{
//  if (this->private_->action_handle_ == nullptr )
//  {
//    return false;
//  }
//
//  long counter = *( this->private_->action_handle_ );
//  if ( this->private_->action_id_ < counter )
//  {
//    return false;
//  }
//
//  // Check whether the target layer exists
//  if ( !LayerManager::CheckLayerExistenceAndType( this->private_->target_layer_id_,
//    Core::VolumeType::DATA_E, context ) )
//  {
//    return false;
//  }

  if ( this->private_->vertices_.size() < 2 )
  {
    context->report_error( "At least 2 points needed for speedline." );
    return false;
  }

  // Check whether the target layer can be used for read
  if ( ! LayerManager::CheckLayerAvailabilityForUse( this->private_->target_layer_id_, context ) )
  {
    context->report_error( "Target layer not available for read." );
    return false;
  }

  if ( this->private_->slice_type_ != Core::VolumeSliceType::AXIAL_E &&
       this->private_->slice_type_ != Core::VolumeSliceType::CORONAL_E &&
       this->private_->slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type." );
    return false;
  }
  
  DataLayerHandle target_layer = LayerManager::FindDataLayer( this->private_->target_layer_id_ );
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >( this->private_->slice_type_ );
  Core::DataVolumeSliceHandle volume_slice( new Core::DataVolumeSlice( target_layer->get_data_volume(), slice_type ) );
  if ( this->private_->slice_number_ >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  return true;
}

bool ActionSpeedline::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // Compare whether this is a newer action
  // If not, drop it
  // else perform the action
//  if (this->private_->action_handle_ == nullptr )
//  {
//    return false;
//  }

//  long counter = *( this->private_->action_handle_ );
//  if ( this->private_->action_id_ < counter )
//  {
//    return false;
//  }

  // Create algorithm
  typedef boost::shared_ptr< ActionSpeedlineAlgo > ActionSpeedlineAlgoHandle;
  ActionSpeedlineAlgoHandle algo( new ActionSpeedlineAlgo );

  algo->target_layer_id_ = this->private_->target_layer_id_;
  algo->roi_mask_layer_id_ = this->private_->roi_mask_layer_id_;
  algo->output_mask_layer_id_ = this->private_->output_mask_layer_id_;

  algo->slice_type_ = this->private_->slice_type_ ;
  algo->slice_number_ = this->private_->slice_number_;
  algo->vertices_ = this->private_->vertices_;
//  algo->current_vertex_index_ = this->private_->current_vertex_index_;
  algo->grad_mag_weight_ = this->private_->grad_mag_weight_;
  algo->zero_cross_weight_ = this->private_->zero_cross_weight_;
  algo->grad_dir_weight_ = this->private_->grad_dir_weight_;
  algo->image_spacing_ = this->private_->image_spacing_;
  algo->face_conn_ = this->private_->face_conn_;

  algo->target_layer_ = LayerManager::FindDataLayer( this->private_->target_layer_id_ );
  if ( algo->roi_mask_layer_id_ != "<none>" )
  {
    algo->roi_mask_layer_ = LayerManager::FindMaskLayer( this->private_->roi_mask_layer_id_ );
  }
  algo->output_mask_layer_ = LayerManager::FindMaskLayer( this->private_->output_mask_layer_id_ );

//  algo->update_all_paths_ = this->private_->update_all_paths_;
//  algo->itk_path_state_id_ = this->private_->itk_path_state_id_;
  algo->world_path_state_id_ = this->private_->world_path_state_id_;
//  algo->path_vertices_state_id_ = this->private_->path_vertices_state_id_;
//  algo->action_id_ = this->private_->action_id_;
//  algo->action_handle_ = this->private_->action_handle_;

  Core::Runnable::Start( algo );

  return true;
}

void ActionSpeedline::Dispatch( Core::ActionContextHandle context,
                                const std::string& layer_id,
                                const std::string& mask_id,
                                const std::string& roi_mask_id,
                                Core::VolumeSliceType slice_type,
                                size_t slice_number,
                                const std::vector< Core::Point > vertices,
                                const double grad_mag_weight,
                                const double zero_cross_weight,
                                const double grad_dir_weight,
                                const bool image_spacing,
                                const bool face_conn,
                                //int current_vertex_index,
                                //bool update_all_paths,
                                //const std::string& itk_path_state_id,
                                const std::string& world_path_state_id
                                //const std::string& path_vertices_state_id,
                                //long action_id,
                                //Core::AtomicCounterHandle action_handle
                              )
{
  ActionSpeedline* action = new ActionSpeedline;

  action->private_->target_layer_id_ = layer_id;
  action->private_->output_mask_layer_id_ = mask_id;
  action->private_->roi_mask_layer_id_ = roi_mask_id;
  action->private_->slice_type_ = slice_type;
  action->private_->slice_number_ = slice_number;
  action->private_->vertices_ = vertices;
  action->private_->grad_mag_weight_ = grad_mag_weight;
  action->private_->zero_cross_weight_ = zero_cross_weight;
  action->private_->grad_dir_weight_ = grad_dir_weight;
  action->private_->image_spacing_ = image_spacing;
  action->private_->face_conn_ = face_conn;
  //action->private_->current_vertex_index_ = current_vertex_index;
  //action->private_->update_all_paths_ = update_all_paths;
  //action->private_->itk_path_state_id_ = itk_path_state_id;
  action->private_->world_path_state_id_ = world_path_state_id;
  //action->private_->path_vertices_state_id_ = path_vertices_state_id;
  //action->private_->action_id_ = action_id;
  //action->private_->action_handle_ = action_handle;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
