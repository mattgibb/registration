#ifndef TRANSFORMWRITER_HPP_
#define TRANSFORMWRITER_HPP_

#include <sstream>
#include <iomanip>
#include "boost/filesystem.hpp"

#include "CommandObserverBase.hpp"

#include "StackBase.hpp"
#include "IOHelpers.hpp"

using namespace std;
using namespace boost::filesystem;

class TransformWriter : public CommandObserverBase
{
public:
  typedef TransformWriter                  Self;
  typedef CommandObserverBase              Superclass;
  typedef itk::SmartPointer<Self>          Pointer;
  
  itkNewMacro( Self );
  
  virtual void run()
  {
    saveTransform();
  }
  
  // setters
	void setStack(StackBase *stack) { m_stack = stack; }
  
	void setSliceNumber(unsigned int sliceNumber) { m_sliceNumber = sliceNumber; }
  
	void setOutputRootDir(const string& outputRootDir) { m_outputRootDir = outputRootDir; }

	protected:
    
    void saveTransform()
    {
      // construct paths
      string dirPath = m_outputRootDir +
                       transformType() + "/" +
                       m_stack->GetBasename(m_sliceNumber) + "/";
                       
      create_directories(dirPath);
      stringstream filePath;
      filePath << dirPath
        // leading zeros
        << setfill('0')
        // 4 digits wide
        << setw(4)
        << m_iteration;
      
      // save transform
      writeTransform(m_stack->GetTransform(m_sliceNumber), filePath.str());
    }
    
    const char *transformType()
    {
      return m_stack->GetTransform(m_sliceNumber)->GetNameOfClass();
    }
    
  private:
    StackBase *m_stack;
    unsigned int m_sliceNumber;
    string m_outputRootDir;
	  
};
#endif
