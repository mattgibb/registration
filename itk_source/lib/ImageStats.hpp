#ifndef IMAGE_STATS_HPP_
#define IMAGE_STATS_HPP_

#include "itkStatisticsImageFilter.h"

using namespace std;

template<class TImage>
void printImageStats(typename TImage::Pointer image)
{
  typedef itk::StatisticsImageFilter< TImage > StatsType;
  typename StatsType::Pointer stats = StatsType::New();
  stats->SetInput( image );
  try 
  { stats->Update(); } 
  catch( itk::ExceptionObject & err ) 
  { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    exit(EXIT_FAILURE);
  }
  
  // Show statistics
  cerr << "Minimum:\n" << stats->GetMinimum() + 0 << endl << endl;
  cerr << "Maximum:\n" << stats->GetMaximum() + 0 << endl << endl;
  cerr << "Mean:\n" << stats->GetMean() << endl << endl;
  cerr << "Sigma:\n" << stats->GetSigma() << endl << endl;
  cerr << "Variance:\n" << stats->GetVariance() << endl << endl;
  cerr << "Sum:\n" << stats->GetSum() << endl << endl;
  cerr << "Size:\n" << image->GetLargestPossibleRegion().GetSize() << endl << endl;
  cerr << "Image:\n" << stats->GetOutput() << endl << endl;
  
}

#endif
