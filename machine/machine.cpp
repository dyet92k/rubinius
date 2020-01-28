#include "config.h"
#include "paths.h"
#include "debug.h"

#include "machine.hpp"
#include "machine_threads.hpp"
#include "environment.hpp"
#include "configuration.hpp"
#include "config_parser.hpp"
#include "type_info.hpp"
#include "exception.hpp"
#include "thread_nexus.hpp"
#include "memory.hpp"

#include "memory/header.hpp"
#include "memory/collector.hpp"

#include <sys/stat.h>

namespace rubinius {
  Machine::Machine(int argc, char** argv)
    : _machine_state_(nullptr)
    , _logger_(nullptr)
    , _thread_nexus_(new ThreadNexus)
    , _configuration_(new Configuration())
    , _environment_(new Environment(argc, argv, this))
    , _diagnostics_(nullptr)
    , _machine_threads_(new MachineThreads())
    , _memory_(new Memory(_environment_->state, _configuration_))
    , _collector_(new memory::Collector())
    , _signals_(nullptr)
    , _codedb_(nullptr)
    , _c_api_(nullptr)
    , _compiler_(nullptr)
    , _debugger_(nullptr)
    , _profiler_(nullptr)
    , _console_(nullptr)
  {
    _environment_->initialize();
  }

  Machine::~Machine() {
    if(_machine_state_) halt();
  }

  /* TODO:
   *
   * [x] 1. Add Machine to SharedState;
   * [x] 1. Pass Machine through Environment to SharedState
   * [ ] 1. Create MachineState;
   * [x] 1. Move SharedState items for env into Environment;
   * [ ] 1. Create stateful Logger;
   * [ ] 1. Create Random;
   * [x] 1. Move Configuration to Machine;
   * [ ] 1. Consolidate ConfigParse with Configuration;
   * [ ] 1. Move Diagnostics to Machine;
   * [x] 1. Move ThreadNexus to Machine;
   * [x] 1. Move MachineThreads to Machine;
   * [x] 1. Move Memory to Machine;
   * [x] 1. Move Collector to Machine;
   * [ ] 1. Move Signals to Machine;
   * [ ] 1. Move CodeDB to Machine;
   * [ ] 1. Move C-API to Machine;
   * [ ] 1. Move Debugger to Machine;
   * [ ] 1. Move Compiler to Machine;
   * [ ] 1. Move Profiler to Machine;
   * [ ] 1. Move Console to Machine;
   * [x] 1. Move SymbolTable into Memory;
   * [x] 1. Move Globals into Memory;
   * [ ] 1. Create ThreadState to replace State;
   * [ ] 1. Create ThreadState instances for new threads;
   * [ ] 1. Consolidate and remove all extra State objects;
   * [ ] 1. Merge VM into ThreadState;
   * [ ] 1. Switch back to booting on main thread;
   */
  void Machine::boot() {
    environment()->setup_cpp_terminate();

    MachineException::guard(environment()->state, true, [&]{
        if(const char* var = getenv("RBX_OPTIONS")) {
          environment()->load_string(var);
        }

        if(const char* path = getenv("RBX_OPTFILE")) {
          environment()->load_conf(path);
        }

        environment()->boot();
      });
  }

  void Machine::halt_console() {
    if(_console_) {
      delete _console_;
      _console_ = nullptr;
    }
  }

  void Machine::halt_profiler() {
    if(_profiler_) {
      delete _profiler_;
      _profiler_ = nullptr;
    }
  }

  void Machine::halt_debugger() {
    if(_debugger_) {
      delete _debugger_;
      _debugger_ = nullptr;
    }
  }

  void Machine::halt_compiler() {
    if(_compiler_) {
      delete _compiler_;
      _compiler_ = nullptr;
    }
  }

  void Machine::halt_c_api() {
    if(_c_api_) {
      delete _c_api_;
      _c_api_ = nullptr;
    }
  }

  void Machine::halt_codedb() {
    if(_codedb_) {
      delete _codedb_;
      _codedb_ = nullptr;
    }
  }

  void Machine::halt_signals() {
    if(_signals_) {
      delete _signals_;
      _signals_ = nullptr;
    }
  }

  void Machine::halt_collector() {
    if(_collector_) {
      delete _collector_;
      _collector_ = nullptr;
    }
  }

  void Machine::halt_memory() {
    if(_memory_) {
      delete _memory_;
      _memory_ = nullptr;
    }
  }

  void Machine::halt_machine_threads() {
    if(_machine_threads_) {
      delete _machine_threads_;
      _machine_threads_ = nullptr;
    }
  }

  void Machine::halt_thread_nexus() {
    if(_thread_nexus_) {
      // TODO: remove restriction on deleting ThreadNexus
      // delete _thread_nexus_;
      _thread_nexus_ = nullptr;
    }
  }

  void Machine::halt_diagnostics() {
    if(_diagnostics_) {
      delete _diagnostics_;
      _diagnostics_ = nullptr;
    }
  }

  void Machine::halt_configuration() {
    if(_configuration_) {
      delete _configuration_;
      _configuration_ = nullptr;
    }
  }

  void Machine::halt_environment() {
    if(_environment_) {
      delete _environment_;
      _environment_ = nullptr;
    }
  }

  void Machine::halt_logger() {
    if(_logger_) {
      delete _logger_;
      _logger_ = nullptr;
    }
  }

  void Machine::halt_machine_state() {
    if(_machine_state_) {
      delete _machine_state_;
      _machine_state_ = nullptr;
    }
  }

  int Machine::halt() {
    halt_console();
    halt_profiler();
    halt_debugger();
    halt_compiler();
    halt_c_api();
    halt_codedb();
    halt_signals();
    halt_collector();
    halt_memory();
    halt_machine_threads();
    halt_thread_nexus();
    halt_diagnostics();
    halt_configuration();
    halt_environment();
    halt_logger();
    halt_machine_state();

    return 0;
  }
}