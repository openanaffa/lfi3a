#include "Value.hpp"
#include <iomanip>
#include <sstream>

bool Value::isTruthy() const {
  switch (type) {
  case ValueType::NIL:
    return false;
  case ValueType::BOOLEAN:
    return std::get<bool>(data);
  case ValueType::NUMBER:
    return std::get<double>(data) != 0;
  case ValueType::STRING:
    return !std::get<std::string>(data).empty();
  case ValueType::ARRAY:
    return !std::get<ArrayPtr>(data)->empty();
  default:
    return false;
  }
}

std::string Value::toString() const {
  switch (type) {
  case ValueType::NIL:
    return "wlou";
  case ValueType::BOOLEAN:
    return std::get<bool>(data) ? "s7i7" : "ghalat";
  case ValueType::NUMBER: {
    double val = std::get<double>(data);
    if (val == (long long)val)
      return std::to_string((long long)val);
    std::ostringstream out;
    out << std::setprecision(10) << val;
    return out.str();
  }
  case ValueType::STRING:
    return std::get<std::string>(data);
  case ValueType::ARRAY: {
    auto arr = std::get<ArrayPtr>(data);
    std::string result = "[";
    for (size_t i = 0; i < arr->size(); ++i) {
      result += (*arr)[i].toString();
      if (i < arr->size() - 1)
        result += ", ";
    }
    result += "]";
    return result;
  }
  default:
    return "";
  }
}

double Value::toNumber() const {
  switch (type) {
  case ValueType::NIL:
    return 0;
  case ValueType::BOOLEAN:
    return std::get<bool>(data) ? 1 : 0;
  case ValueType::NUMBER:
    return std::get<double>(data);
  case ValueType::STRING: {
    try {
      return std::stod(std::get<std::string>(data));
    } catch (...) {
      return 0;
    }
  }
  default:
    return 0;
  }
}

Value Value::fromString(const std::string &s) {
  if (s == "s7i7")
    return Value(true);
  if (s == "ghalat")
    return Value(false);
  if (s == "wlou")
    return Value();

  try {
    size_t pos;
    double val = std::stod(s, &pos);
    if (pos == s.length())
      return Value(val);
  } catch (...) {
  }

  return Value(s);
}
