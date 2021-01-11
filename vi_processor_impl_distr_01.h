#ifndef __VI_PROCESSOR_IMPL_DISTR_H
#define __VI_PROCESSOR_IMPL_DISTR_H

#include "vi_processor_base.h"


class VI_Processor_Impl_Distr_01 : public VI_Processor_Base
{
    protected:

    void value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int u_max, 
        const unsigned int s_max, 
        const unsigned int f_max,
        const unsigned int T
    ) override;

    private:

    // Period between data exchanges
    int comm_period;

    public:

    VI_Processor_Impl_Distr_01(const vi_data_args_t& args, const int root_id, int _comm_period = 100)
    : VI_Processor_Base(args, root_id), comm_period(_comm_period)
    {}

    void SetCommPeriod(int comm_period)
    {
        this->comm_period = comm_period;
    }

};


#endif