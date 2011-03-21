#ifndef STACKIOHELPERS_HHP_
#define STACKIOHELPERS_HHP_

#include <assert.h>

#include "Stack.hpp"

// Stack Persistence
void saveNumberOfTimesTooBig(Stack& stack, const string& fileName)
{
  const vector< unsigned int >& numbers = stack.GetNumberOfTimesTooBig();
  ofstream outfile(fileName.c_str());
  for(vector< unsigned int >::const_iterator cit = numbers.begin();
    cit != numbers.end(); ++cit)
  {
    outfile << *cit << "\n";
  }
}

void loadNumberOfTimesTooBig(Stack& stack, const string& fileName)
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
