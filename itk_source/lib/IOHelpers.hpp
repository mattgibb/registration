#ifndef IO_HELPERS_HPP_
#define IO_HELPERS_HPP_

#include <sys/stat.h> // for fileExists
#include "boost/filesystem.hpp"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"

using namespace std;
using namespace boost::filesystem;

// get list of file names, with no directory, from text list
inline vector < string > getFileNames(const string& fileList)
{
  vector< string > fileNames;
  ifstream infile(fileList.c_str(), ios_base::in);
  string fileName;
  
  while (getline(infile, fileName))
  {
    fileNames.push_back( fileName );
  }
  
  return fileNames;
}

// prepend directory to each filename in fileList and return vector of results
inline vector< string > getFilePaths(const string& directory, const string& fileList) {
  // use boost filesystem to handle presence/absence of trailing slash on directory
  path directoryPath(directory);
  
  vector< string > fileNames = getFileNames(fileList), filePaths;
  for(vector< string >::iterator it = fileNames.begin(); it != fileNames.end(); ++it)
  {
    filePaths.push_back( (directoryPath / *it).string() );
  }
  
  return filePaths;
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
    // We were not able to get the file attributes. 
    // This may mean that we don't have permission to 
    // access the folder which contains this file. If you 
    // need to do that level of checking, lookup the 
    // return values of stat which will give you 
    // more details on why stat failed.
    blnReturn = false; 
  } 
  
  return(blnReturn); 
}


template <typename StackType>
typename StackType::SliceVectorType readImages(vector< string > fileNames)
{
  typename StackType::SliceVectorType originalImages;
  typedef itk::ImageFileReader< typename StackType::SliceType > ReaderType;
  typename ReaderType::Pointer reader;
	
	for(unsigned int i=0; i<fileNames.size(); i++)
	{
	  if( fileExists(fileNames[i]) )
	  {
			reader = ReaderType::New();
			reader->SetFileName( fileNames[i] );
			reader->Update();
			originalImages.push_back( reader->GetOutput() );
			originalImages.back()->DisconnectPipeline();
	  }
	  else
	  {
	    // create a new image of zero size
      originalImages.push_back( StackType::SliceType::New() );
	  }
	}
	
  return originalImages;
}


// Reader helper
template<typename ImageType>
typename ImageType::Pointer readImage(const string& fileName) {
  typedef itk::ImageFileReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
	
  reader->SetFileName( fileName.c_str() );
	
  try {
  	reader->Update();
  }
	catch( itk::ExceptionObject & err ) {
    cerr << "ExceptionObject caught !" << endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
	}
	
  return reader->GetOutput();
}

// Writer helpers
// const Data
template<typename WriterType, typename DataType>
void writeData(const typename DataType::ConstPointer data, const string& fileName) {
  typename WriterType::Pointer writer = WriterType::New();
	
	writer->SetInput( data );
  // writer->AddTransform( anotherTransform ); // only applies to writing transforms
  
  writer->SetFileName( fileName.c_str() );
	
  try {
  	writer->Update();
  }
	catch( itk::ExceptionObject & err ) {
    cerr << "ExceptionObject caught !" << endl;
    cerr << err << endl;
		exit(EXIT_FAILURE);
	}
}

// Data
template<typename WriterType, typename DataType>
void writeData(const typename DataType::Pointer data, const string& fileName) {
  writeData< WriterType, DataType >( (typename DataType::ConstPointer) data, fileName);
}

// Const Image
template<typename ImageType>
void writeImage(const typename ImageType::ConstPointer image, const string& fileName) {
  writeData< itk::ImageFileWriter< ImageType >, ImageType >( image, fileName );
}

// Image
template<typename ImageType>
void writeImage(const typename ImageType::Pointer image, const string& fileName) {
  writeData< itk::ImageFileWriter< ImageType >, ImageType >( image, fileName );
}


#endif
