#ifndef WRITERCOMMAND_HPP_
#define WRITERCOMMAND_HPP_

#include <sstream>
#include <iomanip>
#include "boost/filesystem.hpp"

#include "CommandObserverBase.hpp"

#include "StackBase.hpp"

using namespace std;
using namespace boost::filesystem;

class WriterCommand : public CommandObserverBase
{
public:
  // setters
	void setStack(StackBase *stack) { m_stack = stack; }
  
  virtual void setSliceNumber(unsigned int sliceNumber)=0;
  
	void setOutputRootDir(const string& outputRootDir) { m_outputRootDir = outputRootDir; }
  
protected:
    
  const char *transformType()
  {
    return m_stack->GetTransform(m_sliceNumber)->GetNameOfClass();
  }
  
  StackBase *m_stack;
  string m_outputRootDir;
  unsigned int m_sliceNumber;
  
};
#endif
