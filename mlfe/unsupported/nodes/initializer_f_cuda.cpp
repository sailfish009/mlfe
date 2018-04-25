#include <random>
#include "../core/node.hpp"
#include "../../device_context/cuda_context.hpp"
#include "../../math/blas.hpp"
#include "../../math/functions.hpp"
#include "../../utils/assert.hpp"
#include "../../math/functions_cuda.hpp"

namespace mlfe { namespace node {

template <typename T, typename D = CUDAContext>
struct ConstantInitCudaF : NodeFunctor {
    void Init(OperatorContext *oc) override{
        auto val_str = oc->attr->GetParam<std::string>("Value");
        _val = to_value<T>(val_str);
        _x = oc->inputs[0];
    }

    void Run() override{
        math::set<T, D>(
            _x->Size(),
            _val,
            _x->GetPtr<T>()
            );
    }

    Tensor *_x;
    T _val;
};

REGIST_NODE_FUNCTOR(ConstantInit, DataType::F32, Accelerator::CUDA, ConstantInitCudaF<float>)

template <typename T>
struct XavierInitCudaF : NodeFunctor {
    void Init(OperatorContext *oc) override {
        auto seed_str = oc->attr->GetParam<std::string>("Seed");
        _x = oc->inputs[0];
        T scale_denominator = _x->Dims() > 1 ?
            static_cast<T>(_x->Size()) / _x->Dim(0) : static_cast<T>(_x->Size());
        T scale = std::sqrt(
            static_cast<T>(6) /
            static_cast<T>(scale_denominator)
        );
        _a = -scale;
        _b = scale;

        cudaMalloc((void**)&_states, _x->Size() * sizeof(curandState_t));
        math::InitCurand(to_value<T>(seed_str), _x->Size(), _states);
    }

    void Run() override {
        int size = _x->Size();
        T *x_ptr = _x->GetPtr<T>();
        math::UniformCurand<T>(_states, size, x_ptr, _a, _b);
    }
    
    ~XavierInitCudaF() {
        cudaFree(_states);
    }

    Tensor *_x;
    T _a, _b;
    curandState_t *_states;
};

REGIST_NODE_FUNCTOR(XavierInit, DataType::F32, Accelerator::CUDA, XavierInitCudaF<float>)

} // end namespace node
} // end namespace mlfe