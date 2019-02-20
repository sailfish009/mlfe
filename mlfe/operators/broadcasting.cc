#include "broadcasting.h"
#include "../core/op_algo.h"
#include "../core/gradient_helper.h"

namespace mlfe{
namespace functional{

REGIST_OP(Broadcasting)
    .Input("X", "float32")
    .Output("Y", "float32")
    .Attr("broadcasting_shape", "int32s")
    .ShapeInference([](OpDesignContext * odc){
        using IntVec = std::vector<type::int32::T>;
        auto x = odc->Input(0);
        auto y = odc->Output(0);
        auto shape = odc->GetAttr<IntVec>("broadcasting_shape");
        auto x_shape = x.shape();
        y.reshape(shape, type::float32());
    })
    .Finish();

REGIST_OP_GRAD(Broadcasting)
    .Input("X", "float32")
    .Input("Y", "float32")
    .Input("dY", "float32")
    .Output("dX", "float32")
    .ShapeInference([](OpDesignContext * odc){
        auto x = odc->Input(0);
        auto y = odc->Input(1);
        auto dx = odc->Output(0);
        dx.reshape(x.shape(), type::float32());
    })
    .Finish();

class BroadcastingGradient : public GradientHelper{
public:
    BroadcastingGradient(const OpDesignContext *odc)
        : GradientHelper(odc){}

    VecTensor compute_gradient(Tensor y, Tensor dy) override{
        using IntVec = std::vector<type::int32::T>;
        VecTensor in_grads;
        Tensor x = y.get_children()[0];
        Tensor dx = functional::create_variable(x.shape());
        OpAlgoContext ctx_x_grad("BroadcastingGradient");
        dx.add_child(dy);
        Tensor::AssignOpFunctor(dx, ctx_x_grad);
        in_grads.push_back(dx);
        return in_grads;
    }
};

REGIST_GRADIENT_HELPER(Broadcasting, BroadcastingGradient)

std::vector<int> check_broadcasting(std::vector<int> a,
                                    std::vector<int> b
                                   ){
    std::vector<int> shape;
    int max = std::max(a.size(), b.size());
    while(max != a.size()){
        a.insert(a.begin(), 1);
    }
    while(max != b.size()){
        b.insert(b.begin(), 1);
    }
    shape.resize(max);
    for(int n = max - 1; n >= 1; --n){
        int a_at = a[n];
        int b_at = b[n];
        if(a_at != 1 && b_at != 1 && a_at != b_at){
            throw std::string("Can not broadcasting.");
        }
        else{
            shape[n] = std::max(a_at, b_at);
        }
    }
    shape[0] = std::max(a[0], b[0]);
    return shape;
}

Tensor broadcast(Tensor x,
                 std::vector<type::int32::T> shape
                 ){
    check_broadcasting(x.shape(), shape);
    Tensor y = create_variable(shape);
    OpAlgoContext ctx("Broadcasting");
    ctx.add_attr({"broadcasting_shape", shape});
    y.add_child(x);
    Tensor::AssignOpFunctor(y, ctx);

    return y;
}

} // end namespace functional
} // end namespace mlfe
