#pragma once
#include <string>
#include <userver/formats/parse/to.hpp>

namespace dto {

struct Project {
  std::string name;
  std::string description;
};

template <typename Value>
Project Parse(const Value& data, userver::formats::parse::To<Project>) {
  return {.name = data["name"].template As<std::string>(),
          .description = data["description"].template As<std::string>()};
}
}  // namespace dto