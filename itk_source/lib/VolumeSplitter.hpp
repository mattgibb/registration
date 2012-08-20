#ifndef VOLUMESPLITTER_HPP_
#define VOLUMESPLITTER_HPP_

#include <assert.h>
#include <iomanip>

#include "boost/program_options.hpp"

#include "itkExtractImageFilter.h"

#include "IOHelpers.hpp"
#include "ScaleImages.hpp"

namespace po = boost::program_options;

template <typename PixelType>
class VolumeSplitter
{
public:
  // typedefs
  typedef itk::Image< PixelType, 3 > VolumeType;
  typedef itk::Image< PixelType, 2 > SliceType;
  typedef itk::ExtractImageFilter< VolumeType, SliceType > SplitterType;
  
  // constructors
  VolumeSplitter(po::variables_map vm);
  
  // add a specific slice number to extract and save
  void AddSlice(unsigned int n);
  
  // extract and save the specified images
  void Split();
  
private:
  void ExtractAndSaveSlice(unsigned int n);
  
  // helper methods
  unsigned int GetNumberOfSlices()
  {
    return m_volume->GetLargestPossibleRegion().GetSize()[GetSliceDimension()];
  }
  
  unsigned int GetSliceDimension()
  {
    return m_vm["sliceDimension"].template as<unsigned int>();
  }
  
  // instance variables
  po::variables_map m_vm;
  typename VolumeType::Pointer m_volume;
  typename SplitterType::Pointer m_splitter;
  vector< unsigned int > m_slices;
  
};

template <typename PixelType>
VolumeSplitter< PixelType >::VolumeSplitter(po::variables_map vm):
  m_vm(vm)
{
  // read volume
  cerr << "Reading volume...";
  m_volume = readImage<VolumeType>(m_vm["inputFile"].template as<string>());
  cerr << "done." << endl;
  
  // shrink image spacings so that latex can fit them on a page
  if(vm["latex"].template as<bool>())
  {
    cerr << "spacings before: " << m_volume->GetSpacing() << endl;
    m_volume->SetSpacing( m_volume->GetSpacing() / 100 );
    cerr << "spacings after:  " << m_volume->GetSpacing() << endl;
  }
  
  // set up splitter
  m_splitter = SplitterType::New();
  m_splitter->SetInput( m_volume );
  m_splitter->SetDirectionCollapseToIdentity();
  
  // add slices if specified
  if(vm.count("slice"))
  {
    AddSlice(vm["slice"].template as<unsigned int>());
  }
}

template <typename PixelType>
void VolumeSplitter< PixelType >::AddSlice(unsigned int n)
{
  assert( n < GetNumberOfSlices() );
  m_slices.push_back(n);
}

template <typename PixelType>
void VolumeSplitter< PixelType >::Split()
{
  // if slices are explicitly specified,
  // only generate those ones
  if(m_slices.size())
  {
    for(vector<unsigned int>::iterator it=m_slices.begin(); it != m_slices.end(); ++it)
    {
      ExtractAndSaveSlice(*it);
    }
  }
  else
  {
    // generate all slices
    for(unsigned int i=0; i<GetNumberOfSlices(); ++i) {
      ExtractAndSaveSlice(i);
    }
  }
}

template <typename PixelType>
void VolumeSplitter< PixelType >::ExtractAndSaveSlice(unsigned int n)
{
  cerr << "slice " << setw(4) << n << ": ";

  // set up extraction region
  typename VolumeType::SizeType size = m_volume->GetLargestPossibleRegion().GetSize();
  size[GetSliceDimension()] = 0;
  
  typename VolumeType::IndexType index = {{0, 0, 0}};
  
  typename VolumeType::RegionType sliceRegion;
  sliceRegion.SetSize( size );
  sliceRegion.SetIndex( index );
  
  // Set the z-coordinate of the slice to be extracted
  sliceRegion.SetIndex(GetSliceDimension(), n);
  
  m_splitter->SetExtractionRegion( sliceRegion );
  
  // split volume
  try
  {
    cerr << "extracting slice...";
    m_splitter->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught while updating volume splitter." << std::endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
  }
  
  // write slice
  stringstream outputFile;
  outputFile << m_vm["outputDir"].template as<string>() << "/"
             // leading zeros
             << setfill('0')
             // 4 digits wide
             << setw(4)
             << n
             << "."
             << m_vm["outputExtension"].template as<string>();
  
  typename SliceType::Pointer outputSlice = m_splitter->GetOutput();
  
  cerr << "writing slice...";
  writeImage< SliceType >(outputSlice, outputFile.str());
  cerr << "done." << endl;
}

#endif
