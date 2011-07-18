#include "definitions.h"

CustomWeakFormPoisson::CustomWeakFormPoisson(std::string mat_motor, double eps_motor, 
                                             std::string mat_air, double eps_air) : WeakForm<double>(1)
{
  // Jacobian.
  add_matrix_form(new WeakFormsH1::DefaultJacobianDiffusion<double>(0, 0, mat_motor, new HermesFunction<double>(eps_motor)));
  add_matrix_form(new WeakFormsH1::DefaultJacobianDiffusion<double>(0, 0, mat_air, new HermesFunction<double>(eps_air)));

  // Residual.
  add_vector_form(new WeakFormsH1::DefaultResidualDiffusion<double>(0, mat_motor, new HermesFunction<double>(eps_motor)));
  add_vector_form(new WeakFormsH1::DefaultResidualDiffusion<double>(0, mat_air, new HermesFunction<double>(eps_air)));
}

