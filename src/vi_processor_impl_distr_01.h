#ifndef __VI_PROCESSOR_IMPL_DISTR_01_H
#define __VI_PROCESSOR_IMPL_DISTR_01_H

#include "vi_processor_base.h"


class VI_Processor_Impl_Distr_01 : public VI_Processor_Base
{
    protected:

    void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    ) override;

    private:
    
    int comm_period; // Period between data exchanges

    public:

    VI_Processor_Impl_Distr_01(const vi_processor_args_t& args, const int root_id, int _comm_period = 100)
    : VI_Processor_Base(args, root_id), comm_period(_comm_period)
    {}

    void SetCommPeriod(int comm_period)
    {
        this->comm_period = comm_period;
    }

    std::string GetName() override;

};


#endif