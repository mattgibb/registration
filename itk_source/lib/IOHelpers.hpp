#ifndef IO_HELPERS_HPP_
#define IO_HELPERS_HPP_

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"

#include "PathHelpers.hpp"

using namespace std;

// Reader helpers
itk::TransformBase::Pointer readTransform(const string& fileName)
{
  typedef itk::TransformFileReader ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( fileName.c_str() );
  
  try
  {
    reader->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught while reading transform." << std::endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
  }
  
  return *(reader->GetTransformList()->begin());
}

template<typename ImageType>
typename ImageType::Pointer readImage(const string& fileName)
{
  typedef itk::ImageFileReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
	
  reader->SetFileName( fileName.c_str() );
	
  try {
  	reader->Update();
  }
	catch( itk::ExceptionObject & err ) {
    cerr << "ExceptionObject caught while reading image." << endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
	}
	
  return reader->GetOutput();
}

// Writer helpers
// const Data
template<typename WriterType, typename DataType>
void writeData(const typename DataType::ConstPointer data, const string& fileName)
{
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( data );
  // writer->AddTransform( anotherTransform ); // only applies to writing transforms
  
  writer->SetFileName( fileName.c_str() );
	
  try {
  	writer->Update();
  }
	catch( itk::ExceptionObject & err ) {
    cerr << "ExceptionObject caught while writing." << endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
	}
}

// Data
template<typename WriterType, typename DataType>
void writeData(const typename DataType::Pointer data, const string& fileName)
{
  writeData< WriterType, DataType >( (typename DataType::ConstPointer) data, fileName);
}

// Const Image
template<typename ImageType>
void writeImage(const typename ImageType::ConstPointer image, const string& fileName)
{
  writeData< itk::ImageFileWriter< ImageType >, ImageType >( image, fileName );
}

// Image
template<typename ImageType>
void writeImage(const typename ImageType::Pointer image, const string& fileName)
{
  writeData< itk::ImageFileWriter< ImageType >, ImageType >( image, fileName );
}

// Transform
void writeTransform(const itk::TransformBase *transform, const string& fileName)
{
  itk::TransformBase::ConstPointer transformConstPointer(transform);
  writeData< itk::TransformFileWriter, itk::TransformBase >( transformConstPointer, fileName );
}

bool fileExists(const string& strFilename)
{
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;
  
  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes 
    // so the file obviously exists.
    blnReturn = true; 
  } else { 
    blnReturn = false; 
  } 
  
  return(blnReturn); 
}

template <typename ImageType>
vector< typename ImageType::Pointer > readImages(vector< string > fileNames)
{
  vector< typename ImageType::Pointer > originalImages;
	
	for(unsigned int i=0; i<fileNames.size(); i++)
	{
	  if( fileExists(fileNames[i]) )
	  {
      originalImages.push_back( readImage< ImageType >(fileNames[i]) );
	  }
	  else
	  {
	    // create a new image of zero size
      originalImages.push_back( ImageType::New() );
	  }
	}
	
  return originalImages;
}


#endif
