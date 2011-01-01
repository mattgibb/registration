#ifndef IO_HELPERS_HPP_
#define IO_HELPERS_HPP_

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"


using namespace std;

inline vector< string > getFileNames(const string& directory, const string& fileList) {
  vector< string > fileNames;
  fileNames.clear();
  ifstream infile(fileList.c_str(), ios_base::in);
  string fileName;
  while (getline(infile, fileName)) {
    fileNames.push_back(directory + fileName);
  }
  
  return fileNames;
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