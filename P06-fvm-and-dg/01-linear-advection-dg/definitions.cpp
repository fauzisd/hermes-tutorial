#include "definitions.h"

CustomWeakForm::CustomWeakForm(std::string left_bottom_bnd_part, bool DG) : WeakForm<double>(1) {
  add_matrix_form(new MatrixFormVol(0, 0));
  add_vector_form(new VectorFormVol(0));
  add_matrix_form_surf(new MatrixFormSurface(0, 0));
  if(DG)
    add_matrix_form_surf(new MatrixFormInterface(0, 0));
  add_vector_form_surf(new VectorFormSurface(0, left_bottom_bnd_part));
};

CustomWeakForm::MatrixFormVol::MatrixFormVol(int i, int j) : Hermes::Hermes2D::MatrixFormVol<double>(i, j) { }

template<typename Real, typename Scalar>
Scalar CustomWeakForm::MatrixFormVol::matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const {
  Scalar result = Scalar(0);
  for (int i = 0; i < n; i++)
    result += -wt[i] * u->val[i] * static_cast<CustomWeakForm*>(wf)->calculate_a_dot_v(e->x[i], e->y[i], v->dx[i], v->dy[i]);
  return result;
}

double CustomWeakForm::MatrixFormVol::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const {
  return matrix_form<double, double>(n, wt, u_ext, u, v, e, ext);
}

Ord CustomWeakForm::MatrixFormVol::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
  return matrix_form<Ord, Ord>(n, wt, u_ext, u, v, e, ext);
}

CustomWeakForm::VectorFormVol::VectorFormVol(int i) : Hermes::Hermes2D::VectorFormVol<double>(i) { }

template<typename Real, typename Scalar>
Scalar CustomWeakForm::VectorFormVol::vector_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const {
  Scalar result = Scalar(0);
  for (int i = 0; i < n; i++)
    result += wt[i] * F(e->x[i], e->y[i]) * v->val[i];
  return result;
}

double CustomWeakForm::VectorFormVol::value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e, ExtData<double> *ext) const {
  return vector_form<double, double>(n, wt, u_ext, v, e, ext);
}

Ord CustomWeakForm::VectorFormVol::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
  return vector_form<Ord, Ord>(n, wt, u_ext, v, e, ext);
}

template<typename Real>
Real CustomWeakForm::VectorFormVol::F(Real x, Real y) const {
  return Real(0);
}


CustomWeakForm::MatrixFormSurface::MatrixFormSurface(int i, int j) : Hermes::Hermes2D::MatrixFormSurf<double>(i, j, H2D_DG_BOUNDARY_EDGE) { }

template<typename Real, typename Scalar>
Scalar CustomWeakForm::MatrixFormSurface::matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const {
  Scalar result = Scalar(0);

  for (int i = 0; i < n; i++) {
    Real x = e->x[i], y = e->y[i];
    Real a_dot_n = static_cast<CustomWeakForm*>(wf)->calculate_a_dot_v(x, y, e->nx[i], e->ny[i]);
    result += wt[i] * static_cast<CustomWeakForm*>(wf)->upwind_flux(u->val[i], Scalar(0), a_dot_n) * v->val[i];
  }

  return result;
}

double CustomWeakForm::MatrixFormSurface::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const {
  return matrix_form<double, double>(n, wt, u_ext, u, v, e, ext);
}

Ord CustomWeakForm::MatrixFormSurface::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
  return matrix_form<Ord, Ord>(n, wt, u_ext, u, v, e, ext);
}

CustomWeakForm::MatrixFormInterface::MatrixFormInterface(int i, int j) : Hermes::Hermes2D::MatrixFormSurf<double>(i, j, H2D_DG_INNER_EDGE) { }

template<typename Real, typename Scalar>
Scalar CustomWeakForm::MatrixFormInterface::matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const {
  Scalar result = Scalar(0);

  for (int i = 0; i < n; i++) {
    Real a_dot_n = static_cast<CustomWeakForm*>(wf)->calculate_a_dot_v(e->x[i], e->y[i], e->nx[i], e->ny[i]);
    Real jump_v = v->get_val_central(i) - v->get_val_neighbor(i);
    result += wt[i] * static_cast<CustomWeakForm*>(wf)->upwind_flux(u->get_val_central(i), u->get_val_neighbor(i), a_dot_n) * jump_v;
  }

  return result;
}

double CustomWeakForm::MatrixFormInterface::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const {
  return matrix_form<double, double>(n, wt, u_ext, u, v, e, ext);
}

Ord CustomWeakForm::MatrixFormInterface::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
  return matrix_form<Ord, Ord>(n, wt, u_ext, u, v, e, ext);
}

CustomWeakForm::VectorFormSurface::VectorFormSurface(int i, std::string left_bottom_bnd_part) : Hermes::Hermes2D::VectorFormSurf<double>(i, left_bottom_bnd_part) { }

double CustomWeakForm::VectorFormSurface::value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e, ExtData<double> *ext) const {
  double result = 0;
  for (int i = 0; i < n; i++) {
    double x = e->x[i], y = e->y[i];
    double a_dot_n = static_cast<CustomWeakForm*>(wf)->calculate_a_dot_v(x, y, e->nx[i], e->ny[i]);
    // Function values for Dirichlet boundary conditions.
    result += -wt[i] * static_cast<CustomWeakForm*>(wf)->upwind_flux(0, 1, a_dot_n) * v->val[i];
  }
  return result;
}

Ord CustomWeakForm::VectorFormSurface::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const {
  Ord result = Ord(0);
  for (int i = 0; i < n; i++)
    result += -wt[i] * v->val[i];
  return result;
}

template<typename Real>
Real CustomWeakForm::VectorFormSurface::F(Real x, Real y) const{
  return Real(0);
}

double CustomWeakForm::calculate_a_dot_v(double x, double y, double vx, double vy) const {
  double norm = std::max<double>(1e-12, std::sqrt(Hermes::sqr(x) + Hermes::sqr(y)));
  return -y/norm*vx + x/norm*vy;
}

Ord CustomWeakForm::calculate_a_dot_v(Ord x, Ord y, Ord vx, Ord vy) const {
  return Ord(10);
}

double CustomWeakForm::upwind_flux(double u_cent, double u_neib, double a_dot_n) const {
  return a_dot_n * (a_dot_n >= 0 ? u_cent : u_neib); 
}

Ord CustomWeakForm::upwind_flux(Ord u_cent, Ord u_neib, Ord a_dot_n) const {
  return a_dot_n * (u_cent + u_neib); 
}