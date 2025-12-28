#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

Interpreter::Interpreter() {
  globals = std::make_shared<Environment>();
  environment = globals;
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
    Value value = evaluate(node->children[0]);
    environment->assign(node->value, value);
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
    return environment->get(node->value);
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
        std::cerr << "Error: Division by zero\n";
        exit(1);
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
    } else {
      std::cerr << "Error: Undefined function '" << node->value << "'\n";
      exit(1);
    }
  }

  default:
    break;
  }

  return Value();
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
    std::cerr << "Error: Cannot open module file '" << filename << "'\n";
    exit(1);
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