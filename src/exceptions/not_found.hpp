#pragma once

#include <stdexcept>
namespace exceptions {
class NotFound : public std::runtime_error {
 public:
  NotFound() : std::runtime_error("Not found error"){};
};

}  // namespace exceptions