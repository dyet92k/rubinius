#include "config.h"
#include "vm.hpp"
#include "immix_marker.hpp"

#include "builtin/class.hpp"
#include "builtin/thread.hpp"
#include "configuration.hpp"
#include "gc/gc.hpp"
#include "gc/immix.hpp"
#include "ontology.hpp"
#include "metrics.hpp"

#include "dtrace/dtrace.h"
#include "instruments/timing.hpp"

namespace rubinius {

  ImmixMarker::ImmixMarker(STATE, ImmixGC* immix)
    : AuxiliaryThread(state, "rbx.immix")
    , immix_(immix)
    , data_(NULL)
  {
    state->memory()->set_immix_marker(this);

    initialize(state);
    start_thread(state);
  }

  ImmixMarker::~ImmixMarker() {
    if(data_) {
      delete data_;
      data_ = NULL;
    }
  }

  void ImmixMarker::initialize(STATE) {
    AuxiliaryThread::initialize(state);

    run_lock_.init();
    run_cond_.init();
  }

  void ImmixMarker::wakeup(STATE) {
    utilities::thread::Mutex::LockGuard lg(run_lock_);

    AuxiliaryThread::wakeup(state);

    run_cond_.signal();
  }

  void ImmixMarker::after_fork_child(STATE) {
    if(data_) {
      delete data_;
      data_ = NULL;
    }

    AuxiliaryThread::after_fork_child(state);
  }

  void ImmixMarker::concurrent_mark(GCData* data) {
    utilities::thread::Mutex::LockGuard lg(run_lock_);
    data_ = data;
    run_cond_.signal();
  }

  void ImmixMarker::run(STATE) {
    GCTokenImpl gct;

    metrics().init(metrics::eRubyMetrics);

    while(!thread_exit_) {
      if(data_) {
        {
          timer::StopWatch<timer::milliseconds> timer(
              state->vm()->metrics().m.ruby_metrics.gc_immix_conc_last_ms,
              state->vm()->metrics().m.ruby_metrics.gc_immix_conc_total_ms
            );

          // Allow for a young stop the world GC to occur
          // every bunch of marks. 100 is a fairly arbitrary
          // number, based mostly on the fact it didn't cause
          // big increases in young gc times because of long
          // stop the world wait times.
          while(immix_->process_mark_stack(100)) {
            state->gc_independent(gct, 0);
            state->gc_dependent(gct, 0);
          }
        }

        if(thread_exit_) break;

        {
          timer::StopWatch<timer::milliseconds> timer(
              state->vm()->metrics().m.ruby_metrics.gc_immix_stop_last_ms,
              state->vm()->metrics().m.ruby_metrics.gc_immix_stop_total_ms
            );

          // Finish and pause
          while(!state->stop_the_world()) {
            if(thread_exit_) {
              state->restart_world();
              break;
            }
            state->checkpoint(gct, 0);
          }
          state->memory()->clear_mature_mark_in_progress();
          state->memory()->collect_mature_finish(state, data_);
        }
        state->restart_world();
      }

      {
        utilities::thread::Mutex::LockGuard lg(run_lock_);
        if(data_) {
          delete data_;
          data_ = NULL;
        }
        if(thread_exit_) break;
        state->gc_independent(gct, 0);
        run_cond_.wait(run_lock_);
      }
      state->gc_dependent(gct, 0);
    }

    state->memory()->clear_mature_mark_in_progress();
  }
}
