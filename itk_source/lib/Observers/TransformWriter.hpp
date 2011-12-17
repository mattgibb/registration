#ifndef TRANSFORMWRITER_HPP_
#define TRANSFORMWRITER_HPP_

#include <sstream>
#include "boost/filesystem.hpp"

#include "CommandObserverBase.hpp"
#include "itkGradientDescentOptimizer.h"
#include "itkRegularStepGradientDescentOptimizer.h"

#include "Dirs.hpp"
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
  
  virtual void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      return;
    
    setCurrentIteration(object);
    
    saveTransform();
  }
  
  // setters
	void setStack(StackBase *stack) { m_stack = stack; }
  
	void setSliceNumber(unsigned int sliceNumber) { m_sliceNumber = sliceNumber; }
  
	protected:
    void setCurrentIteration(const itk::Object * object)
    {
      typedef itk::RegularStepGradientDescentOptimizer RSGD;
      typedef itk::GradientDescentOptimizer GD;
      
      // optimizer base class does not have GetCurrentIteration method,
      // so runtime check to extract current iteration
      if(const GD* gd = dynamic_cast< const GD* >( object ))
        m_currentIteration = gd->GetCurrentIteration();
      if(const RSGD * rsgd = dynamic_cast< const RSGD* >( object ))
        m_currentIteration = rsgd->GetCurrentIteration();
    }
    
    void saveTransform()
    {
      // construct paths
      string dirPath = Dirs::IntermediateTransformsDir() +
                       transformType() + "/" +
                       m_stack->GetBasename(m_sliceNumber) + "/";
                       
      create_directories(dirPath);
      stringstream filePath;
      filePath << dirPath << m_currentIteration;
      
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
    unsigned long m_currentIteration;
	
};
#endif
