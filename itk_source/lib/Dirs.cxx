#ifndef _DIRS_CXX_
#define _DIRS_CXX_

#include "Dirs.hpp"
#include <iostream>
#include <stdlib.h>
#include "ProjectRootDir.h"
#include "Parameters.hpp"


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
  return PROJECT_ROOT_DIR;
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
  CheckDataSet();
  string ratio;
  boost::shared_ptr<YAML::Node> downsample_ratios = config(_dataSet + "/downsample_ratios.yml");
  (*(downsample_ratios.get()))["LoRes"] >> ratio;
  return ImagesDir() + "LoRes_rgb/downsamples_" + ratio + "/";
}

string Dirs::SliceDir()
{
  CheckDataSet();
  string ratio;
  boost::shared_ptr<YAML::Node> downsample_ratios = config(_dataSet + "/downsample_ratios.yml");
  (*(downsample_ratios.get()))["HiRes"] >> ratio;
  return ImagesDir() + "HiRes/downsamples_" + ratio + "/";
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
