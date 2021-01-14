#include "vi_processor_impl_distr_42.h"
#include <limits>
#include <algorithm>
#include <map>
#include <mpi.h>

void VI_Processor_Impl_Distr_42::value_iteration_impl(
        Eigen::Ref<Eigen::VectorXi> Pi, 
        Eigen::Ref<Eigen::VectorXf> J, 
        const Eigen::Ref<const SpMat_t> P, 
        const unsigned int max_iter
    )
{
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Rank of this process

    // Split up states for each process (last process gets additional remainder)
    const int process_first_state = (J.size() / world_size) * world_rank;
    const int process_last_state =  (world_size == world_rank + 1) ? J.size() : (J.size() / world_size) * (world_rank + 1);

    std::vector<int> send_partner_ids;
    std::vector<int> recv_partner_ids;

    for(int i=1; i < world_size; ++i) 
    {
        send_partner_ids.push_back( (world_rank + i) % world_size );
        recv_partner_ids.push_back( (world_size + world_rank - i) % world_size );
    } 

    float error = 0;

    for(unsigned int t=0; t < max_iter; ++t)
    {

        // Compute one value iteration step over all states
        float max_diff = iteration_step(Pi, J, P, process_first_state, process_last_state);

        if(max_diff > error) error = max_diff;

        // If we are running with a single process there is no partner
        int partner_idx = -1;

        if(world_size > 1)
        { 
            // Code for the multi-process case
            
            partner_idx = t % (world_size - 1);

            std::vector<float> recv_buffer(J.size() / world_size + J.size() % world_size + 1);
            if(recv_partner_ids[partner_idx] != world_size - 1)
            {
                recv_buffer.resize(J.size() / world_size + 1);
            }

            std::vector<float> send_buffer(process_last_state - process_first_state);
            send_buffer.assign(J.data() + process_first_state, J.data() + process_last_state);
            send_buffer.push_back(error);

            MPI_Status sendrecv_status;
            MPI_Sendrecv(   send_buffer.data(),
                            send_buffer.size(),
                            MPI_FLOAT,
                            send_partner_ids[partner_idx],
                            0,
                            recv_buffer.data(),
                            recv_buffer.size(),
                            MPI_FLOAT,
                            recv_partner_ids[partner_idx],
                            0,
                            MPI_COMM_WORLD,
                            &sendrecv_status);

            float error_recv = recv_buffer.back();

            if(error_recv > error) error = error_recv;

            Eigen::Map<Eigen::VectorXf> J_recv(recv_buffer.data(), recv_buffer.size() - 1);
            J.segment(recv_partner_ids[partner_idx] * (J.size() / world_size), J_recv.size()) = J_recv;
        }


        if(partner_idx == world_size - 2) 
        {
            if(error <= tolerance)
            {   
                debug_message("Converged after " + std::to_string(t) + " iterations");
                break;
            } 
            error = 0;
        } 

    }


    // Merge Policy
    std::vector<int> recvcounts;    // Number of states for each process
    std::vector<int> displs;        // Displacement of sub vectors for each process
    for(int i=0; i < world_size; ++i)
    {
        if(i+1 < world_size) recvcounts.push_back((J.size() / world_size));
        else recvcounts.push_back((J.size() / world_size) + J.size() % world_size);

        if(i == 0) displs.push_back(0);
        else displs.push_back(displs[i-1] + recvcounts[i-1]);
    }
    int* Pi_raw = Pi.data();
    MPI_Gatherv(&Pi_raw[process_first_state], 
                process_last_state - process_first_state,
                MPI_INT, 
                Pi_raw, 
                recvcounts.data(), 
                displs.data(), 
                MPI_INT, 
                root_id, 
                MPI_COMM_WORLD
    );

}

std::string VI_Processor_Impl_Distr_42::GetName()
{
    return VI_Processor_Base::GetName();
}