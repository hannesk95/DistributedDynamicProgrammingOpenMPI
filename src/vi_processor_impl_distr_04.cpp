// fourth implementation, using Isend, Irecv, Ibcast and Igatherv

// includes
#include "vi_processor_impl_distr_04.h" // header
#include <mpi.h> // openmpi support
#include <math.h> // for math calculations

/**
 * define helper function to check if an error occured, print it then
 */
#define MPI_Error_Check(x) {const int err=x; if(x!=MPI_SUCCESS) { fprintf(stderr, "MPI ERROR %d at %d.", err, __LINE__);}}

/**
 * get name of implementation
 * @return name: name of implementation [String]
 */
std::string VI_Processor_Impl_Distr_04::GetName()
{
    return VI_Processor_Base::GetName() + "-" + std::to_string(comm_period);
}

/**
 * set parameter of implementation
 * @param param: param name to set [String]
 * @param value: value of param to set [String]
 * @return hasParam: whether implementation has parameters [=true] or not [=false] [Boolean]
 */
bool VI_Processor_Impl_Distr_04::SetParameter(std::string param, float value)
{
    if(param == "comm_period")
    {
        comm_period = value;
        return true;
    }
    return VI_Processor_Base::SetParameter(param, value);
}

/**
 * get parameter of implementation
 * @return parameters: mapped pair of "alpha" and "tolerance" [Pair]
 */
std::map<std::string, float> VI_Processor_Impl_Distr_04::GetParameters()
{
    std::map<std::string, float> parameters = VI_Processor_Base::GetParameters();
    parameters["comm_period"] = comm_period;
    return parameters;
}

/**
 * actual implementation of inherited abstract function for the communication scheme
 * @param Pi_out: initialized Pi [Integer vector reference]
 * @param J_out: initialized J [Float vector reference]
 * @param P: probability matrix [const Eigen SparseMatrix<RowMajor> reference]
 * @param max_iter: maximum number of iterations before stopping anyway [const unsigned Integer]
 */
void VI_Processor_Impl_Distr_04::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int T)
{
    // init openmpi parameters
    int world_size, world_rank;
    MPI_Error_Check(MPI_Comm_size(MPI_COMM_WORLD, &world_size));
    MPI_Error_Check(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));

    // init the statusses of messages
    MPI_Status status;
    MPI_Request request;

    // get parameters for size of message
    int processor_workload = ceil(float(J.size()) / world_size);
    int processor_workload_last = J.size() - (world_size - 1)*processor_workload;
    // Split up states for each process (last process has a less data)
    int processor_start = processor_workload * world_rank;
    int processor_end = processor_workload * (world_rank +1);

    // last processes end is size of J
    if (world_rank == world_size - 1)
    {
        processor_end = J.size();
    }

    // init buffer for received J and Pi
    std::vector<float> J_buffer(processor_workload);
    std::vector<float> Pi_buffer(processor_workload);

    // Keeps track of the change in J vector
    float error = 0;

    for(unsigned int t=0; t < T; ++t)
    {
        // Compute one value iteration step for a range of states
        float max_diff = iteration_step(Pi, J, P, processor_start, processor_end);

        // Store value of biggest change that appeared while updating J
        if(max_diff > error)
        {
            error = max_diff;
        }

        // Exchange results every comm_period cycles
        if(t % comm_period == 0)
        {
            // all non-roots are sending to root
            if ( world_rank != root_id)
            {
                // calculate size of message
                int send_workload = processor_workload;
                // last processors message is smaller
                if( world_rank == world_size - 1)
                {
                    send_workload = processor_workload_last;
                }

                float* J_raw = J.data();
                MPI_Isend(J_raw + processor_start,
                        send_workload,
                        MPI_FLOAT,
                        root_id,
                        1,
                        MPI_COMM_WORLD,
                        &request);
                ///////////////////////////////////////////////
                // Do some other computations here if needed //
                ///////////////////////////////////////////////
                MPI_Error_Check(MPI_Wait(&request, &status));
            }
            // root receives
            else if ( world_rank == root_id)
            {
                // receive from all
                for (int i = 0; i < world_size - 1; i++)
                {
                    // Wait for the first message that occurs
                    MPI_Error_Check(MPI_Probe(MPI_ANY_SOURCE,
                                    1,
                                    MPI_COMM_WORLD,
                                    &status));
                    int source_rank = status.MPI_SOURCE;
                    // get payload size
                    int recv_workload = processor_workload;
                    // last processors message is smaller
                    if (source_rank == world_size - 1)
                    {
                        recv_workload = processor_workload_last;
                    }
                    // now receive the sub vector into J receive buffer
                    MPI_Irecv(J_buffer.data(),
                          recv_workload,
                          MPI_FLOAT,
                          source_rank,
                          1,
                          MPI_COMM_WORLD,
                          &request);
                    ///////////////////////////////////////////////
                    // Do some other computations here if needed //
                    ///////////////////////////////////////////////
                    MPI_Wait(&request, &status);

                    // Generate Eigen style vector of received data
                    Eigen::Map<Eigen::VectorXf> J_sub(J_buffer.data(), recv_workload);

                    // get difference between J_recv and corresponding segment in J
                    auto vector = J.segment(source_rank * processor_workload, recv_workload);
                    float recv_max_diff = (vector - J_sub).cwiseAbs().maxCoeff();
                    // check if new maximum difference -> if yes overwrite
                    if (recv_max_diff > error)
                    {
                        error = recv_max_diff;
                    }

                    J.segment(source_rank * processor_workload, recv_workload) = J_sub;
                    // update J
                    MPI_Wait(&request, &status); // Blocks and waits for destination process to receive data
                }
            }
            else throw std::runtime_error("Something strange happened!");
            // Broadcast error from root to all processes (that they can stop respectively), also synchronizes all processes
            MPI_Ibcast(&error, 1, MPI_FLOAT, root_id, MPI_COMM_WORLD, &request);
            ///////////////////////////////////////////////
            // Do some other computations here if needed //
            ///////////////////////////////////////////////
            MPI_Wait(&request, &status);
            // If convergence rate is below threshold stop
            if(error <= tolerance)
            {
                debug_message("Converged after " + std::to_string(t) + " iterations with communication period " + std::to_string(comm_period));
                break;
            }
            // Broadcast J again from root to all processes because we did not converge yet
            MPI_Ibcast(J.data(), J.size(), MPI_FLOAT, root_id, MPI_COMM_WORLD, &request);
            ///////////////////////////////////////////////
            // Do some other computations here if needed //
            ///////////////////////////////////////////////
            MPI_Wait(&request, &status);
            // Reset change rate to 0 such that it can be overwritten with a new (smaller) value
            error = 0;
        }
    }
    // Merge Policy

    // set number of states per process and displacements of subvectors
    std::vector<int> recvcounts;    // Number of states for each process
    std::vector<int> displs;        // Displacement of sub vectors for each process
    for(int i=0; i < world_size; i++)
    {
        if(i == world_size - 1)
        {
            recvcounts.push_back(processor_workload_last);
            displs.push_back(i*processor_workload);
        }
        else
        {
            recvcounts.push_back(processor_workload);
            displs.push_back(i*processor_workload);
        }
    }

    int* Pi_raw = Pi.data();
    MPI_Igatherv(&Pi_raw[processor_start],
            processor_end - processor_start,
            MPI_INT,
            Pi_raw,
            recvcounts.data(),
            displs.data(),
            MPI_INT,
            root_id,
            MPI_COMM_WORLD,
            &request);

    ///////////////////////////////////////////////
    // Do some other computations here if needed //
    ///////////////////////////////////////////////

    MPI_Wait(&request, &status);
}
