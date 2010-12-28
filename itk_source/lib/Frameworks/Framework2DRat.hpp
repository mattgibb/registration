// This object constructs and encapsulates the rat registration framework
// of HiRes to LoRes images.

#ifndef FRAMEWORK2DRAT_HPP_
#define FRAMEWORK2DRAT_HPP_

// my files
#include "Stack.hpp"
#include "Framework2DBase.hpp"


class Framework2DRat : public Framework2DBase {
public:
	
	Stack *LoResStack, *HiResStack;
	
	Framework2DRat(Stack *LoRes, Stack *HiRes):
	Framework2DBase(),
	LoResStack(LoRes),
	HiResStack(HiRes) {}
	
void StartRegistration();
	
protected:
  bool bothImagesExist(unsigned int slice_number);
  
  bool tryRegistration();
  
};

#endif
