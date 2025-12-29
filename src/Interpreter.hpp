#ifndef LFI3A_INTERPRETER_HPP
#define LFI3A_INTERPRETER_HPP

#include "AST.hpp"
#include "Environment.hpp"
#include "Value.hpp"
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Interpreter {
public:
  Interpreter();
  void run(const std::vector<ASTNodePtr> &nodes,
           const std::string &currentDir = "");

private:
  std::shared_ptr<Environment> globals;
  std::shared_ptr<Environment> environment;

  std::unordered_map<std::string, ASTNodePtr> functions;
  std::set<std::string> importedModules;

  Value returnValue;
  bool hasReturned = false;
  std::string currentDirectory;

  void registerBuiltin(const std::string &name, NativeFunc func);
  void registerBuiltins();

  Value evaluate(const ASTNodePtr &node);
  void execute(const ASTNodePtr &node);
  void loadModule(const std::string &moduleName);
  void loadModuleItem(const std::string &moduleName,
                      const std::string &itemName);

  void executeBlock(const ASTNodePtr &block, std::shared_ptr<Environment> env);
};

#endif
