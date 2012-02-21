// Static methods to provide directory information
#ifndef _DIRS_HPP_
#define _DIRS_HPP_

#include <string>

using namespace std;

class Dirs {
  static string _dataSet;
  static string _outputDirName;
  static string _paramsFile;
  
public:
  static string GetDataSet();
  
  static void SetDataSet(const string& dataSet);
  
  static void SetOutputDirName(const string& outputDirName);
  
  static void SetParamsFile(const string& paramsFile);
  
  // makes sure _dataSet has been set
  // before returning dependent path strings
  static void CheckDataSet();
  
  // makes sure _resultsDir has been set
  // before returning dependent path strings
  static void CheckOutputDirName();
  
  static string ProjectRootDir();
  
  static string ImagesDir();
  
  static string ResultsDir();
  
  static string LoResTransformsDir();
  
  static string HiResTransformsDir();
  
  static string IntermediateTransformsDir();
  
  static string ColourDir();
  
  static string BlockDir();
  
  static string SliceDir();
  
  static string ConfigDir();
  
  static string ParamsFile();
  
  static string ImageList();
  
  static string TestDir();
  
  // Helper to extract the downsample ratios from the config
  // and build a string out of them
  static string DownsampleSuffix();
  
protected:
  Dirs();
};

#endif

