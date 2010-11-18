#ifndef _DIRS_HPP_
#define _DIRS_HPP_

#include <string>
#include <iostream>
// Singleton to provide directory information
// The static string _dataSet must be set to a non-empty string
// before the first Instance() call is received

using namespace std;

class Dirs {  
public:
  static void SetDataSet(string dataSet);
  
  // makes sure DataSet has been set
  // before returning dependent path strings
  static void CheckDataSet();
  
  static Dirs* Instance();
  
  string ProjectRootDir();
  
  string DTMRIDir();
  
  string ResultsDir();
  
  string ParamsFile();
  
protected:
  Dirs();
  
private:
  static string _dataSet;
  static Dirs* _instance;
};

#endif