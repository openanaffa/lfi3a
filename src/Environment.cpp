#include "Environment.hpp"
#include <iostream>
#include <stdexcept>

void Environment::define(const std::string &name, const Value &value) {
  values[name] = value;
}

void Environment::assign(const std::string &name, const Value &value) {
  if (values.find(name) != values.end()) {
    values[name] = value;
    return;
  }

  if (parent) {
    parent->assign(name, value);
    return;
  }
  throw std::runtime_error("Undefined variable '" + name +
                           "' during assignment.");
}

Value Environment::get(const std::string &name) {
  if (values.find(name) != values.end()) {
    return values[name];
  }

  if (parent) {
    return parent->get(name);
  }
  throw std::runtime_error("Undefined variable '" + name + "'.");
}
