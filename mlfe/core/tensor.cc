#include "tensor.h"
#include "device.h"
#include "op_algo.h"
#include "attribute.h"
#include "graph.h"
#include "gradient_helper.h"
#include "../operators/initializer.h"
#include "../operators/basic_arithmetics.h"
#include "../utils/assert.h"
#include <algorithm>
#include <sstream>

namespace mlfe{

//TODO : use thread for _children_modified
struct Tensor::impl{
    impl() : _exec_order(0), _ctx("unknown"), _children_modified(true){}
    std::vector<Tensor> _parents;
    std::vector<Tensor> _children;
    int _exec_order;
    memory_ptr _mem;
    OpAlgoContext _ctx;
    std::shared_ptr<OpAlgo> _algo;
    std::shared_ptr<Tensor> _gradient;
    Attributes _attrs;
    std::vector<Tensor> _compute_list;
    std::vector<std::shared_ptr<Tensor>> _backward_list;
    bool _children_modified;
};

Tensor::Tensor()
    : _pimpl(std::make_shared<impl>()){
}

Tensor::Tensor(std::string name)
    : Variable(name),
    _pimpl(std::make_shared<impl>()){
}

Tensor::Tensor(std::vector<int> shape)
    : Variable(shape),
    _pimpl(std::make_shared<impl>()){
}

Tensor::~Tensor(){}

void Tensor::add_parent(Tensor p){
    _pimpl->_parents.push_back(p);
    //p.add_child(*this);
    p._pimpl->_children.push_back(*this);
}

void Tensor::add_child(Tensor c){
    _pimpl->_children.push_back(c);
    //c.add_parent(*this);
    c._pimpl->_parents.push_back(*this);
    if(c._pimpl->_exec_order >= _pimpl->_exec_order){
        _pimpl->_exec_order = c._pimpl->_exec_order + 1;
    }
}

std::vector<Tensor> Tensor::get_parents() const{
    return _pimpl->_parents;
}

std::vector<Tensor> Tensor::get_children() const{
    return _pimpl->_children;
}

int Tensor::get_exec_order() const{
    return _pimpl->_exec_order;
}

bool Tensor::operator==(const Tensor &v) const{
    return _pimpl.get() == v._pimpl.get();
}

void Tensor::eval(){
    //compute all children.
    for(auto t : _pimpl->_compute_list){
        if(t._pimpl->_algo != nullptr &&
           t._pimpl->_children_modified
          ){
            t._pimpl->_algo->Compute();
            t._pimpl->_children_modified = false;
            for(int n = 0; n < t._pimpl->_parents.size(); ++n){
                t._pimpl->_parents[n]._pimpl->_children_modified = true;
            }
        }
    }
}

OpAlgoContext Tensor::get_context() const{
    return _pimpl->_ctx;
}

memory_ptr Tensor::get_memory() const{
    return _pimpl->_mem;
}

void Tensor::backprop(){
    if(_pimpl->_backward_list.empty()){
        compute_gradient(*this);
    }
    for(auto &var : _pimpl->_backward_list){
        var->eval();
    }
}

Tensor Tensor::grad(){
    return *_pimpl->_gradient;
}

const void *Tensor::_host_data(){
    return _pimpl->_mem->host_data<void>();
}

void *Tensor::_mutable_host_data(){
    _pimpl->_children_modified = true;
    for(int n = 0; n < _pimpl->_parents.size(); ++n){
        _pimpl->_parents[n]._pimpl->_children_modified = true;
    }
    return _pimpl->_mem->mutable_host_data<void>();
}

const void *Tensor::_device_data(){
    return _pimpl->_mem->device_data<void>();
}

void *Tensor::_mutable_device_data(){
    _pimpl->_children_modified = true;
    for(int n = 0; n < _pimpl->_parents.size(); ++n){
        _pimpl->_parents[n]._pimpl->_children_modified = true;
    }
    return _pimpl->_mem->mutable_device_data<void>();
}

void Tensor::compute_gradient(const Tensor root){
    using TensorUmap = std::unordered_map<Tensor, std::vector<Tensor>>;
    auto make_ptr = [](Tensor &t){
        return std::make_shared<Tensor>(t);
    };
    TensorUmap dy_collector;

    // top-down seuqnce
    auto v_list = visit_bfs(root);
    // sort by execution order.
    std::sort(v_list.begin(), v_list.end(), [](Tensor v1, Tensor v2){
        return v1.get_exec_order() > v2.get_exec_order();
    });
    // root gradient is 1.
    dy_collector[root].push_back(functional::constant(1, root.shape()));
    // set root gradient.
    root._pimpl->_gradient = make_ptr(dy_collector[root][0]);
    //run computing all gradients.
    for(auto &var : v_list){
        if(var._pimpl->_algo != nullptr){
            auto op_name = var._pimpl->_algo->get_name();
            auto helper = GradientHelperRegistry::Get();
            auto op_grad = helper->GetHelper(op_name, nullptr);
            //add all partial gradients and propagate down.
            auto dy = functional::add_n(dy_collector[var]);
            //calculate input's gradients.
            auto input_grad = op_grad->compute_gradient(var, dy);
            //set current variable's gradient, if a partial gradient exists.
            if(dy_collector[var].size() > 1){
                var._pimpl->_gradient = make_ptr(dy);
                root._pimpl->_backward_list.push_back(var._pimpl->_gradient);
            }
            // store all input's gradients.
            for(int n = 0; n < var.get_children().size(); ++n){
                Tensor x = var.get_children()[n];
                Tensor x_grad = input_grad[n];
                dy_collector[x].push_back(x_grad);
                x._pimpl->_gradient = make_ptr(x_grad);
                root._pimpl->_backward_list.push_back(x._pimpl->_gradient);
            }
        }
    }
}

Tensor::AssignOpFunctor::AssignOpFunctor(Tensor t, OpAlgoContext ctx){
    auto reg = OpAlgoRegistry::Get();
    auto dev = get_enabled_device();
    std::string op_name = ctx.get_op_name();
    std::string full_op_name = "Name:" + op_name + "/Device:";
    std::string dev_name = dev->get_device_name();
    std::string with_accel = dev_name + "(" + dev->get_accelerator_name() + ")";
    
    t._pimpl->_compute_list = visit_bfs(t);
    std::reverse(t._pimpl->_compute_list.begin(), 
                 t._pimpl->_compute_list.end()
                );

    ctx.add_output(t);
    t._pimpl->_ctx = ctx;
    if(reg->Has(full_op_name + with_accel)){
        t._pimpl->_algo = reg->GetOpAlgo(full_op_name + with_accel, &ctx);
    }
    else if(reg->Has(full_op_name + dev_name)){
        t._pimpl->_algo = reg->GetOpAlgo(full_op_name + dev_name, &ctx);
    }
    else if(reg->Has(full_op_name + "Any")){
        t._pimpl->_algo = reg->GetOpAlgo(full_op_name + "Any", &ctx);
    }
    else{
        throw std::string(op_name) + " is not supported.";
    }
}


namespace functional{

Tensor create_variable(std::vector<int> shape){
    Tensor var;
    OpAlgoContext ctx("Identity");
    var.reshape(shape);
    var._pimpl->_mem = create_memory(var.size() * var.type().size);
    var._pimpl->_ctx = ctx;
    Tensor::AssignOpFunctor(var, ctx);
    return var;
}

Tensor reshape(Tensor x, std::vector<int> shape){
    Tensor y;
    OpAlgoContext ctx("Reshape");
    y.add_child(x);
    y._pimpl->_algo = nullptr;
    y._pimpl->_gradient = nullptr;
    y._pimpl->_ctx = x._pimpl->_ctx;
    y._pimpl->_mem = x._pimpl->_mem;
    y.reshape(shape);
    Tensor::AssignOpFunctor(y, ctx);
    return y;
}

} // end namespace functional
} // end namespace mlfe

namespace std{

size_t hash<mlfe::Tensor>::operator()(const mlfe::Tensor &v) const{
    return hash<shared_ptr<mlfe::Tensor::impl>>{}(v._pimpl);
}

} // end namespace std
