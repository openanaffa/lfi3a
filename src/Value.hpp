#ifndef LFI3A_VALUE_HPP
#define LFI3A_VALUE_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <map>
enum class ValueType { NIL, BOOLEAN, NUMBER, STRING, ARRAY, MAP, NATIVE_FUNC };


class Value;
using ArrayPtr = std::shared_ptr<std::vector<Value>>;
using MapPtr = std::shared_ptr<std::map<std::string, Value>>;
using NativeFunc = std::function<Value(const std::vector<Value> &)>;

class Value {
public:
  ValueType type;
  std::variant<std::monostate, bool, double, std::string, ArrayPtr, MapPtr, NativeFunc>
      data;

  Value() : type(ValueType::NIL), data(std::monostate{}) {}
  Value(bool val) : type(ValueType::BOOLEAN), data(val) {}
  Value(double val) : type(ValueType::NUMBER), data(val) {}
  Value(const std::string &val) : type(ValueType::STRING), data(val) {}
  Value(const char *val) : type(ValueType::STRING), data(std::string(val)) {}
  Value(ArrayPtr val) : type(ValueType::ARRAY), data(val) {}
  Value(MapPtr val) : type(ValueType::MAP), data(val) {}
  Value(NativeFunc val) : type(ValueType::NATIVE_FUNC), data(val) {}

  bool isTruthy() const;
  std::string toString() const;
  double toNumber() const;

  static Value fromString(const std::string &s);
};

#endif
