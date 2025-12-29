#include "Interpreter.hpp"
#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

Interpreter::Interpreter() {
  globals = std::make_shared<Environment>();
  environment = globals;
  registerBuiltins();
}

void Interpreter::run(const std::vector<ASTNodePtr> &nodes,
                      const std::string &currentDir) {
  currentDirectory = currentDir;
  for (const auto &node : nodes) {
    if (hasReturned)
      break;
    execute(node);
  }
}

void Interpreter::execute(const ASTNodePtr &node) {
  if (!node)
    return;

  switch (node->type) {
  case NodeType::VAR_DECL: {
    Value value = evaluate(node->children[0]);
    environment->define(node->value, value);
    break;
  }

  case NodeType::ASSIGNMENT: {
    ASTNodePtr target = node->children[0];
    Value val = evaluate(node->children[1]);

    if (target->type == NodeType::IDENTIFIER) {
      try {
        environment->assign(target->value, val);
      } catch (const std::runtime_error &e) {
        ErrorHandler::fatal(node->line, node->column, e.what());
      }
    } else if (target->type == NodeType::ARRAY_INDEX) {
      Value arrValue = evaluate(target->children[0]);
      Value indexValue = evaluate(target->children[1]);

      if (arrValue.type != ValueType::ARRAY) {
        ErrorHandler::fatal(target->line, target->column,
                            "Indexing non-array value.");
      }
      if (indexValue.type != ValueType::NUMBER) {
        ErrorHandler::fatal(target->line, target->column,
                            "Index must be a number.");
      }

      auto arr = std::get<ArrayPtr>(arrValue.data);
      int index = (int)indexValue.toNumber();

      if (index < 0 || index >= (int)arr->size()) {
        ErrorHandler::fatal(target->line, target->column,
                            "Array index out of bounds.");
      }

      (*arr)[index] = val;
    }
    break;
  }

  case NodeType::PRINT: {
    for (size_t i = 0; i < node->children.size(); ++i) {
      if (i > 0)
        std::cout << " ";
      std::cout << evaluate(node->children[i]).toString();
    }
    std::cout << std::endl;
    break;
  }

  case NodeType::IF: {
    Value condValue = evaluate(node->children[0]);
    if (condValue.isTruthy()) {
      execute(node->children[1]);
    } else {
      for (size_t i = 2; i < node->children.size(); ++i) {
        auto &child = node->children[i];
        if (child->type == NodeType::IF) {
          Value elifCond = evaluate(child->children[0]);
          if (elifCond.isTruthy()) {
            execute(child->children[1]);
            return;
          }
        } else if (child->type == NodeType::BLOCK) {
          execute(child);
          return;
        }
      }
    }
    break;
  }

  case NodeType::WHILE: {
    while (evaluate(node->children[0]).isTruthy()) {
      execute(node->children[1]);
      if (hasReturned)
        break;
    }
    break;
  }

  case NodeType::FOR: {
    // Create a new scope for the for loop
    auto previous = environment;
    environment = std::make_shared<Environment>(previous);

    // Handle initialization: if it's an assignment, define it in this new scope
    auto initNode = node->children[0];
    if (initNode && initNode->type == NodeType::ASSIGNMENT) {
      Value val = evaluate(initNode->children[0]);
      environment->define(initNode->value, val);
    } else {
      execute(initNode);
    }

    while (evaluate(node->children[1]).isTruthy()) { // condition
      execute(node->children[3]);                    // body
      if (hasReturned)
        break;
      evaluate(node->children[2]); // increment
    }

    environment = previous;
    break;
  }

  case NodeType::FUNCTION_DECL: {
    functions[node->value] = node;
    break;
  }

  case NodeType::RETURN: {
    if (!node->children.empty()) {
      returnValue = evaluate(node->children[0]);
    } else {
      returnValue = Value();
    }
    hasReturned = true;
    break;
  }

  case NodeType::BLOCK: {
    executeBlock(node, std::make_shared<Environment>(environment));
    break;
  }

  case NodeType::IMPORT: {
    loadModule(node->value);
    break;
  }

  case NodeType::IMPORT_FROM: {
    loadModuleItem(node->value, node->params[0]);
    break;
  }

  default:
    break;
  }
}

void Interpreter::executeBlock(const ASTNodePtr &block,
                               std::shared_ptr<Environment> env) {
  auto previous = this->environment;
  this->environment = env;

  for (const auto &stmt : block->children) {
    execute(stmt);
    if (hasReturned)
      break;
  }

  this->environment = previous;
}

Value Interpreter::evaluate(const ASTNodePtr &node) {
  if (!node)
    return Value();

  switch (node->type) {
  case NodeType::NUMBER:
    return Value(std::stod(node->value));

  case NodeType::STRING:
    return Value(node->value);

  case NodeType::BOOLEAN:
    return Value(node->value == "s7i7");

  case NodeType::IDENTIFIER: {
    try {
      return environment->get(node->value);
    } catch (const std::runtime_error &e) {
      ErrorHandler::fatal(node->line, node->column, e.what());
      return Value(); // unreachable
    }
  }

  case NodeType::ARRAY_LITERAL: {
    auto arr = std::make_shared<std::vector<Value>>();
    for (const auto &child : node->children) {
      arr->push_back(evaluate(child));
    }
    return Value(arr);
  }

  case NodeType::ARRAY_INDEX: {
    Value arrValue = evaluate(node->children[0]);
    Value indexValue = evaluate(node->children[1]);

    if (arrValue.type != ValueType::ARRAY) {
      ErrorHandler::fatal(node->line, node->column,
                          "Indexing non-array value.");
    }
    if (indexValue.type != ValueType::NUMBER) {
      ErrorHandler::fatal(node->line, node->column, "Index must be a number.");
    }

    auto arr = std::get<ArrayPtr>(arrValue.data);
    int index = (int)indexValue.toNumber();

    if (index < 0 || index >= (int)arr->size()) {
      ErrorHandler::fatal(node->line, node->column,
                          "Array index out of bounds: " +
                              std::to_string(index));
    }

    return (*arr)[index];
  }

  case NodeType::BINARY_OP: {
    Value left = evaluate(node->children[0]);
    Value right = evaluate(node->children[1]);
    std::string op = node->op;

    if (op == "+") {
      if (left.type == ValueType::NUMBER && right.type == ValueType::NUMBER) {
        return Value(std::get<double>(left.data) +
                     std::get<double>(right.data));
      }
      if (left.type == ValueType::STRING || right.type == ValueType::STRING) {
        return Value(left.toString() + right.toString());
      }
      return Value(left.toNumber() + right.toNumber());
    } else if (op == "-") {
      return Value(left.toNumber() - right.toNumber());
    } else if (op == "*") {
      return Value(left.toNumber() * right.toNumber());
    } else if (op == "/") {
      double l = left.toNumber();
      double r = right.toNumber();
      if (r == 0) {
        ErrorHandler::fatal(node->line, node->column, "Division by zero");
      }
      if (l == (long long)l && r == (long long)r) {
        return Value((double)((long long)l / (long long)r));
      }
      return Value(l / r);
    } else if (op == "==") {
      if (left.type != right.type)
        return Value(false);
      if (left.type == ValueType::NUMBER)
        return Value(std::get<double>(left.data) ==
                     std::get<double>(right.data));
      if (left.type == ValueType::STRING)
        return Value(std::get<std::string>(left.data) ==
                     std::get<std::string>(right.data));
      if (left.type == ValueType::BOOLEAN)
        return Value(std::get<bool>(left.data) == std::get<bool>(right.data));
      return Value(true); // NIL == NIL
    } else if (op == "!=") {
      if (left.type != right.type)
        return Value(true);
      if (left.type == ValueType::NUMBER)
        return Value(std::get<double>(left.data) !=
                     std::get<double>(right.data));
      if (left.type == ValueType::STRING)
        return Value(std::get<std::string>(left.data) !=
                     std::get<std::string>(right.data));
      if (left.type == ValueType::BOOLEAN)
        return Value(std::get<bool>(left.data) != std::get<bool>(right.data));
      return Value(false); // NIL != NIL is false
    } else if (op == "<") {
      return Value(left.toNumber() < right.toNumber());
    } else if (op == ">") {
      return Value(left.toNumber() > right.toNumber());
    } else if (op == "<=") {
      return Value(left.toNumber() <= right.toNumber());
    } else if (op == ">=") {
      return Value(left.toNumber() >= right.toNumber());
    } else if (op == "w") {
      return Value(left.isTruthy() && right.isTruthy());
    } else if (op == "wla") {
      return Value(left.isTruthy() || right.isTruthy());
    }
    break;
  }

  case NodeType::UNARY_OP: {
    Value operand = evaluate(node->children[0]);
    std::string op = node->op;

    if (op == "-") {
      return Value(-operand.toNumber());
    } else if (op == "post++") {
      if (node->children[0]->type == NodeType::IDENTIFIER) {
        double val = operand.toNumber();
        environment->assign(node->children[0]->value, Value(val + 1));
        return Value(val);
      }
    }
    break;
  }

  case NodeType::CALL: {
    auto it = functions.find(node->value);
    if (it != functions.end()) {
      auto funcNode = it->second;

      // Create a new environment for the function call
      // Note: Currently functions link to globals, but for true lexical scoping
      // we should capture the environment where the function was defined.
      // For now, since all functions are global, globals as parent is fine.
      auto callEnv = std::make_shared<Environment>(globals);

      for (size_t i = 0;
           i < funcNode->params.size() && i < node->children.size(); ++i) {
        callEnv->define(funcNode->params[i], evaluate(node->children[i]));
      }

      auto previousEnv = this->environment;
      bool savedHasReturned = hasReturned;
      Value savedReturnValue = returnValue;

      hasReturned = false;
      returnValue = Value();

      executeBlock(funcNode->body, callEnv);

      Value result = returnValue;

      this->environment = previousEnv;
      hasReturned = savedHasReturned;
      returnValue = savedReturnValue;

      return result;
    }

    // Check for native functions in the environment
    try {
      Value val = environment->get(node->value);
      if (val.type == ValueType::NATIVE_FUNC) {
        std::vector<Value> args;
        for (const auto &child : node->children) {
          args.push_back(evaluate(child));
        }
        return std::get<NativeFunc>(val.data)(args);
      }
    } catch (...) {
      // Not in environment, could be a built-in that we handle below (legacy)
    }

    void Interpreter::registerBuiltin(const std::string &name,
                                      NativeFunc func) {
      globals->define(name, Value(func));
    }

    void Interpreter::registerBuiltins() {
      // Essentials
      registerBuiltin("tul", [](const std::vector<Value> &args) -> Value {
        if (args.empty())
          return Value(0.0);
        if (args[0].type == ValueType::STRING) {
          return Value((double)std::get<std::string>(args[0].data).length());
        } else if (args[0].type == ValueType::ARRAY) {
          return Value((double)std::get<ArrayPtr>(args[0].data)->size());
        }
        return Value(0.0);
      });

      registerBuiltin("naw3", [](const std::vector<Value> &args) -> Value {
        if (args.empty())
          return Value("nil");
        switch (args[0].type) {
        case ValueType::NUMBER:
          return Value("number");
        case ValueType::STRING:
          return Value("string");
        case ValueType::BOOLEAN:
          return Value("bool");
        case ValueType::ARRAY:
          return Value("array");
        case ValueType::NIL:
          return Value("nil");
        default:
          return Value("unknown");
        }
      });

      registerBuiltin("ra9m", [](const std::vector<Value> &args) -> Value {
        if (args.empty())
          return Value(0.0);
        return Value(args[0].toNumber());
      });

      registerBuiltin("kelma", [](const std::vector<Value> &args) -> Value {
        if (args.empty())
          return Value("");
        return Value(args[0].toString());
      });

      registerBuiltin("wa9t", [](const std::vector<Value> &args) -> Value {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch())
                      .count();
        return Value((double)ms);
      });

      // Math Library
      registerBuiltin("motla9",
                      [](const std::vector<Value> &args) -> Value { // abs
                        if (args.empty())
                          return Value(0.0);
                        return Value(std::abs(args[0].toNumber()));
                      });

      registerBuiltin("dwer",
                      [](const std::vector<Value> &args) -> Value { // round
                        if (args.empty())
                          return Value(0.0);
                        return Value(std::round(args[0].toNumber()));
                      });

      registerBuiltin("ls9ef",
                      [](const std::vector<Value> &args) -> Value { // ceil
                        if (args.empty())
                          return Value(0.0);
                        return Value(std::ceil(args[0].toNumber()));
                      });

      registerBuiltin("l9a3",
                      [](const std::vector<Value> &args) -> Value { // floor
                        if (args.empty())
                          return Value(0.0);
                        return Value(std::floor(args[0].toNumber()));
                      });

      registerBuiltin("jdr",
                      [](const std::vector<Value> &args) -> Value { // sqrt
                        if (args.empty())
                          return Value(0.0);
                        return Value(std::sqrt(args[0].toNumber()));
                      });

      registerBuiltin("os", [](const std::vector<Value> &args) -> Value { // pow
        if (args.size() < 2)
          return Value(0.0);
        return Value(std::pow(args[0].toNumber(), args[1].toNumber()));
      });

      registerBuiltin("3chwa2i",
                      [](const std::vector<Value> &args) -> Value { // rand
                        return Value((double)std::rand() / RAND_MAX);
                      });

      registerBuiltin(
          "asgher", [](const std::vector<Value> &args) -> Value { // min
            if (args.size() < 2)
              return args.empty() ? Value(0.0) : args[0];
            return Value(std::min(args[0].toNumber(), args[1].toNumber()));
          });

      registerBuiltin(
          "akber", [](const std::vector<Value> &args) -> Value { // max
            if (args.size() < 2)
              return args.empty() ? Value(0.0) : args[0];
            return Value(std::max(args[0].toNumber(), args[1].toNumber()));
          });

      registerBuiltin("logarithm",
                      [](const std::vector<Value> &args) -> Value { // log
                        if (args.empty())
                          return Value(0.0);
                        return Value(std::log(args[0].toNumber()));
                      });

      registerBuiltin("as", [](const std::vector<Value> &args) -> Value { // exp
        if (args.empty())
          return Value(0.0);
        return Value(std::exp(args[0].toNumber()));
      });

      registerBuiltin("atan2", [](const std::vector<Value> &args) -> Value {
        if (args.size() < 2)
          return Value(0.0);
        return Value(std::atan2(args[0].toNumber(), args[1].toNumber()));
      });

      // Vector Algebra (Mojiha)
      registerBuiltin("mowajeha3",
                      [](const std::vector<Value> &args) -> Value { // vec3
                        auto arr = std::make_shared<std::vector<Value>>();
                        for (int i = 0; i < 3; ++i) {
                          arr->push_back(args.size() > (size_t)i ? args[i]
                                                                 : Value(0.0));
                        }
                        return Value(arr);
                      });

      registerBuiltin(
          "mo_dorbat",
          [](const std::vector<Value> &args) -> Value { // dot
            if (args.size() < 2 || args[0].type != ValueType::ARRAY ||
                args[1].type != ValueType::ARRAY)
              return Value(0.0);
            auto a = std::get<ArrayPtr>(args[0].data);
            auto b = std::get<ArrayPtr>(args[1].data);
            double res = 0;
            for (size_t i = 0; i < std::min(a->size(), b->size()); ++i) {
              res += (*a)[i].toNumber() * (*b)[i].toNumber();
            }
            return Value(res);
          });

      registerBuiltin(
          "mo_ti9ati",
          [](const std::vector<Value> &args) -> Value { // cross
            if (args.size() < 2 || args[0].type != ValueType::ARRAY ||
                args[1].type != ValueType::ARRAY)
              return Value();
            auto a = std::get<ArrayPtr>(args[0].data);
            auto b = std::get<ArrayPtr>(args[1].data);
            if (a->size() < 3 || b->size() < 3)
              return Value();
            auto res = std::make_shared<std::vector<Value>>();
            res->push_back(Value((*a)[1].toNumber() * (*b)[2].toNumber() -
                                 (*a)[2].toNumber() * (*b)[1].toNumber()));
            res->push_back(Value((*a)[2].toNumber() * (*b)[0].toNumber() -
                                 (*a)[0].toNumber() * (*b)[2].toNumber()));
            res->push_back(Value((*a)[0].toNumber() * (*b)[1].toNumber() -
                                 (*a)[1].toNumber() * (*b)[0].toNumber()));
            return Value(res);
          });

      registerBuiltin("mo_toul",
                      [](const std::vector<Value> &args) -> Value { // magnitude
                        if (args.empty() || args[0].type != ValueType::ARRAY)
                          return Value(0.0);
                        auto a = std::get<ArrayPtr>(args[0].data);
                        double res = 0;
                        for (const auto &v : *a)
                          res += v.toNumber() * v.toNumber();
                        return Value(std::sqrt(res));
                      });

      registerBuiltin("mo_nidam",
                      [](const std::vector<Value> &args) -> Value { // normalize
                        if (args.empty() || args[0].type != ValueType::ARRAY)
                          return Value();
                        auto a = std::get<ArrayPtr>(args[0].data);
                        double len = 0;
                        for (const auto &v : *a)
                          len += v.toNumber() * v.toNumber();
                        len = std::sqrt(len);
                        if (len == 0)
                          return args[0];
                        auto res = std::make_shared<std::vector<Value>>();
                        for (const auto &v : *a)
                          res->push_back(Value(v.toNumber() / len));
                        return Value(res);
                      });

      // Matrix Operations (Masfofa)
      registerBuiltin("masfofa4",
                      [](const std::vector<Value> &args) -> Value { // mat4
                        auto arr = std::make_shared<std::vector<Value>>();
                        for (int i = 0; i < 16; ++i) {
                          arr->push_back(args.size() > (size_t)i ? args[i]
                                                                 : Value(0.0));
                        }
                        return Value(arr);
                      });

      registerBuiltin("mf_mawjud",
                      [](const std::vector<Value> &args) -> Value { // identity
                        auto arr = std::make_shared<std::vector<Value>>();
                        for (int i = 0; i < 16; ++i) {
                          arr->push_back(Value((i % 5 == 0) ? 1.0 : 0.0));
                        }
                        return Value(arr);
                      });

      registerBuiltin(
          "mf_dorbat",
          [](const std::vector<Value> &args) -> Value { // mat_multiply
            if (args.size() < 2 || args[0].type != ValueType::ARRAY ||
                args[1].type != ValueType::ARRAY)
              return Value();
            auto a = std::get<ArrayPtr>(args[0].data);
            auto b = std::get<ArrayPtr>(args[1].data);
            if (a->size() < 16 || b->size() < 16)
              return Value();
            auto res = std::make_shared<std::vector<Value>>();
            for (int i = 0; i < 4; ++i) {
              for (int j = 0; j < 4; ++j) {
                double sum = 0;
                for (int k = 0; k < 4; ++k) {
                  sum +=
                      (*a)[i * 4 + k].toNumber() * (*b)[k * 4 + j].toNumber();
                }
                res->push_back(Value(sum));
              }
            }
            return Value(res);
          });

      registerBuiltin(
          "mf_translate",
          [](const std::vector<Value> &args) -> Value { // translate
            if (args.size() < 2 || args[0].type != ValueType::ARRAY ||
                args[1].type != ValueType::ARRAY)
              return Value();
            auto m = std::get<ArrayPtr>(args[0].data);
            auto v = std::get<ArrayPtr>(args[1].data);
            if (m->size() < 16 || v->size() < 3)
              return Value();
            auto res = std::make_shared<std::vector<Value>>(*m);
            for (int i = 0; i < 4; ++i) {
              (*res)[12 + i] =
                  Value((*m)[12 + i].toNumber() +
                        (*m)[i].toNumber() * (*v)[0].toNumber() +
                        (*m)[4 + i].toNumber() * (*v)[1].toNumber() +
                        (*m)[8 + i].toNumber() * (*v)[2].toNumber());
            }
            return Value(res);
          });

      registerBuiltin("mf_scale",
                      [](const std::vector<Value> &args) -> Value { // scale
                        if (args.size() < 2 || args[0].type != ValueType::ARRAY)
                          return Value();
                        auto m = std::get<ArrayPtr>(args[0].data);
                        if (m->size() < 16)
                          return Value();
                        double sx, sy, sz;
                        if (args[1].type == ValueType::ARRAY) {
                          auto v = std::get<ArrayPtr>(args[1].data);
                          sx = (*v)[0].toNumber();
                          sy = v->size() > 1 ? (*v)[1].toNumber() : sx;
                          sz = v->size() > 2 ? (*v)[2].toNumber() : 1.0;
                        } else {
                          sx = sy = sz = args[1].toNumber();
                        }
                        auto res = std::make_shared<std::vector<Value>>(*m);
                        for (int i = 0; i < 4; ++i)
                          (*res)[i] = Value((*m)[i].toNumber() * sx);
                        for (int i = 0; i < 4; ++i)
                          (*res)[4 + i] = Value((*m)[4 + i].toNumber() * sy);
                        for (int i = 0; i < 4; ++i)
                          (*res)[8 + i] = Value((*m)[8 + i].toNumber() * sz);
                        return Value(res);
                      });

      registerBuiltin(
          "mf_rotate",
          [](const std::vector<Value> &args) -> Value { // rotate
            if (args.size() < 3 || args[0].type != ValueType::ARRAY ||
                args[2].type != ValueType::ARRAY)
              return Value();
            auto m = std::get<ArrayPtr>(args[0].data);
            double angle = args[1].toNumber();
            auto axis = std::get<ArrayPtr>(args[2].data);
            if (m->size() < 16 || axis->size() < 3)
              return Value();

            double x = (*axis)[0].toNumber();
            double y = (*axis)[1].toNumber();
            double z = (*axis)[2].toNumber();
            double s = std::sin(angle);
            double c = std::cos(angle);
            double oc = 1.0 - c;

            auto r = std::make_shared<std::vector<Value>>(16, Value(0.0));
            (*r)[0] = Value(x * x * oc + c);
            (*r)[1] = Value(x * y * oc - z * s);
            (*r)[2] = Value(x * z * oc + y * s);
            (*r)[4] = Value(y * x * oc + z * s);
            (*r)[5] = Value(y * y * oc + c);
            (*r)[6] = Value(y * z * oc - x * s);
            (*r)[8] = Value(z * x * oc - y * s);
            (*r)[9] = Value(z * y * oc + x * s);
            (*r)[10] = Value(z * z * oc + c);
            (*r)[15] = Value(1.0);

            auto res = std::make_shared<std::vector<Value>>();
            for (int i = 0; i < 4; ++i) {
              for (int j = 0; j < 4; ++j) {
                double sum = 0;
                for (int k = 0; k < 4; ++k) {
                  sum +=
                      (*m)[i * 4 + k].toNumber() * (*r)[k * 4 + j].toNumber();
                }
                res->push_back(Value(sum));
              }
            }
            return Value(res);
          });

      registerBuiltin(
          "masafa",
          [](const std::vector<Value> &args) -> Value { // distance
            if (args.size() < 2 || args[0].type != ValueType::ARRAY ||
                args[1].type != ValueType::ARRAY)
              return Value(0.0);
            auto a = std::get<ArrayPtr>(args[0].data);
            auto b = std::get<ArrayPtr>(args[1].data);
            double sum = 0;
            for (size_t i = 0; i < std::min(a->size(), b->size()); ++i) {
              double d = (*a)[i].toNumber() - (*b)[i].toNumber();
              sum += d * d;
            }
            return Value(std::sqrt(sum));
          });

      registerBuiltin(
          "zawiya",
          [](const std::vector<Value> &args) -> Value { // angle
            if (args.size() < 2 || args[0].type != ValueType::ARRAY ||
                args[1].type != ValueType::ARRAY)
              return Value(0.0);
            auto a = std::get<ArrayPtr>(args[0].data);
            auto b = std::get<ArrayPtr>(args[1].data);
            double dot = 0, la = 0, lb = 0;
            for (size_t i = 0; i < std::min(a->size(), b->size()); ++i) {
              double av = (*a)[i].toNumber();
              double bv = (*b)[i].toNumber();
              dot += av * bv;
              la += av * av;
              lb += bv * bv;
            }
            double mag = std::sqrt(la) * std::sqrt(lb);
            if (mag == 0)
              return Value(0.0);
            double cosTheta = dot / mag;
            if (cosTheta > 1.0)
              cosTheta = 1.0;
            if (cosTheta < -1.0)
              cosTheta = -1.0;
            return Value(std::acos(cosTheta));
          });
    }

    void Interpreter::loadModule(const std::string &moduleName) {
      if (importedModules.find(moduleName) != importedModules.end())
        return;

      std::string filename = moduleName + ".lfi3a";
      std::string fullPath = filename;

      if (!currentDirectory.empty()) {
        fs::path dirPath = fs::path(currentDirectory) / filename;
        if (fs::exists(dirPath))
          fullPath = dirPath.string();
      }

      std::ifstream file(fullPath);
      if (!file.is_open()) {
        ErrorHandler::fatal(0, 0, "Cannot open module file '" + filename + "'");
      }

      std::string code((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
      file.close();

      importedModules.insert(moduleName);
      Lexer lexer(code);
      auto tokens = lexer.tokenize();
      Parser parser(tokens);
      auto ast = parser.parse();

      for (const auto &node : ast) {
        if (hasReturned)
          hasReturned = false;
        execute(node);
      }
    }

    void Interpreter::loadModuleItem(const std::string &moduleName,
                                     const std::string &itemName) {
      loadModule(moduleName);
    }