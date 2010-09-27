#ifndef _DIRS_CXX_
#define _DIRS_CXX_

#include "Dirs.hpp"


string Dirs::_dataSet = "";
Dirs* Dirs::_instance = 0;

void Dirs::SetDataSet(string dataSet)
{
  _dataSet = dataSet;
}

Dirs* Dirs::Instance()
{
  if (_instance == 0) {
    // CheckDataSet is only called the first time an instance is requested
    CheckDataSet();
    _instance = new Dirs();
  }
  return _instance;
}

void Dirs::CheckDataSet()
{
  if ( _dataSet.empty())
  {
    cerr << "Dirs::dataSet not set!\n";
    exit(1);
  }
}

string Dirs::ProjectRootDir()
{
  char projectRootDir[1000], FILE[1000] = __FILE__;
  realpath(strcat(FILE, "/../.."), projectRootDir);
  return string(projectRootDir) + "/";
}

string Dirs::DTMRIDir()
{
  return ProjectRootDir() + "images/" + _dataSet + "/MRI/DTMRI/";
}

string Dirs::ResultsDir()
{
  return ProjectRootDir() + "results/" + _dataSet + "/segmentation/";
}

// Constructor
Dirs::Dirs() {}
#endif