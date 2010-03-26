/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbGCPsToRPCSensorModelImageFilter_txx
#define __otbGCPsToRPCSensorModelImageFilter_txx

#include "otbGCPsToRPCSensorModelImageFilter.h"

#include "itkMetaDataObject.h"

#include "projection/ossimRpcSolver.h"
#include "imaging/ossimImageGeometry.h"

namespace otb {

template < class TImage >
GCPsToRPCSensorModelImageFilter<TImage>
::GCPsToRPCSensorModelImageFilter() : m_UseImageGCPs(true), 
                                  m_RMSGroundError(0.), 
                                  m_UseDEM(false),
                                  m_MeanElevation(0.), 
                                  m_DEMHandler(),
                                  m_GCPsContainer(),
                                  m_RpcProjection()
{
  // This filter does not modify the image buffer, but only its
  // metadata.Therefore, it can be run inplace to reduce memory print.
  this->SetInPlace(true);

  // Clear the GCPs container
  this->ClearGCPs();
  
  // Create projection
  m_RpcProjection = new ossimRpcProjection();

  /** Create the DEM handler */
  m_DEMHandler = DEMHandler::New();
}

template < class TImage >
GCPsToRPCSensorModelImageFilter< TImage >
::~GCPsToRPCSensorModelImageFilter()
{
  // Clear the GCPs container
  this->ClearGCPs();
}

template < class TImage >
typename GCPsToRPCSensorModelImageFilter< TImage >
::GCPsContainerType &
GCPsToRPCSensorModelImageFilter< TImage >
::GetGCPsContainer()
{
  // return the GCPs container
  return m_GCPsContainer;
}

template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::SetGCPsContainer(const GCPsContainerType & container)
{
  // Set the GCPs container
  m_GCPsContainer = container;
  // Call modified method
  this->Modified();
}

template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::AddGCP(const Point2DType& sensorPoint, const Point3DType & groundPoint)
{
  // Create a new GCP
  GCPType newGCP(sensorPoint,groundPoint);
  // Add it to the container
  m_GCPsContainer.push_back(newGCP);
  // Call the modified method */
  this->Modified();
}

template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::AddGCP(const Point2DType& sensorPoint, const Point2DType & groundPoint)
{
  // Create the ground point with elevation
  Point3DType groundPointWithElevation;
  // Fill it with ground point data
  groundPointWithElevation[0] = groundPoint[0];
  groundPointWithElevation[1] = groundPoint[1];
  
  // Check wether we are using a DEM or not
  if(m_UseDEM)
  {
    // If so, use it to get the elevation
    double height = m_DEMHandler->GetHeightAboveMSL(groundPoint);
    // To avoid nan value
    if (height != height)
      height = 0;
    groundPointWithElevation[2] = height;
  }
  else
  {
    // Use the MeanElevation value
    groundPointWithElevation[2] = m_MeanElevation;
  }
  
  // Call the 3D version of the method
  this->AddGCP(sensorPoint,groundPointWithElevation);
}

template< class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::RemoveGCP(unsigned int id)
{
  if (id < m_GCPsContainer.size())
  {
    m_GCPsContainer.erase(m_GCPsContainer.begin() + id);
  }
  
  this->Modified();
}

template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::ClearGCPs()
{
  m_GCPsContainer.clear();
  this->Modified();
}


/** Transform point */
template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::TransformPoint(const Point2DType sensorPoint, Point3DType & groundPoint, double height)
{
  ossimDpt spoint(sensorPoint[0], sensorPoint[1]);
  ossimGpt gpoint;
  
  gpoint.hgt = height;
  
  if( m_Projection != NULL )
  {
    m_Projection->lineSampleToWorld(spoint, gpoint);
    
    groundPoint[0] = gpoint.lon;
    groundPoint[1] = gpoint.lat;
    groundPoint[2] = gpoint.hgt;
  }
  else
  {
    groundPoint[0] = 0.;
    groundPoint[1] = 0.;
    groundPoint[2] = 0.;
  }
}

template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::ComputeErrors()
{
  ContinuousIndexType idFix, idOut;
  Continuous3DIndexType idOut3D, idTrans3D;

  Point2DType sensorPoint;
  Point3DType groundPoint;

  double sum = 0.;
  m_MeanError = 0.;

  // Clear Error container
  m_ErrorsContainer.clear();

  for (unsigned int i=0; i<m_GCPsContainer.size(); i++)
  {
    // GCP value   
    sensorPoint = m_GCPsContainer[i].first;
    groundPoint = m_GCPsContainer[i].second;
  
    // Comute Transform
    Point3DType groundPointTemp;
    this->TransformPoint(sensorPoint, groundPointTemp, groundPoint[2]);
    
    // Compute Euclidian distance
    idFix[0] = sensorPoint[0];
    idFix[1] = sensorPoint[1];
    
    idOut3D[0] = groundPoint[0];
    idOut3D[1] = groundPoint[1];
    idOut3D[2] = groundPoint[2];    

    idTrans3D[0] = groundPointTemp[0];
    idTrans3D[1] = groundPointTemp[1];
    idTrans3D[2] = groundPointTemp[2];
  
    double distance = idOut3D.EuclideanDistanceTo(idTrans3D);
    double error = vcl_sqrt(distance);
    
    // Add error to the container
    m_ErrorsContainer.push_back(error);
    
    // Compute mean error
    sum += distance;
  }
  
  sum = vcl_sqrt(sum);
  m_MeanError = sum / static_cast<double>(m_ErrorsContainer.size());
}

template< class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::GenerateOutputInformation()
{
  // First, call the superclass implementation
  Superclass::GenerateOutputInformation();

  // First, retrieve the image pointer
  typename TImage::Pointer imagePtr = this->GetOutput();

  // The vector where geo and sensor points are stored
  std::vector<ossimDpt> sensorPoints;
  std::vector<ossimGpt>  geoPoints;

  // Temporary points variable
  ossimDpt sensorPoint;
  ossimGpt geoPoint;
  
  // Handle the UseImageGCPs case
  if(m_UseImageGCPs)
    {
    // Iterate on the image GCPs
    for( unsigned int i=0; i<imagePtr->GetGCPCount(); i++ )
      {
      // Store row/col in an index
      typename TImage::IndexType index;
      index[0] = static_cast<long int>(imagePtr->GetGCPCol(i));
      index[1] = static_cast<long int>(imagePtr->GetGCPRow(i));

      // Transform to physical point
      Point2DType physPoint;
      imagePtr->TransformIndexToPhysicalPoint(index,physPoint);
    
      // Fill sensor point
      sensorPoint = ossimDpt(physPoint[0],physPoint[1]);
    
      // Fill geo point
      geoPoint = ossimGpt(imagePtr->GetGCPX(i),imagePtr->GetGCPY(i),
                       imagePtr->GetGCPZ(i));
    
      // Add the sensor point to the list
      sensorPoints.push_back(sensorPoint);
      
      // Add the geo point to the list 
      geoPoints.push_back(geoPoint);
      }
    }

  // Retrieve the additional GCPs
  typename GCPsContainerType::const_iterator gcpIt;
  for(gcpIt = m_GCPsContainer.begin(); gcpIt!=m_GCPsContainer.end();++gcpIt)
    {
    // Fill sensor point
    sensorPoint = ossimDpt(gcpIt->first[0],gcpIt->first[1]);
    
    // Fill geo point
    geoPoint = ossimGpt(gcpIt->second[0],gcpIt->second[1],gcpIt->second[2]);
    
    // Add the sensor point to the list
    sensorPoints.push_back(sensorPoint);

    // Add the geo point to the list 
    geoPoints.push_back(geoPoint);
    }

  // Build the ossim rpc solver
  ossimRefPtr<ossimRpcSolver> rpcSolver = new ossimRpcSolver(false,false);

  // Call the solve method
  rpcSolver->solveCoefficients(sensorPoints,geoPoints);

  // Retrieve the residual ground error
  m_RMSGroundError = rpcSolver->getRmsError();

  // Retrieve the output RPC projection
  ossimRefPtr<ossimRpcModel> rpcModel = dynamic_cast<ossimRpcModel*>(rpcSolver->createRpcModel()->getProjection());
  
  // Export the sensor model in an ossimKeywordlist
  ossimKeywordlist geom_kwl;
  rpcModel->saveState(geom_kwl);
  
  // Compute projection
  ossimRefPtr< ossimRpcProjection > objp = dynamic_cast<ossimRpcProjection*>(rpcSolver->createRpcProjection()->getProjection());
  ossimKeywordlist kwl;
  objp->saveState(kwl);
  m_RpcProjection->loadState(kwl);
  m_Projection = m_RpcProjection.get();
  
  // Compute errors
  this->ComputeErrors();

  // Build an otb::ImageKeywordList
  ImageKeywordlist otb_kwl;
  otb_kwl.SetKeywordlist(geom_kwl);

  // Encapsulate it
  itk::MetaDataDictionary& dict = imagePtr->GetMetaDataDictionary();
  itk::EncapsulateMetaData< ImageKeywordlist >( dict, MetaDataKey::OSSIMKeywordlistKey, otb_kwl );
}

template < class TImage >
void
GCPsToRPCSensorModelImageFilter< TImage >
::PrintSelf( std::ostream & os, itk::Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os<<indent<<"UseImageGCPs: "<<(m_UseImageGCPs ? "yes" : "no")<<std::endl;
  os<<indent<<"UseDEM: "<<(m_UseDEM ? "yes" : "no")<<std::endl;
  if(!m_UseDEM)
    {
    os<<indent<<"MeanElevation: "<<m_MeanElevation<<std::endl;
    }
  os<<indent<<"RMS ground error: "<<m_RMSGroundError<<std::endl;
}

} // end of namespace otb

#endif


