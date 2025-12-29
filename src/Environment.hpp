#ifndef LFI3A_ENVIRONMENT_HPP
#define LFI3A_ENVIRONMENT_HPP

#include "Value.hpp"
#include <memory>
#include <string>
#include <unordered_map>


class Environment : public std::enable_shared_from_this<Environment> {
public:
  Environment() : parent(nullptr) {}
  Environment(std::shared_ptr<Environment> parent) : parent(parent) {}

  void define(const std::string &name, const Value &value);
  void assign(const std::string &name, const Value &value);
  Value get(const std::string &name);

  std::shared_ptr<Environment> getParent() { return parent; }

private:
  std::shared_ptr<Environment> parent;
  std::unordered_map<std::string, Value> values;
};

#endif
