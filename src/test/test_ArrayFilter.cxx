// $Id$

/*!
  \file 
  \ingroup test
 
  \brief tests for the ArrayFilter classes

  \author Kris Thielemans

  $Date$
  $Revision$

*/
/*
    Copyright (C) 2004- $Date$, Hammersmith Imanet Ltd
    See STIR/LICENSE.txt for details
*/

#include "stir/Array.h"
#include "stir/ArrayFilterUsingRealDFTWithPadding.h"
#include "stir/ArrayFilter1DUsingConvolution.h"
#include "stir/ArrayFilter1DUsingConvolutionSymmetricKernel.h"
#ifdef STIR_DEVEL
#include "local/stir/ArrayFilter3DUsingConvolution.h"
#include "stir/IndexRange3D.h"
#endif
#include "stir/Succeeded.h"
#include "stir/modulo.h"
#include "stir/RunTests.h"

//#include "stir/stream.h"
#include <iostream>
#include <algorithm>

#ifdef DO_TIMINGS
#include "stir/CPUTimer.h"
#endif

START_NAMESPACE_STIR


/*!
  \brief Tests Array functionality
  \ingroup test

*/
class ArrayFilterTests : public RunTests
{
public:
  void run_tests();
private:

  void compare_results_2arg(const ArrayFunctionObject<1,float>& filter1,
			    const ArrayFunctionObject<1,float>& filter2,
			    const Array<1,float>& test);
  template <int num_dimensions>
  void compare_results_1arg(const ArrayFunctionObject<num_dimensions,float>& filter1,
			    const ArrayFunctionObject<num_dimensions,float>& filter2,
			    const Array<num_dimensions,float>& test);
};


template <int num_dimensions>
void 
ArrayFilterTests::
compare_results_1arg(const ArrayFunctionObject<num_dimensions,float>& filter1,
		     const ArrayFunctionObject<num_dimensions,float>& filter2,
		     const Array<num_dimensions,float>& test)
{
  {
    Array<num_dimensions,float> out1(test);
    Array<num_dimensions,float> out2(out1);
    filter1(out1);
    filter2(out2);
    
    check_if_equal( out1, out2, "test comparing output of filters, equal length");
    //std::cerr << out1 << out2;
  }
  {
    Array<num_dimensions,float> out1(test);
    BasicCoordinate<num_dimensions, int> min_indices, max_indices;
    check(test.get_regular_range(min_indices, max_indices), "test only works for Arrays of regular range");
    const IndexRange<num_dimensions> larger_range(min_indices-5, max_indices+7);
    out1.resize(larger_range);
    Array<num_dimensions,float> out2(out1);
    filter1(out1);
    filter2(out2);
    
    check_if_equal( out1, out2, "test comparing output of filters, larger length");
    //std::cerr << out1 << out2;
  }
}

void 
ArrayFilterTests::
compare_results_2arg(const ArrayFunctionObject<1,float>& filter1,
		     const ArrayFunctionObject<1,float>& filter2,
		     const Array<1,float>& test)
{
  {
    Array<1,float> out1(test.get_index_range());
    Array<1,float> out2(out1.get_index_range());
    filter1(out1, test);
    filter2(out2, test);
    
    check_if_equal( out1, out2, "test comparing output of filter2, equal length");
    //std::cerr << out1 << out2;
  }
  {
    Array<1,float> out1(IndexRange<1>(200));
    Array<1,float> out2(out1.get_index_range());
    filter1(out1, test);
    filter2(out2, test);
    
    check_if_equal( out1, out2, "test comparing output of filter2, larger length");
    //std::cerr << out1 << out2;
  }
  {
    Array<1,float> out1(IndexRange<1>(50));
    Array<1,float> out2(out1.get_index_range());
    filter1(out1, test);
    filter2(out2, test);
    
    check_if_equal( out1, out2, "test comparing output of filters, smaller length");
  }
  {
    IndexRange<1> influenced_range;
    if (filter2.get_influenced_indices(influenced_range, test.get_index_range())==Succeeded::yes)
      {
	Array<1,float> out1(IndexRange<1>(influenced_range.get_min_index()-3, influenced_range.get_max_index()+4));
	Array<1,float> out2(out1.get_index_range());
	filter1(out1, test);
	filter2(out2, test);
    
	check_if_equal( out1, out2, "test comparing output of filters, out range is in range+ kernel + extra");
	check_if_zero( out2[out2.get_min_index()], "test conv 0 beyond kernel length");
	check_if_zero( out2[out2.get_min_index()+1], "test conv 0 beyond kernel length");
	check_if_zero( out2[out2.get_min_index()+2], "test conv 0 beyond kernel length");
	check_if_zero( out2[out2.get_max_index()], "test conv 0 beyond kernel length");
	check_if_zero( out2[out2.get_max_index()-1], "test conv 0 beyond kernel length");
	check_if_zero( out2[out2.get_max_index()-2], "test conv 0 beyond kernel length");
	check_if_zero( out2[out2.get_max_index()-3], "test conv 0 beyond kernel length");

	// really not necessary if above tests were ok,
	// but in case they failed, this gives some extra info
	check_if_zero( out1[out1.get_min_index()], "test DFT 0 beyond kernel length");
	check_if_zero( out1[out1.get_min_index()+1], "test DFT 0 beyond kernel length");
	check_if_zero( out1[out1.get_min_index()+2], "test DFT 0 beyond kernel length");
	check_if_zero( out1[out1.get_max_index()], "test DFT 0 beyond kernel length");
	check_if_zero( out1[out1.get_max_index()-1], "test DFT 0 beyond kernel length");
	check_if_zero( out1[out1.get_max_index()-2], "test DFT 0 beyond kernel length");
	check_if_zero( out1[out1.get_max_index()-3], "test DFT 0 beyond kernel length");
	//std::cerr << out1 << out2;
      }
  }
}

void
ArrayFilterTests::run_tests()
{ 
  // 1D
  {
    Array<1,float> test(IndexRange<1>(100));
    Array<1,float> test_neg_offset(IndexRange<1>(-10,89));
    Array<1,float> test_pos_offset(IndexRange<1>(10,109));
    // initialise to some arbitrary values
    for (int i=test.get_min_index(); i<=test.get_max_index(); ++i)
      test[i]=i*i*2-i-100.F;
    std::copy(test.begin(), test.end(), test_neg_offset.begin());
    std::copy(test.begin(), test.end(), test_pos_offset.begin());

    {
      const int kernel_half_length=30;
      const int DFT_kernel_size=256;
      // necessary for avoid aliasing in DFT
      assert(DFT_kernel_size>=kernel_half_length*2*2);
      Array<1,float> kernel_for_DFT(IndexRange<1>(0,DFT_kernel_size-1));
      Array<1,float> kernel_for_conv(IndexRange<1>(-kernel_half_length,kernel_half_length));
      for (int i=-kernel_half_length; i<kernel_half_length; ++i)
	{
	  kernel_for_conv[i] = i*i-3*i+1.F;
	  kernel_for_DFT[modulo(i,DFT_kernel_size)] =
	    kernel_for_conv[i];
	}
  
  
      ArrayFilterUsingRealDFTWithPadding<1,float> DFT_filter;
      check(DFT_filter.set_kernel(kernel_for_DFT)==Succeeded::yes, "initialisation DFT filter");
      ArrayFilter1DUsingConvolution<float> conv_filter(kernel_for_conv);

      check(!DFT_filter.is_trivial(), "DFT is_trivial");
      check(!conv_filter.is_trivial(), "conv is_trivial");
      set_tolerance(test.find_max()*kernel_for_conv.sum()*1.E-6);
      //std::cerr << get_tolerance();

      cerr <<"Comparing DFT and Convolution with input offset 0\n";
      compare_results_2arg(DFT_filter, conv_filter, test);
      compare_results_1arg(DFT_filter, conv_filter, test);
      cerr <<"Comparing DFT and Convolution with input negative offset\n";
      compare_results_2arg(DFT_filter, conv_filter, test_neg_offset);
      compare_results_1arg(DFT_filter, conv_filter, test_neg_offset);
      cerr <<"Comparing DFT and Convolution with input positive offset\n";
      compare_results_2arg(DFT_filter, conv_filter, test_pos_offset);
      compare_results_1arg(DFT_filter, conv_filter, test_pos_offset);
    }
    {
      const int kernel_half_length=30;
      Array<1,float> kernel_for_symconv(IndexRange<1>(0,kernel_half_length));
      Array<1,float> kernel_for_conv(IndexRange<1>(-kernel_half_length,kernel_half_length));
      for (int i=0; i<kernel_half_length; ++i)
	{
	  kernel_for_symconv[i] =
	    kernel_for_conv[i] = 
	    kernel_for_conv[-i] = i*i-3*i+1.F;
	}

      // symmetric convolution currently requires equal in and out range
      Array<1,float> test(IndexRange<1>(100));
      // initialise to some arbitrary values
      for (int i=test.get_min_index(); i<=test.get_max_index(); ++i)
	test[i]=i*i*2-i-100.F;
    
  
  
      ArrayFilter1DUsingConvolution<float> conv_filter(kernel_for_conv);
      ArrayFilter1DUsingConvolutionSymmetricKernel<float> symconv_filter(kernel_for_symconv);

      check(!symconv_filter.is_trivial(), "symconv is_trivial");
      check(!conv_filter.is_trivial(), "conv is_trivial");
      set_tolerance(test.find_max()*kernel_for_conv.sum()*1.E-6);
      cerr <<"Comparing SymmetricConvolution and Convolution\n";
      // note: SymmetricConvolution cannot handle different input and output ranges
      compare_results_1arg(symconv_filter, conv_filter, test);
    }
  } // 1D

#ifdef STIR_DEVEL
  // 3D
  {
    Array<3,float> test(IndexRange3D(10,21,30));
    Array<3,float> test_neg_offset(IndexRange3D(-5,4,-10,10,-4,25));
    Array<3,float> test_pos_offset(IndexRange3D(1,11,2,23,3,33));
    // initialise to some arbitrary values
    {
      Array<3,float>::full_iterator iter = test.begin_all();
      for (int i=-100; iter != test.end_all(); ++i, ++iter)
      *iter = i*i*2.F-i-100.F;
      std::copy(test.begin_all(), test.end_all(), test_neg_offset.begin_all());
      std::copy(test.begin_all(), test.end_all(), test_pos_offset.begin_all());
    }
    {
      const int kernel_half_length=10;
      const int DFT_kernel_size=64;
      // necessary for avoid aliasing in DFT
      assert(DFT_kernel_size>=kernel_half_length*2*2);
      const Coordinate3D<int> sizes(DFT_kernel_size/2,DFT_kernel_size,DFT_kernel_size);
      Array<3,float> kernel_for_DFT(IndexRange3D(DFT_kernel_size/2,DFT_kernel_size,DFT_kernel_size));
      Array<3,float> kernel_for_conv(IndexRange3D(-(kernel_half_length/2),kernel_half_length/2,
						  -kernel_half_length,kernel_half_length,
						  -kernel_half_length,kernel_half_length));
      for (int i=-(kernel_half_length/2); i<kernel_half_length/2; ++i)
	for (int j=-kernel_half_length; j<kernel_half_length; ++j)
	  for (int k=-kernel_half_length; k<kernel_half_length; ++k)
	{
	  Coordinate3D<int> index(i,j,k);
	  kernel_for_conv[index] = i*i-3*i+1.F+j*i/20.F+k;
	  kernel_for_DFT[modulo(index,sizes)] =
	    kernel_for_conv[index];
	}
  
  
      ArrayFilterUsingRealDFTWithPadding<3,float> DFT_filter;
      check(DFT_filter.set_kernel(kernel_for_DFT)==Succeeded::yes, "initialisation DFT filter");
      ArrayFilter3DUsingConvolution<float> conv_filter(kernel_for_conv);

      check(!DFT_filter.is_trivial(), "DFT is_trivial");
      check(!conv_filter.is_trivial(), "conv is_trivial");
      set_tolerance(test.find_max()*kernel_for_conv.sum()*1.E-6);
      //std::cerr << get_tolerance();

      cerr <<"Comparing DFT and Convolution with input offset 0\n";
      //compare_results_2arg(DFT_filter, conv_filter, test);
      compare_results_1arg(DFT_filter, conv_filter, test);
      cerr <<"Comparing DFT and Convolution with input negative offset\n";
      //compare_results_2arg(DFT_filter, conv_filter, test_neg_offset);
      compare_results_1arg(DFT_filter, conv_filter, test_neg_offset);
      cerr <<"Comparing DFT and Convolution with input positive offset\n";
      //compare_results_2arg(DFT_filter, conv_filter, test_pos_offset);
      compare_results_1arg(DFT_filter, conv_filter, test_pos_offset);
    }
  }
#endif
}

END_NAMESPACE_STIR

USING_NAMESPACE_STIR

int main()
{
  ArrayFilterTests tests;
  tests.run_tests();
  return tests.main_return_value();
}
