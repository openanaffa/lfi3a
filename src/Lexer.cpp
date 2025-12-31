#include "Lexer.hpp"
#include <cctype>
#include <unordered_map>

Lexer::Lexer(const std::string &src) : src(src), pos(0), line(1), column(1) {}

char Lexer::peek(size_t offset) {
  return pos + offset < src.size() ? src[pos + offset] : '\0';
}

char Lexer::advance() {
  char c = peek();
  pos++;
  if (c == '\n') {
    line++;
    column = 1;
  } else {
    column++;
  }
  return c;
}

void Lexer::skipWhitespace() {
  while (isspace(peek())) {
    advance();
  }
}

void Lexer::skipComment() {
  if (peek() == '/' && peek(1) == '/') {
    advance(); // skip /
    advance(); // skip /
    while (peek() != '\n' && peek() != '\0') {
      advance();
    }
  }
}

Token Lexer::string() {
  int startLine = line;
  int startCol = column;
  advance(); // skip opening "
  std::string result;
  while (peek() != '"' && peek() != '\0') {
    if (peek() == '\\') {
      advance();
      char escaped = advance();
      switch (escaped) {
      case 'n':
        result += '\n';
        break;
      case 't':
        result += '\t';
        break;
      case '\\':
        result += '\\';
        break;
      case '"':
        result += '"';
        break;
      default:
        result += escaped;
      }
    } else {
      result += advance();
    }
  }
  if (peek() == '"')
    advance(); // skip closing "
  return {STRING, result, startLine, startCol};
}

Token Lexer::identifier() {
  int startLine = line;
  int startCol = column;
  std::string result;
  while (isalnum(peek()) || peek() == '_') {
    result += advance();
  }

  // Check for keywords
  static const std::unordered_map<std::string, TokenType> keywords = {
      {"dir", DIR},     {"kteb", KTEB},   {"ila", ILA},   {"wila", WILA},
      {"wla", WLA},     {"ma7ad", MA7AD}, {"kol", KOL},   {"dalla", DALLA},
      {"kalla", KALLA}, {"rje3", RJE3},   {"s7i7", S7I7}, {"ghalat", GHALAT},
      {"w", W},         {"jib", JIB},     {"man", MAN}};

  auto it = keywords.find(result);
  if (it != keywords.end()) {
    return {it->second, result, startLine, startCol};
  }

  return {IDENT, result, startLine, startCol};
}

Token Lexer::number() {
  int startLine = line;
  int startCol = column;
  std::string result;
  while (isdigit(peek())) {
    result += advance();
  }

  // Handle decimals
  if (peek() == '.' && isdigit(peek(1))) {
    result += advance(); // add .
    while (isdigit(peek())) {
      result += advance();
    }
  }

  return {NUMBER, result, startLine, startCol};
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (peek() != '\0') {
    skipWhitespace();

    if (peek() == '/' && peek(1) == '/') {
      skipComment();
      continue;
    }

    char c = peek();
    if (c == '\0')
      break;

    // Strings
    if (c == '"') {
      tokens.push_back(string());
      continue;
    }

    // Identifiers and keywords
    if (isalpha((unsigned char)c) || c == '_') {
      tokens.push_back(identifier());
      continue;
    }

    // Numbers
    if (isdigit((unsigned char)c)) {
      tokens.push_back(number());
      continue;
    }

    int startLine = line;
    int startCol = column;

    // Single and multi-character operators
    if (c == '(') {
      advance();
      tokens.push_back({LPAREN, "(", startLine, startCol});
    } else if (c == ')') {
      advance();
      tokens.push_back({RPAREN, ")", startLine, startCol});
    } else if (c == '{') {
      advance();
      tokens.push_back({LBRACE, "{", startLine, startCol});
    } else if (c == '}') {
      advance();
      tokens.push_back({RBRACE, "}", startLine, startCol});
    } else if (c == ';') {
      advance();
      tokens.push_back({SEMICOLON, ";", startLine, startCol});
    } else if (c == ',') {
      advance();
      tokens.push_back({COMMA, ",", startLine, startCol});
    } else if (c == '+') {
      advance();
      if (peek() == '+') {
        advance();
        tokens.push_back({PLUS_PLUS, "++", startLine, startCol});
      } else {
        tokens.push_back({PLUS, "+", startLine, startCol});
      }
    } else if (c == '-') {
      advance();
      tokens.push_back({MINUS, "-", startLine, startCol});
    } else if (c == '*') {
      advance();
      tokens.push_back({STAR, "*", startLine, startCol});
    } else if (c == '/') {
      advance();
      tokens.push_back({SLASH, "/", startLine, startCol});
    } else if (c == '=') {
      advance();
      if (peek() == '=') {
        advance();
        tokens.push_back({EQ_EQ, "==", startLine, startCol});
      } else {
        tokens.push_back({EQUAL, "=", startLine, startCol});
      }
    } else if (c == '!') {
      advance();
      if (peek() == '=') {
        advance();
        tokens.push_back({NOT_EQ, "!=", startLine, startCol});
      } else {
        tokens.push_back({UNKNOWN, "!", startLine, startCol});
      }
    } else if (c == '<') {
      advance();
      if (peek() == '=') {
        advance();
        tokens.push_back({LE, "<=", startLine, startCol});
      } else {
        tokens.push_back({LT, "<", startLine, startCol});
      }
    } else if (c == '>') {
      advance();
      if (peek() == '=') {
        advance();
        tokens.push_back({GE, ">=", startLine, startCol});
      } else {
        tokens.push_back({GT, ">", startLine, startCol});
      }
    } else if (c == '[') {
      advance();
      tokens.push_back({LBRACKET, "[", startLine, startCol});
    } else if (c == ']') {
      advance();
      tokens.push_back({RBRACKET, "]", startLine, startCol});
    } else if (c == '%') {
      advance();
      tokens.push_back({PERCENT, "%", startLine, startCol});
    } else {
      int startLine = line;
      int startCol = column;
      advance();
      tokens.push_back({UNKNOWN, std::string(1, c), startLine, startCol});
    }
  }

  tokens.push_back({END, ""});
  return tokens;
}
