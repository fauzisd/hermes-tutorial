#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "definitions.h"

using namespace Hermes;
using namespace Hermes::Hermes2D;
using namespace Hermes::Hermes2D::Views;
using namespace Hermes::Hermes2D::RefinementSelectors;

//  The purpose of this example is to show how to use Trilinos
//  for time-dependent PDE problem.
//  NOX solver is used, either using Newton's method or JFNK and
//  with or without preconditioning,
//
//  PDE: Heat transfer: HEATCAP*RHO*du/dt - div(LAMBDA * grad u) = 0.
//
//  Domain: Unit square.
//
//  BC: Dirichlet at the bottom, Newton du/dn = ALPHA*(TEMP_EXT - u) elsewhere.
//

const int INIT_REF_NUM = 4;       // Number of initial uniform mesh refinements.
const int P_INIT = 1;             // Initial polynomial degree of all mesh elements.
const double ALPHA = 10.0;        // Coefficient for the Nwwton boundary condition.
const double LAMBDA = 1e5;
const double HEATCAP = 1e6;
const double RHO = 3000.0;
const double TEMP_EXT = 20.0;
const double TEMP_INIT = 10.0;
const double TAU = 50.0;          // Time step.        

// NOX parameters.
const bool TRILINOS_JFNK = true;                  // true = Jacobian-free method (for NOX),
                                                  // false = Newton (for NOX).
const bool PRECOND = true;                        // Preconditioning by jacobian in case of JFNK (for NOX),
                                                  // default ML preconditioner in case of Newton.
const char* iterative_method = "GMRES";           // Name of the iterative method employed by AztecOO (ignored
                                                  // by the other solvers). 
                                                  // Possibilities: gmres, cg, cgs, tfqmr, bicgstab.
const char* preconditioner = "AztecOO";           // Name of the preconditioner employed by AztecOO 
                                                  // Possibilities: None" - No preconditioning. 
                                                  // "AztecOO" - AztecOO internal preconditioner.
                                                  // "New Ifpack" - Ifpack internal preconditioner.
                                                  // "ML" - Multi level preconditione
unsigned message_type = NOX::Utils::Error | NOX::Utils::Warning | NOX::Utils::OuterIteration | NOX::Utils::InnerIteration | NOX::Utils::Parameters | NOX::Utils::LinearSolverDetails;
                                                  // NOX error messages, see NOX_Utils.h.
double ls_tolerance = 1e-5;                       // Tolerance for linear system.
unsigned flag_absresid = 0;                       // Flag for absolute value of the residuum.
double abs_resid = 1.0e-3;                        // Tolerance for absolute value of the residuum.
unsigned flag_relresid = 1;                       // Flag for relative value of the residuum.
double rel_resid = 1.0e-2;                        // Tolerance for relative value of the residuum.
int max_iters = 100;                              // Max number of iterations.

int main(int argc, char* argv[])
{
  // Load the mesh.
  Mesh mesh;
  MeshReaderH2D mloader;
  mloader.load("square.mesh", &mesh);

  // Perform initial mesh refinemets.
  for (int i=0; i < INIT_REF_NUM; i++) mesh.refine_all_elements();

  // Initialize boundary conditions.
  DefaultEssentialBCConst<double> bc("Bdy_bottom", TEMP_INIT);
  EssentialBCs<double> bcs(&bc);

  // Create an H1 space with default shapeset.
  H1Space<double> space(&mesh, &bcs, P_INIT);
  int ndof = Space<double>::get_num_dofs(&space);
  info("ndof: %d", ndof);

  // Define constant initial condition. 
  ConstantSolution<double> t_prev_time(&mesh, TEMP_INIT);

  // Initialize the weak formulation.
  CustomWeakForm wf(Hermes::vector<std::string>("Bdy_right", "Bdy_top", "Bdy_left"), 
                    HEATCAP, RHO, TAU, LAMBDA, ALPHA, TEMP_EXT, &t_prev_time, TRILINOS_JFNK);

  // Initialize the finite element problem.
  DiscreteProblem<double> dp(&wf, &space);

  // Project the function "t_prev_time" on the FE space 
  // in order to obtain initial vector for NOX. 
  info("Projecting initial solution on the FE mesh.");
  double* coeff_vec = new double[ndof];
  OGProjection<double>::project_global(&space, &t_prev_time, coeff_vec);

  // Initialize the NOX solver.
  info("Initializing NOX.");
  NewtonSolverNOX<double> solver_nox(&dp);
  solver_nox.set_output_flags(message_type);

  solver_nox.set_ls_type(iterative_method);
  solver_nox.set_ls_tolerance(ls_tolerance);

  solver_nox.set_conv_iters(max_iters);
  if (flag_absresid)
    solver_nox.set_conv_abs_resid(abs_resid);
  if (flag_relresid)
    solver_nox.set_conv_rel_resid(rel_resid);

  // Select preconditioner.
  MlPrecond<double> pc("sa");
  if (PRECOND)
  {
    if (TRILINOS_JFNK) solver_nox.set_precond(pc);
    else solver_nox.set_precond("ML");
  }

  // Initialize the view.
  ScalarView Tview("Temperature", new WinGeom(0, 0, 450, 400));
  Tview.set_min_max_range(10,20);

  // Time stepping loop:
  double total_time = 0.0;
  for (int ts = 1; total_time <= 2000.0; ts++)
  {
    info("---- Time step %d, t = %g s", ts, total_time += TAU);

    info("Assembling by DiscreteProblem, solving by NOX.");
    try
    {
      solver_nox.solve(coeff_vec);
    }
    catch(Hermes::Exceptions::Exception e)
    {
      e.printMsg();
      error("NOX failed.");
    }

    Solution<double>::vector_to_solution(solver_nox.get_sln_vector(), &space, &t_prev_time);

    // Show the new solution.
    Tview.show(&t_prev_time);

    info("Number of nonlin iterations: %d (norm of residual: %g)", 
      solver_nox.get_num_iters(), solver_nox.get_residual());
    info("Total number of iterations in linsolver: %d (achieved tolerance in the last step: %g)", 
      solver_nox.get_num_lin_iters(), solver_nox.get_achieved_tol());
  }

  // Wait for all views to be closed.
  View::wait();
  return 0;
}
