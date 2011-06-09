#ifndef _DIRS_HPP_
#define _DIRS_HPP_

#include <string>
// Singleton to provide directory information
// The static string _dataSet must be set to a non-empty string
// before the first Instance() call is received

using namespace std;

class Dirs {
  static string _dataSet;
  static string _outputDirName;
  static string _paramsFile;
  static Dirs* _instance;
  
public:
  static string GetDataSet();
  
  static void SetDataSet(const string dataSet);
  
  static void SetOutputDirName(const string outputDirName);
  
  static void SetParamsFile(string paramsFile);
  
  // makes sure _dataSet has been set
  // before returning dependent path strings
  static void CheckDataSet();
  
  // makes sure _resultsDir has been set
  // before returning dependent path strings
  static void CheckOutputDirName();
  
  static Dirs* Instance();
  
  static string ProjectRootDir();
  
  static string ImagesDir();
  
  static string ResultsDir();
  
  static string DTMRIDir();
  
  static string BlockDir();
  
  static string SliceDir();
  
  static string ConfigDir();
  
  static string ParamsFile();
  
  static string SliceFile();
  
  static string TestDir();
  
protected:
  Dirs();
  
};

#endif
