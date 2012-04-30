#ifndef IO_HELPERS_HPP_
#define IO_HELPERS_HPP_

#include <sys/stat.h> // for fileExists
#include "boost/filesystem.hpp"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkTxtTransformIO.h"

#include "PathHelpers.hpp"

using namespace std;
typedef itk::TxtTransformIO TransformIOType;


vector< string > directoryContents(const string& directory)
{
  using namespace boost::filesystem;
  
  // retrieve and sort paths
  vector< path > contents;
  try
  {
    copy(directory_iterator(directory), directory_iterator(), back_inserter(contents));
  }
  catch(std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  sort(contents.begin(), contents.end());
  
  // extract leaves
  vector< string > contents_strings;
  for(vector< path >::const_iterator it = contents.begin(); it != contents.end(); ++it)
  {
    contents_strings.push_back(it->leaf().string());
  }
  
  return contents_strings;
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

// Transform helpers
// specific IO is picked rather than using factory
// because we need TxtTransformIO, regardless of file extension
itk::TransformBase::Pointer readTransform(const string& fileName)
{
  TransformIOType::Pointer transformIO = TransformIOType::New();
  transformIO->SetFileName(fileName);
  
  try
  {
    transformIO->Read();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught while reading transform." << std::endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
  }
  
  return *(transformIO->GetTransformList().begin());
}

void writeTransform(const itk::TransformBase *transform, const string& fileName)
{
  TransformIOType::Pointer transformIO = TransformIOType::New();
  transformIO->SetAppendMode(false);
  transformIO->SetFileName(fileName);
  
  TransformIOType::ConstTransformListType transformList(1, TransformIOType::ConstTransformPointer(transform));
  transformIO->SetTransformList(transformList);
  
  try
  {
    transformIO->Write();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught while writing transform." << std::endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
  }
}

// Image helpers
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

// Const Image
template<typename ImageType>
void writeImage(const typename ImageType::ConstPointer image, const string& fileName)
{
  typedef itk::ImageFileWriter< ImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( image );
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

// Image
template<typename ImageType>
void writeImage(const typename ImageType::Pointer image, const string& fileName)
{
  writeImage< ImageType >( (typename ImageType::ConstPointer) image, fileName);
}

#endif
