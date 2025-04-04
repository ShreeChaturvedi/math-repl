#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#if !defined(REPL_USE_LINENOISE)
#define REPL_USE_LINENOISE 0
#endif

#if REPL_USE_LINENOISE
#include "linenoise.h"
#endif

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "repl/evaluator.hpp"
#include "repl/errors.hpp"
#include "repl/state.hpp"

namespace repl::detail {

constexpr std::string_view kHistoryFile = ".repl_history";
constexpr std::size_t kHistoryMax = 200;

std::string trim(std::string_view input) {
    std::size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start])) != 0) {
        ++start;
    }

    std::size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
        --end;
    }

    return std::string{input.substr(start, end - start)};
}

std::string strip_comments(std::string_view input) {
    std::string_view line = input;
    std::size_t hash_pos = line.find('#');
    std::size_t slash_pos = line.find("//");

    std::size_t cut = std::string_view::npos;
    if (hash_pos != std::string_view::npos) {
        cut = hash_pos;
    }
    if (slash_pos != std::string_view::npos) {
        cut = std::min(cut, slash_pos);
    }
    if (cut != std::string_view::npos) {
        line = line.substr(0, cut);
    }
    return trim(line);
}

void clear_screen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

std::string format_variables(const State& state) {
    if (state.vars.empty()) {
        return "No user variables defined.";
    }

    std::vector<std::string> names;
    names.reserve(state.vars.size());
    for (const auto& [name, _] : state.vars) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());

    std::ostringstream out;
    out << "Variables:";
    for (const auto& name : names) {
        out << "\n  " << name << " = " << state.vars.at(name);
    }
    return out.str();
}

std::string format_functions(const State& state) {
    if (state.fns.empty()) {
        return "No user functions defined.";
    }

    std::vector<std::string> names;
    names.reserve(state.fns.size());
    for (const auto& [name, _] : state.fns) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());

    std::ostringstream out;
    out << "Functions:";
    for (const auto& name : names) {
        const auto& fn = state.fns.at(name);
        out << "\n  " << name << '(';
        for (std::size_t index = 0; index < fn.params.size(); ++index) {
            if (index > 0) {
                out << ", ";
            }
            out << fn.params[index];
        }
        out << ')';
    }
    return out.str();
}

std::string format_constants() {
    const auto& values = constants();
    std::vector<std::string> names;
    names.reserve(values.size());
    for (const auto& [name, _] : values) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());

    std::ostringstream out;
    out << "Constants:";
    for (const auto& name : names) {
        out << "\n  " << name << " = " << values.at(name);
    }
    return out.str();
}

std::string format_builtins() {
    const auto& builtins = builtin_functions();
    std::vector<std::string> names;
    names.reserve(builtins.size());
    for (const auto& [name, _] : builtins) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());

    std::ostringstream out;
    out << "Built-in functions:";
    for (const auto& name : names) {
        const auto& spec = builtins.at(name);
        out << "\n  " << name << '/' << spec.arity << " - " << spec.description;
    }
    return out.str();
}

std::string help_text() {
    std::ostringstream out;
    out << "Commands:";
    out << "\n  help            Show this help";
    out << "\n  vars            List user variables";
    out << "\n  fns             List user functions";
    out << "\n  consts          List built-in constants";
    out << "\n  builtins        List built-in functions";
    out << "\n  clear           Clear the screen";
    out << "\n  reset           Clear variables and functions";
    out << "\n  history         Show recent inputs";
    out << "\n  load <file>     Run a script file";
    out << "\n  exit | quit     Exit the REPL";
    out << "\n\nExpressions:";
    out << "\n  +  -  *  /  %  ^";
    out << "\n  <  <=  >  >=  ==  !=";
    out << "\n  a ? b : c";
    out << "\n  f(x) = x * x";
    out << "\n  _   (last result)";
    return out.str();
}

void print_history(const std::vector<std::string>& history) {
    if (history.empty()) {
        std::cout << "No history entries." << '\n';
        return;
    }

    std::size_t start = history.size() > kHistoryMax ? history.size() - kHistoryMax : 0;
    for (std::size_t index = start; index < history.size(); ++index) {
        std::cout << std::setw(4) << (index + 1) << "  " << history[index] << '\n';
    }
}

bool starts_with(std::string_view input, std::string_view prefix) {
    return input.size() >= prefix.size() && input.substr(0, prefix.size()) == prefix;
}

bool run_script(const std::string& path, State& state) {
    std::ifstream file(path);
    if (!file) {
        throw CommandError("Could not open script file");
    }

    std::string line;
    std::size_t line_no = 0;
    while (std::getline(file, line)) {
        ++line_no;
        std::string processed = strip_comments(line);
        if (processed.empty()) {
            continue;
        }
        try {
            EvalResult result = process_query(processed, state);
            if (result.info) {
                std::cout << *result.info << '\n';
            } else if (result.value) {
                std::cout << *result.value << '\n';
            }
        } catch (const std::exception& e) {
            std::cerr << "Script error (line " << line_no << "): " << e.what() << '\n';
            return false;
        }
    }

    return true;
}

bool handle_command(const std::string& line, State& state,
                    std::vector<std::string>& history) {
    if (line == "exit" || line == "quit") {
        return false;
    }
    if (line == "clear") {
        clear_screen();
        return true;
    }
    if (line == "help") {
        std::cout << help_text() << '\n';
        return true;
    }
    if (line == "vars") {
        std::cout << format_variables(state) << '\n';
        return true;
    }
    if (line == "fns") {
        std::cout << format_functions(state) << '\n';
        return true;
    }
    if (line == "consts") {
        std::cout << format_constants() << '\n';
        return true;
    }
    if (line == "builtins") {
        std::cout << format_builtins() << '\n';
        return true;
    }
    if (line == "history") {
        print_history(history);
        return true;
    }
    if (line == "reset") {
        state = State{};
        std::cout << "State cleared." << '\n';
        return true;
    }
    if (starts_with(line, "load ")) {
        std::string path = trim(line.substr(5));
        if (path.empty()) {
            throw CommandError("Usage: load <file>");
        }
        run_script(path, state);
        return true;
    }
    return true;
}

bool is_interactive() {
#if defined(_WIN32)
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(fileno(stdin)) != 0;
#endif
}

bool read_line(std::string& out, bool use_linenoise, bool interactive) {
    if (use_linenoise) {
#if REPL_USE_LINENOISE
        char* raw = linenoise("> ");
        if (!raw) {
            return false;
        }
        out.assign(raw);
        std::free(raw);
        return true;
#else
        return false;
#endif
    }

    if (interactive) {
        std::cout << "> " << std::flush;
    }
    return static_cast<bool>(std::getline(std::cin, out));
}

}  // namespace repl::detail

int main() {
    repl::State state;
    std::vector<std::string> history;

    const bool interactive = repl::detail::is_interactive();
    const bool use_linenoise = interactive && REPL_USE_LINENOISE;

#if REPL_USE_LINENOISE
    if (use_linenoise) {
        linenoiseSetMultiLine(1);
        linenoiseHistorySetMaxLen(static_cast<int>(repl::detail::kHistoryMax));
        linenoiseHistoryLoad(repl::detail::kHistoryFile.data());
    }
#endif

    if (interactive) {
        repl::detail::clear_screen();
        std::cout << "Type 'help' for commands." << '\n';
    }

    while (true) {
        std::string input;
        if (!repl::detail::read_line(input, use_linenoise, interactive)) {
            break;
        }

        std::string processed = repl::detail::strip_comments(input);
        if (processed.empty()) {
            continue;
        }

        if (use_linenoise) {
#if REPL_USE_LINENOISE
            linenoiseHistoryAdd(input.c_str());
            linenoiseHistorySave(repl::detail::kHistoryFile.data());
#endif
        }
        if (interactive) {
            history.push_back(input);
        }

        try {
            if (processed == "exit" || processed == "quit") {
                break;
            }
            if (processed == "help" || processed == "vars" || processed == "fns" ||
                processed == "consts" || processed == "builtins" || processed == "history" ||
                processed == "reset" || processed == "clear" ||
                repl::detail::starts_with(processed, "load ")) {
                if (!repl::detail::handle_command(processed, state, history)) {
                    break;
                }
                continue;
            }

            repl::EvalResult result = repl::process_query(processed, state);
            if (result.info) {
                std::cout << *result.info << '\n';
            } else if (result.value) {
                std::cout << *result.value << '\n';
            }
        } catch (const repl::CommandError& e) {
            std::cerr << "Command error: " << e.what() << '\n';
        } catch (const repl::EvalError& e) {
            std::cerr << "Evaluation error: " << e.what() << '\n';
        } catch (const repl::ParseError& e) {
            std::cerr << "Parse error: " << e.what() << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Unknown error: " << e.what() << '\n';
        }
    }

    return 0;
}
