#ifndef RBX_THREAD_STATE_HPP
#define RBX_THREAD_STATE_HPP

#include "raise_reason.hpp"
#include <functional>
#include <iostream>

namespace rubinius {
  class Object;
  class Exception;
  class VariableScope;
  class VM;
  class ThreadState;

  class VMThreadState {
    Exception* current_exception_;
    Object* raise_value_;
    Object* throw_dest_;
    VariableScope* destination_scope_;
    RaiseReason raise_reason_;

  public:
    VMThreadState(VM* state);

    Exception* current_exception() {
      return current_exception_;
    }

    void set_current_exception(Exception* exc) {
      current_exception_ = exc;
    }

    Object* raise_value() const {
      return raise_value_;
    }

    RaiseReason raise_reason() const {
      return raise_reason_;
    }

    VariableScope* destination_scope() const {
      return destination_scope_;
    }

    Object* throw_dest() const {
      return throw_dest_;
    }

    void clear_break();
    void clear_return();
    void clear_raise();
    void clear();

    ThreadState* state_as_object(STATE);
    void set_state(STATE, ThreadState* obj);
    void set_state(VMThreadState* thread_state);

    void raise_exception(Exception* exc);
    void raise_return(Object* value, VariableScope* dest);
    void raise_break(Object* value, VariableScope* dest);
    void raise_exit(Object* code);
    void raise_throw(Object* dest, Object* value);
    void raise_thread_kill();
    void raise_fiber_cancel();

    void gc_scan(STATE, std::function<void (STATE, Object**)> f);
  };
};

#endif
