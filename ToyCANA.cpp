#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <stack>

using namespace std;

// Token types enumeration
enum TokenType {
    // Keywords
    TOK_INT, TOK_VOID, TOK_IF, TOK_ELSE, TOK_WHILE, 
    TOK_BREAK, TOK_CONTINUE, TOK_RETURN,
    
    // Identifiers and literals
    TOK_ID, TOK_NUMBER,
    
    // Operators
    TOK_PLUS, TOK_MINUS, TOK_MULTIPLY, TOK_DIVIDE, TOK_MODULO,
    TOK_ASSIGN, TOK_LT, TOK_GT, TOK_LE, TOK_GE, TOK_EQ, TOK_NE,
    TOK_AND, TOK_OR, TOK_NOT,
    
    // Delimiters
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_SEMICOLON, TOK_COMMA,
    
    // Special
    TOK_EOF, TOK_ERROR
};

// Token structure
struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    
    Token(TokenType t = TOK_EOF, const string& v = "", int l = 1, int c = 1) 
        : type(t), value(v), line(l), column(c) {}
};

// Lexer class
class Lexer {
private:
    string source;
    size_t pos;
    int line;
    int column;
    map<string, TokenType> keywords;
    vector<string> errors;
    
    void initKeywords() {
        keywords["int"] = TOK_INT;
        keywords["void"] = TOK_VOID;
        keywords["if"] = TOK_IF;
        keywords["else"] = TOK_ELSE;
        keywords["while"] = TOK_WHILE;
        keywords["break"] = TOK_BREAK;
        keywords["continue"] = TOK_CONTINUE;
        keywords["return"] = TOK_RETURN;
    }
    
    char currentChar() {
        if (pos >= source.length()) return '\0';
        return source[pos];
    }
    
    char peekChar(int offset = 1) {
        size_t newPos = pos + offset;
        if (newPos >= source.length()) return '\0';
        return source[newPos];
    }
    
    void advance() {
        if (pos < source.length()) {
            if (source[pos] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
        }
    }
    
    void skipWhitespace() {
        while (isspace(currentChar())) {
            advance();
        }
    }
    
    void skipSingleLineComment() {
        advance(); // skip first /
        advance(); // skip second /
        while (currentChar() != '\n' && currentChar() != '\0') {
            advance();
        }
    }
    
    void skipMultiLineComment() {
        int startLine = line;
        advance(); // skip /
        advance(); // skip *
        
        while (currentChar() != '\0') {
            if (currentChar() == '*' && peekChar() == '/') {
                advance(); // skip *
                advance(); // skip /
                return;
            }
            advance();
        }
        
        // Unterminated comment
        errors.push_back(to_string(startLine) + " Unterminated comment");
    }
    
    Token readNumber() {
        int startColumn = column;
        string num;
        
        while (isdigit(currentChar())) {
            num += currentChar();
            advance();
        }
        
        return Token(TOK_NUMBER, num, line, startColumn);
    }
    
    Token readIdentifier() {
        int startColumn = column;
        string id;
        
        while (isalnum(currentChar()) || currentChar() == '_') {
            id += currentChar();
            advance();
        }
        
        // Check if it's a keyword
        auto it = keywords.find(id);
        if (it != keywords.end()) {
            return Token(it->second, id, line, startColumn);
        }
        
        return Token(TOK_ID, id, line, startColumn);
    }
    
public:
    Lexer(const string& src) : source(src), pos(0), line(1), column(1) {
        initKeywords();
    }
    
    vector<Token> tokenize() {
        vector<Token> tokens;
        
        while (currentChar() != '\0') {
            skipWhitespace();
            
            if (currentChar() == '\0') break;
            
            int startColumn = column;
            int startLine = line;
            char ch = currentChar();
            
            // Comments
            if (ch == '/' && peekChar() == '/') {
                skipSingleLineComment();
                continue;
            }
            
            if (ch == '/' && peekChar() == '*') {
                skipMultiLineComment();
                continue;
            }
            
            // Numbers
            if (isdigit(ch)) {
                tokens.push_back(readNumber());
                continue;
            }
            
            // Identifiers and keywords
            if (isalpha(ch) || ch == '_') {
                tokens.push_back(readIdentifier());
                continue;
            }
            
            // Single character tokens
            switch (ch) {
                case '+':
                    tokens.push_back(Token(TOK_PLUS, "+", startLine, startColumn));
                    advance();
                    break;
                case '-':
                    tokens.push_back(Token(TOK_MINUS, "-", startLine, startColumn));
                    advance();
                    break;
                case '*':
                    tokens.push_back(Token(TOK_MULTIPLY, "*", startLine, startColumn));
                    advance();
                    break;
                case '/':
                    tokens.push_back(Token(TOK_DIVIDE, "/", startLine, startColumn));
                    advance();
                    break;
                case '%':
                    tokens.push_back(Token(TOK_MODULO, "%", startLine, startColumn));
                    advance();
                    break;
                case '(':
                    tokens.push_back(Token(TOK_LPAREN, "(", startLine, startColumn));
                    advance();
                    break;
                case ')':
                    tokens.push_back(Token(TOK_RPAREN, ")", startLine, startColumn));
                    advance();
                    break;
                case '{':
                    tokens.push_back(Token(TOK_LBRACE, "{", startLine, startColumn));
                    advance();
                    break;
                case '}':
                    tokens.push_back(Token(TOK_RBRACE, "}", startLine, startColumn));
                    advance();
                    break;
                case ';':
                    tokens.push_back(Token(TOK_SEMICOLON, ";", startLine, startColumn));
                    advance();
                    break;
                case ',':
                    tokens.push_back(Token(TOK_COMMA, ",", startLine, startColumn));
                    advance();
                    break;
                case '=':
                    advance();
                    if (currentChar() == '=') {
                        tokens.push_back(Token(TOK_EQ, "==", startLine, startColumn));
                        advance();
                    } else {
                        tokens.push_back(Token(TOK_ASSIGN, "=", startLine, startColumn));
                    }
                    break;
                case '<':
                    advance();
                    if (currentChar() == '=') {
                        tokens.push_back(Token(TOK_LE, "<=", startLine, startColumn));
                        advance();
                    } else {
                        tokens.push_back(Token(TOK_LT, "<", startLine, startColumn));
                    }
                    break;
                case '>':
                    advance();
                    if (currentChar() == '=') {
                        tokens.push_back(Token(TOK_GE, ">=", startLine, startColumn));
                        advance();
                    } else {
                        tokens.push_back(Token(TOK_GT, ">", startLine, startColumn));
                    }
                    break;
                case '!':
                    advance();
                    if (currentChar() == '=') {
                        tokens.push_back(Token(TOK_NE, "!=", startLine, startColumn));
                        advance();
                    } else {
                        tokens.push_back(Token(TOK_NOT, "!", startLine, startColumn));
                    }
                    break;
                case '&':
                    advance();
                    if (currentChar() == '&') {
                        tokens.push_back(Token(TOK_AND, "&&", startLine, startColumn));
                        advance();
                    } else {
                        errors.push_back(to_string(startLine) + " Lack of '&'");
                    }
                    break;
                case '|':
                    advance();
                    if (currentChar() == '|') {
                        tokens.push_back(Token(TOK_OR, "||", startLine, startColumn));
                        advance();
                    } else {
                        errors.push_back(to_string(startLine) + " Lack of '|'");
                    }
                    break;
                default:
                    errors.push_back(to_string(startLine) + " Unexpected character: " + string(1, ch));
                    advance();
            }
        }
        
        tokens.push_back(Token(TOK_EOF, "", line, column));
        return tokens;
    }
    
    bool hasErrors() const { return !errors.empty(); }
    vector<string> getErrors() const { return errors; }
};

// Forward declarations for AST nodes
class ASTNode;
class Expression;
class Statement;
class Block;

// AST Node base class
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Expression nodes
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class NumberExpr : public Expression {
public:
    int value;
    NumberExpr(int v) : value(v) {}
};

class IdentifierExpr : public Expression {
public:
    string name;
    IdentifierExpr(const string& n) : name(n) {}
};

class BinaryOpExpr : public Expression {
public:
    string op;
    shared_ptr<Expression> left;
    shared_ptr<Expression> right;
    BinaryOpExpr(const string& o, shared_ptr<Expression> l, shared_ptr<Expression> r) 
        : op(o), left(l), right(r) {}
};

class UnaryOpExpr : public Expression {
public:
    string op;
    shared_ptr<Expression> operand;
    UnaryOpExpr(const string& o, shared_ptr<Expression> expr) 
        : op(o), operand(expr) {}
};

class AssignmentExpr : public Expression {
public:
    string target;
    shared_ptr<Expression> value;
    AssignmentExpr(const string& t, shared_ptr<Expression> v) 
        : target(t), value(v) {}
};

class FunctionCallExpr : public Expression {
public:
    string name;
    vector<shared_ptr<Expression>> args;
    FunctionCallExpr(const string& n) : name(n) {}
};

// Statement nodes
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

class VarDeclStatement : public Statement {
public:
    string type;
    string name;
    shared_ptr<Expression> init;
    VarDeclStatement(const string& t, const string& n, shared_ptr<Expression> i = nullptr)
        : type(t), name(n), init(i) {}
};

class ExprStatement : public Statement {
public:
    shared_ptr<Expression> expr;
    ExprStatement(shared_ptr<Expression> e = nullptr) : expr(e) {}
};

class Block : public Statement {
public:
    vector<shared_ptr<Statement>> statements;
};

class IfStatement : public Statement {
public:
    shared_ptr<Expression> condition;
    shared_ptr<Statement> thenStmt;
    shared_ptr<Statement> elseStmt;
    IfStatement(shared_ptr<Expression> c, shared_ptr<Statement> t, shared_ptr<Statement> e = nullptr)
        : condition(c), thenStmt(t), elseStmt(e) {}
};

class WhileStatement : public Statement {
public:
    shared_ptr<Expression> condition;
    shared_ptr<Statement> body;
    WhileStatement(shared_ptr<Expression> c, shared_ptr<Statement> b) 
        : condition(c), body(b) {}
};

class BreakStatement : public Statement {};
class ContinueStatement : public Statement {};

class ReturnStatement : public Statement {
public:
    shared_ptr<Expression> value;
    ReturnStatement(shared_ptr<Expression> v = nullptr) : value(v) {}
};

// Function definition
class FunctionDef : public ASTNode {
public:
    string returnType;
    string name;
    vector<pair<string, string>> params;  // type, name pairs
    shared_ptr<Block> body;
    
    FunctionDef(const string& ret, const string& n) 
        : returnType(ret), name(n) {}
};

// Program (root of AST)
class Program : public ASTNode {
public:
    vector<shared_ptr<FunctionDef>> functions;
};

// Parser class
class Parser {
private:
    vector<Token> tokens;
    size_t pos;
    vector<string> errors;
    
    Token currentToken() {
        if (pos < tokens.size()) return tokens[pos];
        return Token(TOK_EOF);
    }
    
    Token peekToken(int offset = 1) {
        size_t newPos = pos + offset;
        if (newPos < tokens.size()) return tokens[newPos];
        return Token(TOK_EOF);
    }
    
    void advance() {
        if (pos < tokens.size() - 1) pos++;
    }
    
    bool expect(TokenType type) {
        if (currentToken().type == type) {
            advance();
            return true;
        }
        return false;
    }
    
    bool match(TokenType type) {
        return currentToken().type == type;
    }
    
    shared_ptr<Expression> parseExpression();
    shared_ptr<Expression> parseAssignment();
    shared_ptr<Expression> parseLogicalOr();
    shared_ptr<Expression> parseLogicalAnd();
    shared_ptr<Expression> parseRelational();
    shared_ptr<Expression> parseAdditive();
    shared_ptr<Expression> parseMultiplicative();
    shared_ptr<Expression> parseUnary();
    shared_ptr<Expression> parsePrimary();
    
    shared_ptr<Statement> parseStatement();
    shared_ptr<Block> parseBlock();
    shared_ptr<Statement> parseIfStatement();
    shared_ptr<Statement> parseWhileStatement();
    shared_ptr<Statement> parseReturnStatement();
    shared_ptr<Statement> parseVarDecl();
    
    shared_ptr<FunctionDef> parseFunction();
    
public:
    Parser(const vector<Token>& toks) : tokens(toks), pos(0) {}
    
    shared_ptr<Program> parse() {
        auto program = make_shared<Program>();
        
        while (!match(TOK_EOF)) {
            auto func = parseFunction();
            if (func) {
                program->functions.push_back(func);
            } else {
                // Skip to next function or EOF
                while (!match(TOK_INT) && !match(TOK_VOID) && !match(TOK_EOF)) {
                    advance();
                }
            }
        }
        
        return program;
    }
    
    bool hasErrors() const { return !errors.empty(); }
    vector<string> getErrors() const { return errors; }
};

// Parser implementation
shared_ptr<FunctionDef> Parser::parseFunction() {
    // Parse return type
    string returnType;
    if (match(TOK_INT)) {
        returnType = "int";
        advance();
    } else if (match(TOK_VOID)) {
        returnType = "void";
        advance();
    } else {
        return nullptr;
    }
    
    // Parse function name
    if (!match(TOK_ID)) {
        return nullptr;
    }
    string funcName = currentToken().value;
    advance();
    
    auto func = make_shared<FunctionDef>(returnType, funcName);
    
    // Parse parameter list
    if (!expect(TOK_LPAREN)) return nullptr;
    
    if (!match(TOK_RPAREN)) {
        while (true) {
            // Parse parameter type
            if (!match(TOK_INT)) {
                break;
            }
            advance();
            
            // Parse parameter name
            if (!match(TOK_ID)) {
                break;
            }
            string paramName = currentToken().value;
            func->params.push_back(make_pair("int", paramName));
            advance();
            
            if (match(TOK_COMMA)) {
                advance();
            } else {
                break;
            }
        }
    }
    
    if (!expect(TOK_RPAREN)) return nullptr;
    
    // Parse function body
    func->body = parseBlock();
    if (!func->body) return nullptr;
    
    return func;
}

shared_ptr<Block> Parser::parseBlock() {
    if (!expect(TOK_LBRACE)) return nullptr;
    
    auto block = make_shared<Block>();
    
    while (!match(TOK_RBRACE) && !match(TOK_EOF)) {
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(stmt);
        }
    }
    
    if (!expect(TOK_RBRACE)) return nullptr;
    
    return block;
}

shared_ptr<Statement> Parser::parseStatement() {
    // Check for variable declaration
    if (match(TOK_INT)) {
        return parseVarDecl();
    }
    
    if (match(TOK_IF)) {
        return parseIfStatement();
    } else if (match(TOK_WHILE)) {
        return parseWhileStatement();
    } else if (match(TOK_BREAK)) {
        advance();
        expect(TOK_SEMICOLON);
        return make_shared<BreakStatement>();
    } else if (match(TOK_CONTINUE)) {
        advance();
        expect(TOK_SEMICOLON);
        return make_shared<ContinueStatement>();
    } else if (match(TOK_RETURN)) {
        return parseReturnStatement();
    } else if (match(TOK_LBRACE)) {
        return parseBlock();
    } else {
        // Expression statement
        auto expr = parseExpression();
        expect(TOK_SEMICOLON);
        return make_shared<ExprStatement>(expr);
    }
}

shared_ptr<Statement> Parser::parseVarDecl() {
    expect(TOK_INT);
    
    if (!match(TOK_ID)) {
        return nullptr;
    }
    string varName = currentToken().value;
    advance();
    
    shared_ptr<Expression> init = nullptr;
    if (match(TOK_ASSIGN)) {
        advance();
        init = parseExpression();
    }
    
    expect(TOK_SEMICOLON);
    return make_shared<VarDeclStatement>("int", varName, init);
}

shared_ptr<Statement> Parser::parseIfStatement() {
    expect(TOK_IF);
    expect(TOK_LPAREN);
    auto condition = parseExpression();
    expect(TOK_RPAREN);
    
    auto thenStmt = parseStatement();
    shared_ptr<Statement> elseStmt = nullptr;
    
    if (match(TOK_ELSE)) {
        advance();
        elseStmt = parseStatement();
    }
    
    return make_shared<IfStatement>(condition, thenStmt, elseStmt);
}

shared_ptr<Statement> Parser::parseWhileStatement() {
    expect(TOK_WHILE);
    expect(TOK_LPAREN);
    auto condition = parseExpression();
    expect(TOK_RPAREN);
    auto body = parseStatement();
    
    return make_shared<WhileStatement>(condition, body);
}

shared_ptr<Statement> Parser::parseReturnStatement() {
    expect(TOK_RETURN);
    
    shared_ptr<Expression> value = nullptr;
    if (!match(TOK_SEMICOLON)) {
        value = parseExpression();
    }
    
    expect(TOK_SEMICOLON);
    return make_shared<ReturnStatement>(value);
}

shared_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
}

shared_ptr<Expression> Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (match(TOK_ASSIGN)) {
        // Handle assignment
        if (auto id = dynamic_pointer_cast<IdentifierExpr>(expr)) {
            advance();
            auto value = parseAssignment();  // Right associative
            return make_shared<AssignmentExpr>(id->name, value);
        }
    }
    
    return expr;
}

shared_ptr<Expression> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    
    while (match(TOK_OR)) {
        string op = currentToken().value;
        advance();
        auto right = parseLogicalAnd();
        left = make_shared<BinaryOpExpr>(op, left, right);
    }
    
    return left;
}

shared_ptr<Expression> Parser::parseLogicalAnd() {
    auto left = parseRelational();
    
    while (match(TOK_AND)) {
        string op = currentToken().value;
        advance();
        auto right = parseRelational();
        left = make_shared<BinaryOpExpr>(op, left, right);
    }
    
    return left;
}

shared_ptr<Expression> Parser::parseRelational() {
    auto left = parseAdditive();
    
    while (match(TOK_LT) || match(TOK_GT) || match(TOK_LE) || 
           match(TOK_GE) || match(TOK_EQ) || match(TOK_NE)) {
        string op = currentToken().value;
        advance();
        auto right = parseAdditive();
        left = make_shared<BinaryOpExpr>(op, left, right);
    }
    
    return left;
}

shared_ptr<Expression> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    
    while (match(TOK_PLUS) || match(TOK_MINUS)) {
        string op = currentToken().value;
        advance();
        auto right = parseMultiplicative();
        left = make_shared<BinaryOpExpr>(op, left, right);
    }
    
    return left;
}

shared_ptr<Expression> Parser::parseMultiplicative() {
    auto left = parseUnary();
    
    while (match(TOK_MULTIPLY) || match(TOK_DIVIDE) || match(TOK_MODULO)) {
        string op = currentToken().value;
        advance();
        auto right = parseUnary();
        left = make_shared<BinaryOpExpr>(op, left, right);
    }
    
    return left;
}

shared_ptr<Expression> Parser::parseUnary() {
    if (match(TOK_PLUS) || match(TOK_MINUS) || match(TOK_NOT)) {
        string op = currentToken().value;
        advance();
        auto operand = parseUnary();
        return make_shared<UnaryOpExpr>(op, operand);
    }
    
    return parsePrimary();
}

shared_ptr<Expression> Parser::parsePrimary() {
    // Number literal
    if (match(TOK_NUMBER)) {
        int value = stoi(currentToken().value);
        advance();
        return make_shared<NumberExpr>(value);
    }
    
    // Identifier or function call
    if (match(TOK_ID)) {
        string name = currentToken().value;
        advance();
        
        // Function call
        if (match(TOK_LPAREN)) {
            advance();
            auto funcCall = make_shared<FunctionCallExpr>(name);
            
            if (!match(TOK_RPAREN)) {
                while (true) {
                    funcCall->args.push_back(parseExpression());
                    if (match(TOK_COMMA)) {
                        advance();
                    } else {
                        break;
                    }
                }
            }
            
            expect(TOK_RPAREN);
            return funcCall;
        }
        
        // Simple identifier
        return make_shared<IdentifierExpr>(name);
    }
    
    // Parenthesized expression
    if (match(TOK_LPAREN)) {
        advance();
        auto expr = parseExpression();
        expect(TOK_RPAREN);
        return expr;
    }
    
    return nullptr;
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <source_file>" << endl;
        return 1;
    }
    
    // Read source file
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cout << "reject" << endl;
        return 1;
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    string source = buffer.str();
    file.close();
    
    // Lexical analysis
    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();
    
    if (lexer.hasErrors()) {
        cout << "reject" << endl;
        for (const auto& error : lexer.getErrors()) {
            cout << error << endl;
        }
        return 1;
    }
    
    // Syntax analysis
    Parser parser(tokens);
    shared_ptr<Program> program = parser.parse();
    
    if (parser.hasErrors()) {
        cout << "reject" << endl;
        for (const auto& error : parser.getErrors()) {
            cout << error << endl;
        }
        return 1;
    }
    
    // If parsing succeeded, output accept
    cout << "accept" << endl;
    
    return 0;
}