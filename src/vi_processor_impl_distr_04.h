// Header of fourth implementation

// be sure class is defined
#ifndef __VI_PROCESSOR_IMPL_DISTR_04_H
#define __VI_PROCESSOR_IMPL_DISTR_04_H
// includes
#include "vi_processor_base.h" // parent class

/**
 * fourth implementation
 */
class VI_Processor_Impl_Distr_04 : public VI_Processor_Base
{
public:
    /**
     * Constructor of fourth implementation
     * @param vi_data_args_t: arguments dictionary with path to respective .npy files [const Dictionary reference]
     * @param root_id: id of root processor [const Integer]
     * @param _comm_period: number of periods to wait before remerging result [Integer]
     */
    VI_Processor_Impl_Distr_04(const vi_data_args_t& args, const int root_id, int _comm_period = 100)
            : VI_Processor_Base(args, root_id), comm_period(_comm_period)
    {}

    /**
     * get name of implementation
     * @return name: name of implementation [String]
     */
    std::string GetName() override;

    /**
     * set parameter of implementation
     * @param param: param name to set [String]
     * @param value: value of param to set [String]
     * @return hasParam: whether implementation has parameters [=true] or not [=false] [Boolean]
     */
    bool SetParameter(std::string param, float value) override;

    /**
     * get parameter of implementation
     * @return parameters: mapped pair of "alpha" and "tolerance" [Pair]
     */
    std::map<std::string, float> GetParameters() override;

private:
    int comm_period; // Period between data exchanges

protected:
    /**
     * actual implementation of inherited abstract function for the communication scheme
     * @param Pi: initialized Pi [Integer vector reference]
     * @param J: initialized J [Float vector reference]
     * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
     * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
     */
    void value_iteration_impl(
            Eigen::Ref<Eigen::VectorXi> Pi,
            Eigen::Ref<Eigen::VectorXf> J,
            const Eigen::Ref<const SpMat_t> P,
            const unsigned int max_iter
    ) override;

};
#endif