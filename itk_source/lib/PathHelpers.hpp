#ifndef PATH_HELPERS_HPP_
#define PATH_HELPERS_HPP_

#include "boost/filesystem.hpp"
#include <sys/stat.h> // for fileExists

using namespace std;
using namespace boost::filesystem;


// get list of file names, with no directory, from text list
inline vector < string > getFileNames(const string& fileList, const string& extension = "")
{
  vector< string > fileNames;
  ifstream infile(fileList.c_str(), ios_base::in);
  string fileName;
  
  while (getline(infile, fileName))
  {
    fileNames.push_back( fileName + extension );
  }
  
  return fileNames;
}

// prepend directory to each filename in fileNames,
// possibly adding extension,
// and return vector of results
inline vector< string > constructPaths(const string& directory, const vector< string >& fileNames, const string& extension = "")
{
  vector< string > filePaths;
  
  // use boost filesystem to handle presence/absence of trailing slash on directory
  path directoryPath(directory);
  
  for(vector< string >::const_iterator it = fileNames.begin(); it != fileNames.end(); ++it)
  {
    filePaths.push_back( (directoryPath / *it).string() + extension );
  }
  
  return filePaths;
}

// prepend directory to each line of fileList,
// possibly adding extension,
// and return vector of results
inline vector< string > constructPaths(const string& directory, const string& fileList, const string& extension = "")
{
  vector< string > basenames = getFileNames(fileList);
  
  return constructPaths(directory, basenames, extension);
}


#endif
