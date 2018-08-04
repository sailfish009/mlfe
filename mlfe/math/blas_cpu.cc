#include <Eigen/Dense>
#include "blas.h"
#include "../device_context/cpu_context.h"

namespace mlfe{ namespace math{

template<>
void gemm<float, CPUContext>(
                             const bool trans_a,
                             const bool trans_b,
                             const int m,
                             const int n,
                             const int k,
                             const float alpha,
                             const float *a_ptr,
                             const int lda,
                             const float *b_ptr,
                             const int ldb,
                             const float beta,
                             float *c_ptr,
                             const int ldc,
                             CPUContext *context
                             ){
    Eigen::Map<Eigen::MatrixXf> c(c_ptr, n, m);
    if(beta == 0.f){
        c.setZero();
    }
    else{
        c *= beta;
    }
    if(!trans_a && !trans_b){
        c.noalias() += alpha * (
                                 Eigen::Map<const Eigen::MatrixXf>(b_ptr, n, k) *
                                 Eigen::Map<const Eigen::MatrixXf>(a_ptr, k, m));
    }
    else if(trans_a && !trans_b){
        c.noalias() += alpha * (
                                 Eigen::Map<const Eigen::MatrixXf>(b_ptr, n, k) *
                                 Eigen::Map<const Eigen::MatrixXf>(a_ptr, m, k).transpose());
    }
    else if(!trans_a && trans_b){
        c.noalias() += alpha * (
                                 Eigen::Map<const Eigen::MatrixXf>(b_ptr, k, n).transpose() *
                                 Eigen::Map<const Eigen::MatrixXf>(a_ptr, k, m));
    }
    else{
        c.noalias() += alpha * (
                                 Eigen::Map<const Eigen::MatrixXf>(b_ptr, k, n).transpose() *
                                 Eigen::Map<const Eigen::MatrixXf>(a_ptr, m, k).transpose());
    }
}

template<>
void gemm<double, CPUContext>(
                              const bool trans_a,
                              const bool trans_b,
                              const int m,
                              const int n,
                              const int k,
                              const double alpha,
                              const double *a_ptr,
                              const int lda,
                              const double *b_ptr,
                              const int ldb,
                              const double beta,
                              double *c_ptr,
                              const int ldc,
                              CPUContext *context
                              ){
    Eigen::Map<Eigen::MatrixXd> c(c_ptr, n, m);
    if(beta == 0.){
        c.setZero();
    }
    else{
        c *= beta;
    }
    if(!trans_a && !trans_b){
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXd>(b_ptr, n, k) *
                                Eigen::Map<const Eigen::MatrixXd>(a_ptr, k, m));
    }
    else if(trans_a && !trans_b){
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXd>(b_ptr, n, k) *
                                Eigen::Map<const Eigen::MatrixXd>(a_ptr, m, k).transpose());
    }
    else if(!trans_a && trans_b){
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXd>(b_ptr, k, n).transpose() *
                                Eigen::Map<const Eigen::MatrixXd>(a_ptr, k, m));
    }
    else{
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXd>(b_ptr, k, n).transpose() *
                                Eigen::Map<const Eigen::MatrixXd>(a_ptr, m, k).transpose());
    }
}

template <>
void gemv<float, CPUContext>(const bool trans_a,
                             const int m,
                             const int n,
                             const float alpha,
                             const float *a_ptr,
                             const int lda,
                             const float *b_ptr,
                             const float beta,
                             float *c_ptr,
                             const int ldc,
                             CPUContext *context
                             ){
    Eigen::Map<Eigen::VectorXf> c(c_ptr, !trans_a ? m : n);
    if(beta == 0.f){
        c.setZero();
    }
    else{
        c *= beta;
    }
    
    if(!trans_a){
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXf>(a_ptr, n, m).transpose() *
                                Eigen::Map<const Eigen::VectorXf>(b_ptr, n)
                                );
    }
    else{
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXf>(a_ptr, n, m) *
                                Eigen::Map<const Eigen::VectorXf>(b_ptr, m)
                                );
    }
}

template <>
void gemv<double, CPUContext>(const bool trans_a,
                              const int m,
                              const int n,
                              const double alpha,
                              const double *a_ptr,
                              const int lda,
                              const double *b_ptr,
                              const double beta,
                              double *c_ptr,
                              const int ldc,
                              CPUContext *context
                              ){
    Eigen::Map<Eigen::VectorXd> c(c_ptr, !trans_a ? m : n);
    if(beta == 0.){
        c.setZero();
    }
    else{
        c *= beta;
    }
    
    if(!trans_a){
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXd>(a_ptr, n, m).transpose() *
                                Eigen::Map<const Eigen::VectorXd>(b_ptr, n)
                                );
    }
    else{
        c.noalias() += alpha * (
                                Eigen::Map<const Eigen::MatrixXd>(a_ptr, n, m) *
                                Eigen::Map<const Eigen::VectorXd>(b_ptr, m)
                                );
    }
}

template <>
void rowwise_max<float, CPUContext>(
                                    const int m, const int n,
                                    const float *a_ptr,
                                    float *b_ptr
                                    ){
    Eigen::Map<Eigen::VectorXf>(b_ptr, m) =  Eigen::Map<const Eigen::MatrixXf>(a_ptr, n, m).colwise().maxCoeff();
}

template <>
void rowwise_max<double, CPUContext>(
                                     const int m, const int n,
                                     const double *a_ptr,
                                     double *b_ptr
                                     ){
    Eigen::Map<Eigen::VectorXd>(b_ptr, m) =  Eigen::Map<const Eigen::MatrixXd>(a_ptr, n, m).colwise().maxCoeff();
}

template <>
void rowwise_normalize<float, CPUContext>(
                                          const int m, const int n,
                                          const float *scaler_ptr,
                                          float *norm_dest
                                          ){
    for(int i = 0; i < m; ++i){
        for(int j = 0; j < n; ++j){
            norm_dest[i * n + j] /= scaler_ptr[i];
        }
    }
}

template <>
void rowwise_normalize<double, CPUContext>(
                                           const int m, const int n,
                                           const double *scaler_ptr,
                                           double *norm_dest
                                           ){
    for(int i = 0; i < m; ++i){
        for(int j = 0; j < n; ++j){
            norm_dest[i * n + j] /= scaler_ptr[i];
        }
    }
}

template <>
void cross_entropy<float, CPUContext>(
                                      const int m, const int n,
                                      const float *prob_ptr,
                                      const float *label_ptr,
                                      float *loss_ptr
                                      ){
    for(int i = 0; i < m; ++i){
        float row_loss = 0.f;
        for(int j = 0; j < n; ++j){
            row_loss += -std::log(std::max(prob_ptr[i * n + j], 1e-20f)) * label_ptr[i * n + j];
        }
        loss_ptr[i] = row_loss;
    }
}

template <>
void cross_entropy<double, CPUContext>(
                                       const int m, const int n,
                                       const double *prob_ptr,
                                       const double *label_ptr,
                                       double *loss_ptr
                                       ){
    for(int i = 0; i < m; ++i){
        double row_loss = 0.;
        for(int j = 0; j < n; ++j){
            row_loss += -std::log(std::max(prob_ptr[i * n + j], 1e-20)) * label_ptr[i * n + j];
        }
        loss_ptr[i] = row_loss;
    }
}

template <>
void cross_entropy_gradients<float, CPUContext>(
                                                const int m, const int n,
                                                const float *prob_ptr,
                                                const float *label_ptr,
                                                const float *loss_ptr,
                                                float *dx_ptr
                                                ){
    float avg_loss = loss_ptr[0] / static_cast<float>(m);
    for (int i = 0; i < m * n; ++i) {
        dx_ptr[i] = (prob_ptr[i] - label_ptr[i]) * avg_loss;
    }
}

template <>
void cross_entropy_gradients<double, CPUContext>(
                                                 const int m, const int n,
                                                 const double *prob_ptr,
                                                 const double *label_ptr,
                                                 const double *loss_ptr,
                                                 double *dx_ptr
                                                 ){
    double avg_loss = loss_ptr[0] / static_cast<double>(m);
    for (int i = 0; i < m * n; ++i) {
        dx_ptr[i] = (prob_ptr[i] - label_ptr[i]) * avg_loss;
    }
}

template<>
void exp<float, CPUContext>(
                            const int size,
                            const float *x_ptr,
                            float *y_ptr){
    Eigen::Map<Eigen::VectorXf>(y_ptr, size) = Eigen::Map<const Eigen::VectorXf>(x_ptr, size).array().exp();
}

template<>
void exp<double, CPUContext>(
                             const int size,
                             const double *x_ptr,
                             double *y_ptr){
    Eigen::Map<Eigen::VectorXd>(y_ptr, size) = Eigen::Map<const Eigen::VectorXd>(x_ptr, size).array().exp();
}

template<>
void axpy<float, CPUContext>(int size,
                             const float alpha,
                             const float *x_ptr,
                             float *y_ptr){
    Eigen::Map<Eigen::VectorXf>(y_ptr, size) += alpha * Eigen::Map<const Eigen::VectorXf>(x_ptr, size);
}

template<>
void axpy<double, CPUContext>(int size,
                              const double alpha,
                              const double *x_ptr,
                              double *y_ptr){
    Eigen::Map<Eigen::VectorXd>(y_ptr, size) += alpha * Eigen::Map<const Eigen::VectorXd>(x_ptr, size);
}

template <>
void scal<float, CPUContext>(const int size,
                              const float alpha,
                              const float *x_ptr,
                              float *y_ptr){
    Eigen::Map<Eigen::VectorXf> y(y_ptr, size);
    if(alpha != 0.f){
        y = alpha * Eigen::Map<const Eigen::VectorXf>(x_ptr, size);
    }
    else{
        y.setZero();
    }
}

template <>
void scal<double, CPUContext>(const int size,
                              const double alpha,
                              const double *x_ptr,
                              double *y_ptr){
    Eigen::Map<Eigen::VectorXd> y(y_ptr, size);
    if(alpha != 0.){
        y = alpha * Eigen::Map<const Eigen::VectorXd>(x_ptr, size);
    }
    else{
        y.setZero();
    }
}

template <>
void sum<float, CPUContext>(
    const int size,
    const float *x_ptr,
    float *y_ptr) {
    y_ptr[0] = Eigen::Map<const Eigen::VectorXf>(x_ptr, size).sum();
}

template <>
void sum<double, CPUContext>(
    const int size,
    const double *x_ptr,
    double *y_ptr) {
    y_ptr[0] = Eigen::Map<const Eigen::VectorXd>(x_ptr, size).sum();
}

template<>
void set<float, CPUContext>(
    const int size,
    const float val,
    float *x_ptr
    ){
    Eigen::Map<Eigen::VectorXf>(x_ptr, size).setConstant(val);
}

template<>
void set<double, CPUContext>(
    const int size,
    const double val,
    double *x_ptr
    ) {
    Eigen::Map<Eigen::VectorXd>(x_ptr, size).setConstant(val);
}

} /* math */
} /* mlfe */
