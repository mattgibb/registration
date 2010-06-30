#ifndef HELPER_FUNCTIONS_HPP_
#define HELPER_FUNCTIONS_HPP_


using namespace std;

vector< string > getFileNames(string directory, string fileList) {
  vector< string > fileNames;
  string fileName;
  fileNames.clear();
  ifstream infile(fileList.c_str(), ios_base::in);
  while (getline(infile, fileName)) {
    fileNames.push_back(directory + fileName);
  }
  
  return fileNames;
}

template<typename WriterType, typename DataType>
void writeData(typename DataType::Pointer data, string fileName) {
  // typedef itkTransformFileWriter WriterType;
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

template<typename ImageType>
void writeImage(typename ImageType::Pointer image, const string& fileName) {
  writeData< itk::ImageFileWriter< ImageType >, ImageType >( image, fileName );
}

void readRegistrationParameters(YAML::Node & parameters, const string& yamlFile) {
  // TODO: extract filename into ARGV.
  ifstream config_filestream( yamlFile.c_str() );
  YAML::Parser parser(config_filestream);
  parser.GetNextDocument(parameters);
}

#endif