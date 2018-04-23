#include "../core/node.hpp"
#include "../../device_context/cpu_context.hpp"
#include "../../math/blas.hpp"

namespace mlfe { namespace node {

template <typename T, typename D = CPUContext>
struct SoftmaxXentCpuF : NodeFunctor {
    void Init(OperatorContext *oc) override {
        auto dt = DataType::F32;
        _x = oc->inputs[0];
        _label = oc->inputs[1];
        _prob = oc->outputs[0];
        _loss = oc->outputs[1];
        _m = _x->Dim(0);
        _n = _x->Dim(1);

        // TODO : not use type size compare.
        if (sizeof(T) == 8) {
            dt = DataType::F64;
        }
        _prob->Allocate(Accelerator::Default, dt);
        _loss->Allocate(Accelerator::Default, dt);

        _sum_multiplier.Reshape({ _n });
        _sum_multiplier.AllocateAccelerator::Default, dt();
        _rowwise_max.Reshape({ _m });
        _rowwise_max.Allocate(Accelerator::Default, dt);
        _scaler.Reshape({ _m });
        _scaler.Allocate(Accelerator::Default, dt);

        math::set<T, D>(
            _sum_multiplier.Size(),
            static_cast<T>(1),
            _sum_multiplier.GetPtr<T>()
            );
    }

    void Run() override {
        math::rowwise_max<T, D>(
            _m, _n,
            _x->GetPtr<T>(),
            _rowwise_max.GetPtr<T>()
            );

        math::scal<T, D>(
            _m * _n, T(1),
            _x->GetPtr<T>(),
            _prob->GetPtr<T>()
            );

        math::gemm<T, D>(false, false,
            _m, _n, 1,
            T(-1), _rowwise_max.GetPtr<T>(), 1,
            _sum_multiplier.GetPtr<T>(), _n,
            T(1), _prob->GetPtr<T>(), _n, nullptr);

        math::exp<T, D>(
            _prob->Size(),
            _prob->GetPtr<T>(),
            _prob->GetPtr<T>()
            );

        math::gemv<T, D>(false,
            _m, _n,
            T(1), _prob->GetPtr<T>(), _n,
            _sum_multiplier.GetPtr<T>(),
            T(0), _scaler.GetPtr<T>(), 1, nullptr);

        math::rowwise_normalize<T, D>(_m, _n,
            _scaler.GetPtr<T>(),
            _prob->GetPtr<T>()
            );

        math::cross_entropy<T, D>(_m, _n,
            _prob->GetPtr<T>(),
            _label->GetPtr<T>(),
            _rowwise_max.GetPtr<T>()
            );

        math::sum<T, D>(
            _m,
            _rowwise_max.GetPtr<T>(),
            _loss->GetPtr<T>()
            );

        math::scal<T, D>(
            1,
            static_cast<T>(1) / static_cast<T>(_m),
            _loss->GetPtr<T>(),
            _loss->GetPtr<T>()
            );
    }

    Tensor *_x, *_label;
    Tensor *_prob, *_loss;
    Tensor _rowwise_max;
    Tensor _scaler;
    Tensor _sum_multiplier;
    int _m, _n;
};

REGIST_NODE_FUNCTOR(SoftmaxXent, DataType::F32, Accelerator::Default, SoftmaxXentCpuF<float>)
REGIST_NODE_FUNCTOR(SoftmaxXent, DataType::F64, Accelerator::Default, SoftmaxXentCpuF<double>)

template <typename T, typename D = CPUContext>
struct SoftmaxXentGradCpuF : NodeFunctor {
    void Init(OperatorContext *oc) override {
        auto dt = DataType::F32;
        _label = oc->inputs[0];
        _prob = oc->inputs[1];
        _loss = oc->inputs[2];
        _dx = oc->outputs[0];

        _m = _prob->Dim(0);
        _n = _prob->Dim(1);

        // TODO : not use type size compare.
        if (sizeof(T) == 8) {
            dt = DataType::F64;
        }
        _dx->Allocate(Accelerator::Default, dt);
    }
    void Run() override {
        math::cross_entropy_gradients<T, D>(_m, _n,
            _prob->GetPtr<T>(),
            _label->GetPtr<T>(),
            _loss->GetPtr<T>(),
            _dx->GetPtr<T>()
            );
    }

    Tensor *_label, *_prob, *_loss;
    Tensor *_dx;
    int _m, _n;
};

REGIST_NODE_GRADIENT_FUNCTOR(SoftmaxXent, DataType::F32, Accelerator::Default, SoftmaxXentGradCpuF<float>)
REGIST_NODE_GRADIENT_FUNCTOR(SoftmaxXent, DataType::F64, Accelerator::Default, SoftmaxXentGradCpuF<double>)

} // end namespace node
} // end namespace mlfe
