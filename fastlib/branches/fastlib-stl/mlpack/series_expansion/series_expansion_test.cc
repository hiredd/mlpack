#include <fastlib/fastlib.h>
#include <fastlib/fx/io.h>
#include <complex>
#include "fourier_expansion.h"
#include "fourier_series_expansion_aux.h"
#include "fourier_kernel_aux.h"
#include <mlpack/core/kernels/lmetric.h>

using namespace mlpack;

class SeriesExpansionTest {

 private:

  template<typename TKernelAux>
  double BaseCase_(const arma::vec& query_point, const arma::mat& reference_set,
		   const arma::vec& reference_weights,
		   const TKernelAux &kernel_aux) {

    double sum = 0.0;
    
    for(index_t i = 0; i < reference_set.n_cols; i++) {
      double squared_distance = kernel::LMetric<2>::Evaluate(query_point,
          reference_set.unsafe_col(i));
      printf("Got distance: %g\n", squared_distance);
      sum += kernel_aux.kernel_.EvalUnnormOnSq(squared_distance);
    }
    return sum;
  }

 public:

  void Init() {
  }

  void TestFourierExpansion() {
    
    IO::Info << "[*] TestFourierExpansion" << std::endl;
    GaussianKernelFourierAux<double> kernel_aux;
    index_t order = 3;
    index_t dim = 3;
    double bandwidth = 30;
    kernel_aux.Init(bandwidth, order, dim);

    // Set the integral truncation limit manually.
    kernel_aux.sea_.set_integral_truncation_limit(bandwidth * 3.0);

    // Create an expansion from a random synthetic dataset.
    arma::mat random_dataset(3, 20);
    arma::vec weights(20);
    arma::vec center(3);
    center.zeros();
    for(index_t j = 0; j < 20; j++) {
      for(index_t i = 0; i < 3; i++) {
	random_dataset(i, j) = math::Random(0, i);
      }
    }
    for(index_t i = 0; i < 20; i++) {
      center += random_dataset.unsafe_col(i);
    }
    center /= 20.0;
    std::cout << center;
    weights.ones();
    FourierExpansion<GaussianKernelFourierAux<double> > expansion;
    expansion.Init(center, kernel_aux);
    
    expansion.AccumulateCoeffs(random_dataset, weights, 0, 20, 3);
    
    // Retrieve the coefficients and print them out.
    const arma::Col<std::complex<double> > &coeffs = expansion.get_coeffs();
    std::cout << coeffs;

    // Evaluate the expansion, and compare against the naive.
    arma::vec evaluation_point(3);
    evaluation_point.fill(2.0);
    IO::Info << "Expansion evaluated to be: "
 	     << expansion.EvaluateField(evaluation_point.memptr(), 3) << std::endl;
    IO::Info << "Naive sum: " << BaseCase_(evaluation_point, random_dataset,weights, kernel_aux) << std::endl;
  }

  void TestFourierExpansionMapping() {
    IO::Info << "[*] TestFourierExpansionMapping" << std::endl;
    FourierSeriesExpansionAux<double> series_aux;
    index_t order = 2;
    index_t dim = 3;
    series_aux.Init(order, dim);
    series_aux.PrintDebug();

    // Verify that the shifting of each multiindex by the max order
    // roughly corresponds to the base ((2 * order) + 1) number.
    index_t total_num_mapping = (index_t) pow(2 * order + 1, dim);
    for(index_t i = 0; i < total_num_mapping; i++) {
      const std::vector<index_t> &mapping = series_aux.get_multiindex(i);

      index_t number = 0;
      printf("The mapping: ");
      for(index_t j = 0; j < dim; j++) {
	number = (2 * order + 1) * number + (mapping[j] + order);
	printf("%"LI" ", mapping[j]);
      }
      printf("maps to %"LI"\n", number);
      
      if(number != i) {
        IO::Info << "The mapping at the position " << i << LI << " is computed incorrectly!" << std::endl;
      }
    }
  }
  
  void TestAll() {
    TestFourierExpansionMapping();
    TestFourierExpansion();
    IO::Info << "[*] All tests passed !!" << std::endl;
  }
};

int main(int argc, char *argv[]) {
  IO::ParseCommandLine(argc, argv);
  SeriesExpansionTest test;
  test.Init();
  test.TestAll();
  return 0;
}
