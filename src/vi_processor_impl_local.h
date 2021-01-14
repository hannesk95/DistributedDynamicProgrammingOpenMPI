#ifndef __VI_PROCESSOR_IMPL_LOCAL_H
#define __VI_PROCESSOR_IMPL_LOCAL_H

#include "vi_processor_base.h"


class VI_Processor_Impl_Local : public VI_Processor_Base
{
    protected:

    void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    ) override;

    public:

    VI_Processor_Impl_Local(const vi_processor_args_t& args, const int root_id)
    : VI_Processor_Base(args, root_id)
    {}

    std::string GetName() override;

};


#endif