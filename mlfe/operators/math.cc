#include "math.h"
#include "../core/op_algo.h"
#include "../core/gradient_helper.h"
#include "../operators/basic_arithmetics.h"
#include "../operators/initializer.h"

namespace mlfe{
namespace functional{

REGIST_OP(SquaredDifference)
    .Input("X1", "float32")
    .Input("X2", "float32")
    .Output("Y", "float32")
    .Attr("output", "int32")
    .ShapeInference([](OpDesignContext * odc){
        auto x1 = odc->Input(0);
        auto x2 = odc->Input(1);
        auto y = odc->Output(0);
        
        if(x1.Dims() != x2.Dims()){
            throw std::string("squared_difference op : ") +
                std::string("x1.Shape() != x2.Shape()");
        }
        for(int n = 0; n < x1.Dims(); ++n){
            if(x1.Shape()[n] != x2.Shape()[n]){
                throw std::string("squared_difference op : ") +
                    std::string("x1.Shape() != x2.Shape()");
            }
        }
        y.Reshape(x1.Shape(), type::float32());
    })
    .Finish();

REGIST_OP_GRAD(SquaredDifference)
    .Input("X1", "float32")
    .Input("X2", "float32")
    .Input("dY", "float32")
    .Output("dX1", "float32")
    .Output("dX2", "float32")
    .ShapeInference([](OpDesignContext * odc){
        auto x1 = odc->Input(0);
        auto x2 = odc->Input(1);
        auto dy = odc->Input(2);
        auto dx1 = odc->Output(0);
        auto dx2 = odc->Output(1);
        dx1.Reshape(x1.Shape(), type::float32());
        dx2.Reshape(x2.Shape(), type::float32());
    })
    .Finish();

class SquaredDifferenceGradient : public GradientHelper{
public:
    SquaredDifferenceGradient(const OpDesignContext *odc)
        : GradientHelper(odc){}

    VecTensor compute_gradient(Tensor y, Tensor dy) override{
        namespace fn = functional;
        VecTensor in_grads;
        auto x1 = y.get_children()[0];
        auto x2 = y.get_children()[1];
        auto two = functional::constant(2, x1.Shape());
        Tensor dx1 = fn::mul(two, fn::sub(x1, x2));
        Tensor dx2 = fn::negative(fn::mul(two, fn::sub(x1, x2)));
        in_grads.push_back(dx1);
        in_grads.push_back(dx2);
        return in_grads;
    }
};

REGIST_GRADIENT_HELPER(SquaredDifference, SquaredDifferenceGradient)

Tensor squared_difference(Tensor x1, Tensor x2){
    Tensor y = create_variable(x1.Shape());
    OpAlgoContext ctx("SquaredDifference");
    y.add_child(x1);
    y.add_child(x2);
    Tensor::AssignOpFunctor(y, ctx);
    return y;
}

} // end namespace functional
} // end namespace mlfe
