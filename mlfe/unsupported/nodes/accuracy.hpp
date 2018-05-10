#ifndef __ACCURACY_OP_HPP__
#define __ACCURACY_OP_HPP__
#include "../core/node.hpp"

namespace mlfe { namespace node {

class Accuracy final : public NodeIO<Accuracy> {
public:
    Accuracy();

protected:
    void InternalInit(Workspace *ws, OperatorContext *oc) override;

    void InternalGradientInit(Workspace *ws, OperatorContext *oc) override;
};

} // end namespace node
} // end namespace mlfe
#endif // end ifndef __ACCURACY_OP_HPP__