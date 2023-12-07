#ifndef __SUBPROCESS_PIPE_VAR__
#define __SUBPROCESS_PIPE_VAR__

#include <string>
#include <variant>
#include <iostream>
#include <cstdio>

#include <subprocess/basic_types.h>


namespace subprocess {
    enum class PipeVarIndex {
        option,
        string,
        handle,
        istream,
        ostream,
        file
    };

    typedef std::variant<PipeOption, std::string, PipeHandle,
        std::istream*, std::ostream*, FILE*> PipeVar;


    inline PipeOption get_pipe_option(const PipeVar& option) {
        PipeVarIndex index = static_cast<PipeVarIndex>(option.index());

        switch(index) {
        case PipeVarIndex::option:  return std::get<PipeOption>(option);
        case PipeVarIndex::handle:  return PipeOption::specific;

        default:                    return PipeOption::pipe;
        }
    }
}

#endif // __SUBPROCESS_PIPE_VAR__