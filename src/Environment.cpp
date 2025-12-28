#include "Environment.hpp"
#include <iostream>

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

  std::cerr << "Error: Undefined variable '" << name
            << "' during assignment.\n";
  exit(1);
}

Value Environment::get(const std::string &name) {
  if (values.find(name) != values.end()) {
    return values[name];
  }

  if (parent) {
    return parent->get(name);
  }

  std::cerr << "Error: Undefined variable '" << name << "'.\n";
  exit(1);
}
