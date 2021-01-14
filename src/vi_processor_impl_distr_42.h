#ifndef __VI_PROCESSOR_IMPL_DISTR_42_H
#define __VI_PROCESSOR_IMPL_DISTR_42_H

#include "vi_processor_base.h"


class VI_Processor_Impl_Distr_42 : public VI_Processor_Base
{
    protected:

    void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    ) override;

    private:
    

    public:

    VI_Processor_Impl_Distr_42(const vi_data_args_t& args, const int root_id)
    : VI_Processor_Base(args, root_id)
    {}

    std::string GetName() override;
    
};


#endif