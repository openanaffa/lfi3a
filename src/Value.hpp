#ifndef LFI3A_VALUE_HPP
#define LFI3A_VALUE_HPP

#include <memory>
#include <string>
#include <variant>
#include <vector>

enum class ValueType { NIL, BOOLEAN, NUMBER, STRING, ARRAY };

class Value;
using ArrayPtr = std::shared_ptr<std::vector<Value>>;

class Value {
public:
  ValueType type;
  std::variant<std::monostate, bool, double, std::string, ArrayPtr> data;

  Value() : type(ValueType::NIL), data(std::monostate{}) {}
  Value(bool val) : type(ValueType::BOOLEAN), data(val) {}
  Value(double val) : type(ValueType::NUMBER), data(val) {}
  Value(const std::string &val) : type(ValueType::STRING), data(val) {}
  Value(const char *val) : type(ValueType::STRING), data(std::string(val)) {}
  Value(ArrayPtr val) : type(ValueType::ARRAY), data(val) {}

  bool isTruthy() const;
  std::string toString() const;
  double toNumber() const;

  static Value fromString(const std::string &s);
};

#endif
