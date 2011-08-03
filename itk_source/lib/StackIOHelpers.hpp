#ifndef STACKIOHELPERS_HHP_
#define STACKIOHELPERS_HHP_

#include <assert.h>
#include "boost/filesystem.hpp"

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTranslationTransform.h"
#include "itkTransformFactory.h"

#include "Stack.hpp"
#include "StackTransforms.hpp"

using namespace boost::filesystem;

// helper to construct transform file path
const string TransformFilePath(const string& imageFileName, const string& transformDirName) {
  path slicePath( imageFileName );
  string transformFileName( basename( slicePath.leaf() ) + ".meta" );
  path transformFilePath = path(transformDirName) / path(transformFileName);
  return transformFilePath.string();
}

// Stack Persistence
template <typename StackType>
void Save(StackType& stack, vector< string > fileNames, const string& dirName)
{
  typedef itk::TransformFileWriter WriterType;

  for(int slice_number=0; slice_number < stack.GetSize(); ++slice_number)
	{
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( TransformFilePath(fileNames[slice_number], dirName).c_str() );
    writer->AddTransform( stack.GetTransform(slice_number) );
    
    try
    {
    	writer->Update();
    }
  	catch( itk::ExceptionObject & err )
  	{
      cerr << "ExceptionObject caught while saving transforms." << endl;
      cerr << err << endl;
  		std::abort();
  	}
  
  }
  
}


template <typename StackType>
void Load(StackType& stack, vector< string > fileNames, const string& dirName)
{
  typedef itk::TransformFileReader ReaderType;
  
  // Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  // TODO: below registers a new version of transform every time Load() is called
  itk::TransformFactory< itk::TranslationTransform< double, 2 > >::RegisterTransform();
  
  typename StackType::TransformVectorType newTransforms;
  
  for(unsigned int slice_number=0; slice_number<stack.GetSize(); ++slice_number)
  {
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( TransformFilePath(fileNames[slice_number], dirName).c_str() );
    
    try
    {
      reader->Update();
    }
    catch( itk::ExceptionObject & err )
    {
      std::cerr << "ExceptionObject caught while reading transforms." << std::endl;
      cerr << err << endl;
  		std::abort();
    }
    
    typename StackType::TransformType::Pointer transform = static_cast<typename StackType::TransformType*>( reader->GetTransformList()->begin()->GetPointer() );
    newTransforms.push_back( transform );
  }
  
  stack.SetTransforms(newTransforms);
}

template <typename StackType>
void ApplyAdjustments(StackType& stack, vector< string > fileNames, const string& dirName)
{
  typedef itk::TransformFileReader ReaderType;
  
  // Some transforms might not be registered
  // with the factory so we add them manually
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  itk::TransformFactory< itk::TranslationTransform< double, 2 > >::RegisterTransform();
  
  typename StackType::TransformVectorType newTransforms;
  
  for(unsigned int slice_number=0; slice_number<stack.GetSize(); ++slice_number)
  {
    // construct path to config transform file
    // config/Rat28/LoRes_adustments/0053.meta
    string transformFilePath = TransformFilePath(fileNames[slice_number], dirName);
    
    if( exists(transformFilePath) )
    {
      ReaderType::Pointer reader = ReaderType::New();
      reader->SetFileName( transformFilePath.c_str() );
      
      try
      {
        reader->Update();
      }
      catch( itk::ExceptionObject & err )
      {
        std::cerr << "ExceptionObject caught while reading transforms." << std::endl;
        cerr << err << endl;
           std::abort();
      }
      
      // convert Array to Vector
      itk::Array< double > parameters ( reader->GetTransformList()->begin()->GetPointer()->GetParameters() );
      itk::Vector< double, 2 > translation;
      translation[0] = parameters[0];
      translation[1] = parameters[1];
      
      // translate block image
      StackTransforms::Translate<StackType>(stack, translation, slice_number );
    }
  }
}


template <typename StackType>
void saveNumberOfTimesTooBig(StackType& stack, const string& fileName)
{
  const vector< unsigned int >& numbers = stack.GetNumberOfTimesTooBig();
  ofstream outfile(fileName.c_str());
  for(vector< unsigned int >::const_iterator cit = numbers.begin();
    cit != numbers.end(); ++cit)
  {
    outfile << *cit << "\n";
  }
}

template <typename StackType>
void loadNumberOfTimesTooBig(StackType& stack, const string& fileName)
{
  ifstream infile(fileName.c_str());
  assert(infile.is_open());
  unsigned int nottb, nol = 0;
  while ( !infile.eof() )
  {
    infile >> nottb;
    
    // if not e.g. an empty line
    if( !infile.fail() )
    {
      for(unsigned int i=0; i<nottb; ++i)
      {
        stack.ShrinkMaskSlice(nol);
      }
      ++nol;
    }
  }
  assert( nol == stack.GetSize() );
}


#endif
