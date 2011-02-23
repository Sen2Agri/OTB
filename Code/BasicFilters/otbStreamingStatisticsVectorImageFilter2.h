/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.

  Some parts of this code are derived from ITK. See ITKCopyright.txt
  for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANT2ABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbStreamingStatisticsVectorImageFilter2_h
#define __otbStreamingStatisticsVectorImageFilter2_h

#include "otbPersistentImageFilter.h"
#include "otbPersistentFilterStreamingDecorator.h"
#include "itkNumericTraits.h"
#include "itkArray.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkImageRegionSplitter.h"
#include "itkVariableSizeMatrix.h"
#include "itkVariableLengthVector.h"

namespace otb
{

/** \class PersistentStreamingStatisticsVectorImageFilter2
 * \brief Compute covariance & correlation of a large image using streaming
 *
 *  This filter persists its temporary data. It means that if you Update it n times on n different
 * requested regions, the output statistics will be the statitics of the whole set of n regions.
 *
 * To reset the temporary data, one should call the Reset() function.
 *
 * To get the statistics once the regions have been processed via the pipeline, use the Synthetize() method.
 *
 * \sa PersistentImageFilter
 * \ingroup Streamed
 * \ingroup Multithreaded
 * \ingroup MathematicalStatisticsImageFilters
 *
 */
template<class TInputImage, class TPrecision >
class ITK_EXPORT PersistentStreamingStatisticsVectorImageFilter2 :
  public PersistentImageFilter<TInputImage, TInputImage>
{
public:
  /** Standard Self typedef */
  typedef PersistentStreamingStatisticsVectorImageFilter2           Self;
  typedef PersistentImageFilter<TInputImage, TInputImage> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(PersistentStreamingStatisticsVectorImageFilter2, PersistentImageFilter);

  /** Image related typedefs. */
  typedef TInputImage                             ImageType;
  typedef typename TInputImage::Pointer           InputImagePointer;
  typedef typename TInputImage::RegionType        RegionType;
  typedef typename TInputImage::SizeType          SizeType;
  typedef typename TInputImage::IndexType         IndexType;
  typedef typename TInputImage::PixelType         PixelType;
  typedef typename TInputImage::InternalPixelType InternalPixelType;

  typedef TPrecision                              PrecisionType;
  typedef PrecisionType                           RealType;

  itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Type to use for computations. */
  typedef itk::VariableLengthVector<PrecisionType>      RealPixelType;

  /** Smart Pointer type to a DataObject. */
  typedef typename itk::DataObject::Pointer DataObjectPointer;

  /** Type of DataObjects used for scalar outputs */
  typedef itk::VariableSizeMatrix<PrecisionType>        MatrixType;
  typedef itk::SimpleDataObjectDecorator<RealPixelType> RealPixelObjectType;
  typedef itk::SimpleDataObjectDecorator<MatrixType>    MatrixObjectType;

  /** Return the computed Mean. */
  RealPixelType GetMean() const
  {
    return this->GetMeanOutput()->Get();
  }
  RealPixelObjectType* GetMeanOutput();
  const RealPixelObjectType* GetMeanOutput() const;

  /** Return the computed Covariance. */
  MatrixType GetCovariance() const
  {
    return this->GetCovarianceOutput()->Get();
  }
  MatrixObjectType* GetCovarianceOutput();
  const MatrixObjectType* GetCovarianceOutput() const;

  /** Return the computed Covariance. */
  MatrixType GetCorrelation() const
  {
    return this->GetCorrelation()->Get();
  }
  MatrixObjectType* GetCorrelationOutput();
  const MatrixObjectType* GetCorrelationOutput() const;

  /** Make a DataObject of the correct type to be used as the specified
   * output.
   */
  virtual DataObjectPointer MakeOutput(unsigned int idx);

  /** Pass the input through unmodified. Do this by Grafting in the
   *  AllocateOutputs method.
   */
  virtual void AllocateOutputs();
  virtual void GenerateOutputInformation();
  virtual void Synthetize(void);
  virtual void Reset(void);

  itkSetMacro(EnableMean, bool);
  itkGetMacro(EnableMean, bool);

  itkSetMacro(EnableCorrelation, bool);
  itkGetMacro(EnableCorrelation, bool);

  itkSetMacro(EnableCovariance, bool);
  itkGetMacro(EnableCovariance, bool);

protected:
  PersistentStreamingStatisticsVectorImageFilter2();
  virtual ~PersistentStreamingStatisticsVectorImageFilter2() {}
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;
  /** Multi-thread version GenerateData. */
  void  ThreadedGenerateData(const RegionType& outputRegionForThread, int threadId);

private:
  PersistentStreamingStatisticsVectorImageFilter2(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  bool m_EnableMean;
  bool m_EnableCorrelation;
  bool m_EnableCovariance;

  std::vector<RealPixelType> m_FirstOrderAccumulators;
  std::vector<MatrixType>    m_SecondOrderAccumulators;

}; // end of class PersistentStreamingStatisticsVectorImageFilter2

/**===========================================================================*/

/** \class StreamingStatisticsVectorImageFilter2
 * \brief This class streams the whole input image through the PersistentStatisticsImageFilter.
 *
 * This way, it allows to compute the first and second order global statistics of this image. It calls the
 * Reset() method of the PersistentStreamingStatisticsVectorImageFilter2 before streaming the image and the
 * Synthetize() method of the PersistentStreamingStatisticsVectorImageFilter2 after having streamed the image
 * to compute the statistics. The accessor on the results are wrapping the accessors of the
 * internal PersistentStreamingStatisticsVectorImageFilter2.
 *
 * \sa PersistentStreamingStatisticsVectorImageFilter2
 * \sa PersistentImageFilter
 * \sa PersistentFilterStreamingDecorator
 * \sa StreamingImageVirtualWriter
 * \ingroup Streamed
 * \ingroup Multithreaded
 * \ingroup MathematicalStatisticsImageFilters
 */

template<class TInputImage, class TPrecision = typename itk::NumericTraits<typename TInputImage::InternalPixelType>::RealType>
class ITK_EXPORT StreamingStatisticsVectorImageFilter2 :
  public PersistentFilterStreamingDecorator<PersistentStreamingStatisticsVectorImageFilter2<TInputImage, TPrecision> >
{
public:
  /** Standard Self typedef */
  typedef StreamingStatisticsVectorImageFilter2 Self;
  typedef PersistentFilterStreamingDecorator
  <PersistentStreamingStatisticsVectorImageFilter2<TInputImage, TPrecision> > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(StreamingStatisticsVectorImageFilter2, PersistentFilterStreamingDecorator);

  typedef TInputImage                                 InputImageType;
  typedef typename Superclass::FilterType             StatFilterType;

  /** Type of DataObjects used for outputs */
  typedef typename StatFilterType::PixelType           PixelType;
  typedef typename StatFilterType::RealType            RealType;
  typedef typename StatFilterType::RealPixelType       RealPixelType;
  typedef typename StatFilterType::RealPixelObjectType RealPixelObjectType;
  typedef typename StatFilterType::MatrixType          MatrixType;
  typedef typename StatFilterType::MatrixObjectType    MatrixObjectType;

  void SetInput(InputImageType * input)
  {
    this->GetFilter()->SetInput(input);
  }
  const InputImageType * GetInput()
  {
    return this->GetFilter()->GetInput();
  }

  /** Return the computed Mean. */
  RealPixelType GetMean() const
  {
    return this->GetFilter()->GetMeanOutput()->Get();
  }
  RealPixelObjectType* GetMeanOutput()
  {
    return this->GetFilter()->GetMeanOutput();
  }
  const RealPixelObjectType* GetMeanOutput() const
  {
    return this->GetFilter()->GetMeanOutput();
  }

  /** Return the computed Covariance. */
  MatrixType GetCovariance() const
  {
    return this->GetFilter()->GetCovarianceOutput()->Get();
  }
  MatrixObjectType* GetCovarianceOutput()
  {
    return this->GetFilter()->GetCovarianceOutput();
  }
  const MatrixObjectType* GetCovarianceOutput() const
  {
    return this->GetFilter()->GetCovarianceOutput();
  }

  /** Return the computed Covariance. */
  MatrixType GetCorrelation() const
  {
    return this->GetFilter()->GetCorrelationOutput()->Get();
  }
  MatrixObjectType* GetCorrelationOutput()
  {
    return this->GetFilter()->GetCorrelationOutput();
  }
  const MatrixObjectType* GetCorrelationOutput() const
  {
    return this->GetFilter()->GetCorrelationOutput();
  }

  otbSetObjectMemberMacro(Filter, EnableMean, bool);
  otbGetObjectMemberMacro(Filter, EnableMean, bool);

  otbSetObjectMemberMacro(Filter, EnableCorrelation, bool);
  otbGetObjectMemberMacro(Filter, EnableCorrelation, bool);

  otbSetObjectMemberMacro(Filter, EnableCovariance, bool);
  otbGetObjectMemberMacro(Filter, EnableCovariance, bool);

protected:
  /** Constructor */
  StreamingStatisticsVectorImageFilter2() {}

  /** Destructor */
  virtual ~StreamingStatisticsVectorImageFilter2() {}

private:
  StreamingStatisticsVectorImageFilter2(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented
};

} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbStreamingStatisticsVectorImageFilter2.txx"
#endif

#endif
