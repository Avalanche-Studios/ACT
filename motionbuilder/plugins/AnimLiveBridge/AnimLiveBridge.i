%module AnimLiveBridge

%include "exception.i"

%{
	#define SWIG_FILE_WITH_INIT
	#include <assert.h>
    #include "AnimLiveBridge.h"
	static int lastErr = 0; // flag to save test struct error state
%}

%feature("autodoc", "1");

%include "std_vector.i"

%template(SJointDataVector) std::vector<SJointData>;
%template(SPropertyDataVector) std::vector<SPropertyData>;

%include "AnimLiveBridge.h"
%include "carrays.i"
%array_class(float, FloatArray);
%array_class(unsigned int, UIntArray);

// set exception handling for __getitem__
%exception SJointDataArray::__getitem__ {
  assert(!lastErr);
  $action
  if ( lastErr ){
    lastErr = 0; // clear flag for next time
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
}

// set exception handling for __setitem__
%exception SJointDataArray::__setitem__ {
  assert(!lastErr);
  $action
  if ( lastErr ){
    lastErr = 0; // clear flag for next time
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
}

// set exception handling for insert()
%exception SJointDataArray::insert {
  assert(!lastErr);
  $action
  if ( lastErr ){
    lastErr = 0; // clear flag for next time
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
}

//
//

// set exception handling for __getitem__
%exception SPropertyDataArray::__getitem__ {
  assert(!lastErr);
  $action
  if ( lastErr ){
    lastErr = 0; // clear flag for next time
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
}

// set exception handling for __setitem__
%exception SPropertyDataArray::__setitem__ {
  assert(!lastErr);
  $action
  if ( lastErr ){
    lastErr = 0; // clear flag for next time
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
}

// set exception handling for insert()
%exception SPropertyDataArray::insert {
  assert(!lastErr);
  $action
  if ( lastErr ){
    lastErr = 0; // clear flag for next time
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
}

// "extend" the structure with various methods
%extend SJointDataArray{
  // add a __getitem__ method to the structure to get values from the data array
  SJointData __getitem__(size_t i) {
    if (i >= NUMBER_OF_JOINTS) {
      lastErr = 1;
      return SJointData();
    }
    return $self->m_Data[i];
  }

  // add a __setitem__ method to the structure to set values in the data array
  void __setitem__(size_t i, SJointData value) {
    if ( i >= NUMBER_OF_JOINTS ){
      lastErr = 1;
      return;
    }
    $self->m_Data[i] = value;  
  }

  
  size_t __len__(){
    return NUMBER_OF_JOINTS;
  }

  void insert(size_t i, SJointData value) {
    if ( i >= NUMBER_OF_JOINTS ){
      lastErr = 1;
      return;
    }
    $self->m_Data[i] = value;
  }
}

// "extend" the structure with various methods
%extend SPropertyDataArray{
  // add a __getitem__ method to the structure to get values from the data array
  SPropertyData __getitem__(size_t i) {
    if (i >= NUMBER_OF_PROPERTIES) {
      lastErr = 1;
      return SPropertyData();
    }
    return $self->m_Data[i];
  }

  // add a __setitem__ method to the structure to set values in the data array
  void __setitem__(size_t i, SPropertyData value) {
    if ( i >= NUMBER_OF_PROPERTIES ){
      lastErr = 1;
      return;
    }
    $self->m_Data[i] = value;  
  }

  
  size_t __len__(){
    return NUMBER_OF_PROPERTIES;
  }

  void insert(size_t i, SPropertyData value) {
    if ( i >= NUMBER_OF_PROPERTIES ){
      lastErr = 1;
      return;
    }
    $self->m_Data[i] = value;
  }
}