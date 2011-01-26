#ifndef _DIRS_CXX_
#define _DIRS_CXX_

#include "Dirs.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>


string Dirs::_dataSet = "";
string Dirs::_paramsFile = ConfigDir() + "registration_parameters.yml";
Dirs* Dirs::_instance = 0;

void Dirs::SetDataSet(string dataSet)
{
  _dataSet = dataSet;
}

void Dirs::SetParamsFile(string paramsFile)
{
  _paramsFile = paramsFile;
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
  realpath(strcat(FILE, "/../../.."), projectRootDir);
  return string(projectRootDir) + "/";
}

string Dirs::ImagesDir()
{
  CheckDataSet();
  return ProjectRootDir() + "images/" + _dataSet + "/";
}

string Dirs::ResultsDir()
{
  CheckDataSet();
  return ProjectRootDir() + "results/" + _dataSet + "/";
}

string Dirs::DTMRIDir()
{
  return ImagesDir() + "MRI/DTMRI/";
}

string Dirs::BlockDir()
{
  return ImagesDir() + "LoRes/downsamples_8/";
}

string Dirs::SliceDir()
{
  return ImagesDir() + "HiRes/downsamples_64/";
}

string Dirs::SegmentationDir()
{
  return ResultsDir() + "segmentation/";
}

string Dirs::ConfigDir()
{
  return ProjectRootDir() + "config/";
}

string Dirs::ParamsFile()
{
  return _paramsFile;
}

string Dirs::SliceFile()
{
  return ConfigDir() + "picked_files.txt";
}

string Dirs::TestDir()
{
  return ProjectRootDir() + "itk_source/test/";
}

// Constructor
Dirs::Dirs() {}
#endif
