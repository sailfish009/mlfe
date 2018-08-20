#include "../core/op_algo.h"
#include "../core/tensor_mem_ref.h"
#include "../math/blas.h"
#include "../device_context/cpu_context.h"
#include "../core/device.h"

namespace mlfe{ namespace algorithm_cpu{

template <class Dev, class Tp>
class Dropout : public OpAlgo{
using T = typename Tp::T;
public:
    Dropout(OpAlgoContext *oac) : OpAlgo(oac){
        x = oac->GetVar("X");
        y = oac->GetVar("Y");
        mask = oac->GetVar("Mask");
        drop_ratio = oac->GetAttr<T>("dropout_ratio");
        training = oac->GetAttr<bool>("is_training_step");
        drop_ratio_inv = T(1) / (T(1) - drop_ratio);
        size = x->Size();
        b_dist = std::bernoulli_distribution(T(1) - drop_ratio);
    }

    void Compute() override{
        auto x_ptr = x->Data<T>();
        auto y_ptr = y->Data<T>();
        auto mask_ptr = mask->Data<T>();
        if(training){
            for(int n = 0; n < size; ++n){
                T mask_val = mask_ptr[n] = b_dist(CPUContext::rng);
                y_ptr[n] = x_ptr[n] * mask_val * drop_ratio_inv;
            }
        }
        else{
            Device::Copy<Device::CPU, Device::CPU>(x->GetDevice(), y->GetDevice());
        }
    }
private:
    TensorMemRef *x;
    TensorMemRef *y;
    TensorMemRef *mask;
    std::bernoulli_distribution b_dist;
    T drop_ratio, drop_ratio_inv;
    bool training;
    int size;
};

REGIST_OP_ALGO(Dropout)
    .Input("X", type::float32::string)
    .Input("W", type::float32::string)
    .Input("B", type::float32::string)
    .Output("Y", type::float32::string)
    .Device(Device::CPU::string)
    .CreatorFn([](OpAlgoContext *oac) ->std::shared_ptr<OpAlgo>{
        using T = Dropout<Device::CPU, type::float32>;
        return std::make_shared<T>(oac);
    })
    .Finish();

template <class Dev, class Tp>
class DropoutGrad : public OpAlgo{
using T = typename Tp::T;
public:
    DropoutGrad(OpAlgoContext *oac) : OpAlgo(oac){
        dy = oac->GetVar("dY");
        dx = oac->GetVar("dX");
        mask = oac->GetVar("Mask");
        drop_ratio = oac->GetAttr<T>("dropout_ratio");
        drop_ratio_inv = T(1) / (T(1) - drop_ratio);
        size = dy->Size();
    }

    void Compute() override{
        auto dy_ptr = dy->Data<T>();
        auto dx_ptr = dx->Data<T>();
        auto mask_ptr = mask->Data<T>();
        for(int n = 0; n < size; ++n){
            dx_ptr[n] = dy_ptr[n] * mask_ptr[n] * drop_ratio_inv;
        }
    }

private:
    TensorMemRef *x;
    TensorMemRef *y;
    TensorMemRef *dy;
    TensorMemRef *dx;
    TensorMemRef *mask;
    T drop_ratio, drop_ratio_inv;
    int size;
};

REGIST_OP_GRAD_ALGO(Dropout)
    .Input("X", type::float32::string)
    .Input("Y", type::float32::string)
    .Input("dY", type::float32::string)
    .Output("dX", type::float32::string)
    .Device(Device::CPU::string)
    .CreatorFn([](OpAlgoContext *oac) ->std::shared_ptr<OpAlgo>{
        using T = DropoutGrad<Device::CPU, type::float32>;
        return std::make_shared<T>(oac);
    })
    .Finish();

} // end namespace algorithm_cpu
} // end namespace mlfe