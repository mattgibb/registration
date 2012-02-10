#ifndef _DIRS_CXX_
#define _DIRS_CXX_

#include "Dirs.hpp"
#include <iostream>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include "ProjectRootDir.h"
#include "Parameters.hpp"


string Dirs::_dataSet = "";
string Dirs::_outputDirName = "";
string Dirs::_paramsFile = "";

string Dirs::GetDataSet()
{
  CheckDataSet();
  return _dataSet;
}

void Dirs::SetDataSet(const string& dataSet)
{
  _dataSet = dataSet;
}

void Dirs::SetOutputDirName(const string& outputDirName)
{
  _outputDirName = outputDirName;
}

void Dirs::SetParamsFile(const string& paramsFile)
{
  _paramsFile = paramsFile;
}

void Dirs::CheckDataSet()
{
  if ( _dataSet.empty())
  {
    cerr << "Dirs::dataSet not set!\n";
    exit(1);
  }
}

void Dirs::CheckOutputDirName()
{
  if ( _outputDirName.empty())
  {
    cerr << "Dirs::outputDirName not set!\n";
    exit(1);
  }
}

string Dirs::ProjectRootDir()
{
  // return PROJECT_ROOT_DIR;

  using namespace boost::filesystem;
  // path filePath(PROJECT_ROOT_DIR);
  path filePath(__FILE__);
  return filePath.branch_path().branch_path().branch_path().string() + "/";
}

string Dirs::ImagesDir()
{
  CheckDataSet();
  return ProjectRootDir() + "images/" + _dataSet + "/";
}

string Dirs::ResultsDir()
{
  CheckDataSet();
  return ProjectRootDir() + "results/" + _dataSet + "/" + _outputDirName + "/";
}

string Dirs::LoResTransformsDir()
{
  // read transforms from directories labeled by both ds ratios
  return ResultsDir() + "LoResTransforms_" + DownsampleSuffix() + "/";
}

string Dirs::HiResTransformsDir()
{
  // read transforms from directories labeled by both ds ratios
  return ResultsDir() + "HiResTransforms_" + DownsampleSuffix() + "/";
}

string Dirs::IntermediateTransformsDir()
{
  // read transforms from directories labeled by both ds ratios
  return ResultsDir() + "IntermediateTransforms_" + DownsampleSuffix() + "/";
}

string Dirs::HiResPairTransformsDir()
{
  // read transforms from directories labeled by both ds ratios
  return ResultsDir() + "HiResPairTransforms_" + DownsampleSuffix() + "/";
}

string Dirs::ColourDir()
{
  return ResultsDir() + "ColourResamples_" + DownsampleSuffix() + "/";
}

string Dirs::BlockDir()
{
  CheckDataSet();
  string ratio;
  boost::shared_ptr<YAML::Node> downsample_ratios = config("downsample_ratios.yml");
  (*(downsample_ratios.get()))["LoRes"] >> ratio;
  return ImagesDir() + "LoRes_rgb/downsamples_" + ratio + "/";
}

string Dirs::SliceDir()
{
  CheckDataSet();
  string ratio;
  boost::shared_ptr<YAML::Node> downsample_ratios = config("downsample_ratios.yml");
  (*(downsample_ratios.get()))["HiRes"] >> ratio;
  return ImagesDir() + "HiRes/downsamples_" + ratio + "/";
}

string Dirs::ConfigDir()
{
  return ProjectRootDir() + "config/" + GetDataSet() + "/";
}

string Dirs::ParamsFile()
{
  if(!_paramsFile.empty()) return _paramsFile;
  CheckDataSet();
  return ConfigDir() + "registration_parameters.yml";
}

string Dirs::ImageList()
{
  return ConfigDir() + "image_lists/image_list.txt";
}

string Dirs::TestDir()
{
  return ProjectRootDir() + "itk_source/test/";
}

string Dirs::DownsampleSuffix()
{
  // get downsample ratios
  boost::shared_ptr<YAML::Node> downsample_ratios = config("/downsample_ratios.yml");
  string LoResDownsampleRatio, HiResDownsampleRatio;
  (*downsample_ratios)["LoRes"] >> LoResDownsampleRatio;
  (*downsample_ratios)["HiRes"] >> HiResDownsampleRatio;
  
  // read transforms from directories labeled by both ds ratios
  return LoResDownsampleRatio + "_" + HiResDownsampleRatio;
}

// Constructor
Dirs::Dirs() {}

#endif

