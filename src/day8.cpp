#include "input_file_loader.h"

#include <fmt/core.h>
#include <range/v3/all.hpp>
#include <charconv>
#include <optional>
#include <vector>

// TODO: Handle the parsing errors gracefully (lift out of parse)
// TODO: implement shortest modification path solution for part 2

struct Instruction
{
  enum class Type {
    Acc,
    Jmp,
    Noop,
  };

  Type type;
  int argument;
};

std::vector<Instruction> parse(std::istream &&is)
{
  std::vector<Instruction> result;
  for (std::string line; std::getline(is, line);) {
    Instruction::Type type = [&] {
      if (line.starts_with("acc")) {
        return Instruction::Type::Acc;
      } else if (line.starts_with("jmp")) {
        return Instruction::Type::Jmp;
      } else {
        return Instruction::Type::Noop;
      }
    }();
    int arg;
    std::from_chars(line.data() + 5, line.data() + line.size(), arg);
    result.emplace_back(type, arg * (line[4] == '-' ? -1 : 1));
  }
  return result;
}

std::pair<bool, int> run_until_first_loop(const std::vector<Instruction> &program)
{
  std::vector<bool> hit(program.size(), false);
  int acc{ 0 };

  for (int cursor = 0; cursor < program.size();) {
    const auto [t, a] = program[cursor];
    if (hit[cursor]) {
      return std::pair{ false, acc };
    } else {
      hit[cursor] = true;
    }

    if (t == Instruction::Type::Jmp) {
      cursor += a;
    } else if (t == Instruction::Type::Acc) {
      acc += a;
      ++cursor;
    } else {
      ++cursor;
    }
  }

  return std::pair{ true, acc };
}

std::optional<std::vector<Instruction>> fix_program(const std::vector<Instruction> &program)
{
  const auto next_possible_index = [&](auto curr) {
    return std::distance(
      program.begin(),
      std::find_if(
        program.begin() + curr,
        program.end(),
        [](auto el) {
          return el.type == Instruction::Type::Jmp || el.type == Instruction::Type::Noop;
        }));
  };
  auto program_copy = program;
  for (auto i = next_possible_index(0); i < program.size(); i = next_possible_index(i + 1)) {
    program_copy[i].type = program[i].type == Instruction::Type::Noop ? Instruction::Type::Jmp : Instruction::Type::Noop;
    if (run_until_first_loop(program_copy).first) {
      return program_copy;
    } else {
      program_copy[i] = program[i];
    }
  }
  return std::nullopt;
}

int main(int argc, char **argv)
{
  const auto program = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", run_until_first_loop(program).second);
  fmt::print("Part 2: {}\n", run_until_first_loop(*fix_program(program)).second);
}
