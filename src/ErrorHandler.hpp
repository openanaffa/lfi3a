#ifndef LFI3A_ERROR_HANDLER_HPP
#define LFI3A_ERROR_HANDLER_HPP

#include <iostream>
#include <string>


class ErrorHandler {
public:
  static void report(int line, int col, const std::string &message,
                     const std::string &type = "Error") {
    std::cerr << "[" << type << "] line " << line << ", col " << col << ": "
              << message << std::endl;
  }

  static void fatal(int line, int col, const std::string &message) {
    report(line, col, message, "Fatal Error");
    exit(1);
  }
};

#endif
