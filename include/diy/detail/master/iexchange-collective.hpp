namespace diy
{
    struct Master::IExchangeInfoCollective: public IExchangeInfo
    {
                        IExchangeInfoCollective(mpi::communicator comm_, size_t min_queue_size, size_t max_hold_time, bool fine):
                            IExchangeInfo(comm_, fine, min_queue_size, max_hold_time)       { time_stamp_send(); }

      inline bool       all_done() override;                    // get global all done status
      inline void       add_work(int work) override;            // add work to global work counter
      inline void       control() override;

      Time              consensus_start_time() override         { return ibarrier_start_time; }

      int               local_work_ = 0;
      int               dirty = 0;
      int               local_dirty, all_dirty;

      int               state = 0;
      mpi::request      r;

      // debug
      Time              ibarrier_start_time;
      bool              first_ibarrier = true;
    };
}

bool
diy::Master::IExchangeInfoCollective::
all_done()
{
    return state == 3;
}

void
diy::Master::IExchangeInfoCollective::
add_work(int work)
{
    local_work_ += work;
    if (local_work_ > 0)
        dirty = 1;
}

void
diy::Master::IExchangeInfoCollective::
control()
{
    if (state == 0 && local_work_ == 0)
    {
        // debug
        if (first_ibarrier)
        {
            ibarrier_start_time = Clock::now();
            first_ibarrier = false;
        }

        r = ibarrier(comm);
        dirty = 0;
        state = 1;
    } else if (state == 1)
    {
        mpi::optional<mpi::status> ostatus = r.test();
        if (ostatus)
        {
            local_dirty = dirty;
            r = mpi::iall_reduce(comm, local_dirty, all_dirty, std::logical_or<int>());
            state = 2;
        }
    } else if (state == 2)
    {
        mpi::optional<mpi::status> ostatus = r.test();
        if (ostatus)
        {
            if (all_dirty == 0)     // done
                state = 3;
            else
                state = 0;          // reset
        }
    }
}

