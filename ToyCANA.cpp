#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>  // 添加这行以使用 sort()

using namespace std;

// Token types
enum TokenType {
    TOK_EOF,
    TOK_INT, TOK_VOID, TOK_IF, TOK_ELSE, TOK_WHILE,
    TOK_BREAK, TOK_CONTINUE, TOK_RETURN,
    TOK_ID, TOK_NUMBER,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_DIV, TOK_MOD,
    TOK_LT, TOK_LE, TOK_GT, TOK_GE, TOK_EQ, TOK_NE,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_ASSIGN,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_SEMICOLON, TOK_COMMA,
    TOK_ERROR
};

struct Token {
    TokenType type;
    string value;
    int line;
    int col;
};

class Lexer {
private:
    string input;
    size_t pos;
    int line;
    int col;
    vector<pair<int, string>> errors;

    char peek(int offset = 0) {
        if (pos + offset >= input.length()) return '\0';
        return input[pos + offset];
    }

    char advance() {
        if (pos >= input.length()) return '\0';
        char ch = input[pos++];
        if (ch == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        return ch;
    }

    void skipWhitespace() {
        while (isspace(peek())) {
            advance();
        }
    }

    bool skipComment() {
        if (peek() == '/' && peek(1) == '/') {
            while (peek() != '\n' && peek() != '\0') {
                advance();
            }
            return true;
        }
        if (peek() == '/' && peek(1) == '*') {
            int startLine = line;
            advance(); advance();
            while (true) {
                if (peek() == '\0') {
                    errors.push_back({startLine, "Unterminated comment"});
                    return false;
                }
                if (peek() == '*' && peek(1) == '/') {
                    advance(); advance();
                    break;
                }
                advance();
            }
            return true;
        }
        return false;
    }

public:
    Lexer(const string& src) : input(src), pos(0), line(1), col(1) {}

    vector<pair<int, string>> getErrors() { return errors; }

    Token nextToken() {
        while (true) {
            skipWhitespace();
            if (!skipComment()) break;
        }

        Token tok;
        tok.line = line;
        tok.col = col;

        if (peek() == '\0') {
            tok.type = TOK_EOF;
            return tok;
        }

        // Identifiers and keywords
        if (isalpha(peek()) || peek() == '_') {
            string id;
            while (isalnum(peek()) || peek() == '_') {
                id += advance();
            }
            tok.value = id;

            if (id == "int") tok.type = TOK_INT;
            else if (id == "void") tok.type = TOK_VOID;
            else if (id == "if") tok.type = TOK_IF;
            else if (id == "else") tok.type = TOK_ELSE;
            else if (id == "while") tok.type = TOK_WHILE;
            else if (id == "break") tok.type = TOK_BREAK;
            else if (id == "continue") tok.type = TOK_CONTINUE;
            else if (id == "return") tok.type = TOK_RETURN;
            else tok.type = TOK_ID;

            return tok;
        }

        // Numbers
        if (isdigit(peek())) {
            string num;
            while (isdigit(peek())) {
                num += advance();
            }
            tok.type = TOK_NUMBER;
            tok.value = num;
            return tok;
        }

        // Operators and punctuation
        char ch = peek();
        switch (ch) {
            case '+': advance(); tok.type = TOK_PLUS; return tok;
            case '-': advance(); tok.type = TOK_MINUS; return tok;
            case '*': advance(); tok.type = TOK_STAR; return tok;
            case '/': advance(); tok.type = TOK_DIV; return tok;
            case '%': advance(); tok.type = TOK_MOD; return tok;
            case '(': advance(); tok.type = TOK_LPAREN; return tok;
            case ')': advance(); tok.type = TOK_RPAREN; return tok;
            case '{': advance(); tok.type = TOK_LBRACE; return tok;
            case '}': advance(); tok.type = TOK_RBRACE; return tok;
            case ';': advance(); tok.type = TOK_SEMICOLON; return tok;
            case ',': advance(); tok.type = TOK_COMMA; return tok;
            case '<':
                advance();
                if (peek() == '=') {
                    advance();
                    tok.type = TOK_LE;
                } else {
                    tok.type = TOK_LT;
                }
                return tok;
            case '>':
                advance();
                if (peek() == '=') {
                    advance();
                    tok.type = TOK_GE;
                } else {
                    tok.type = TOK_GT;
                }
                return tok;
            case '=':
                advance();
                if (peek() == '=') {
                    advance();
                    tok.type = TOK_EQ;
                } else {
                    tok.type = TOK_ASSIGN;
                }
                return tok;
            case '!':
                advance();
                if (peek() == '=') {
                    advance();
                    tok.type = TOK_NE;
                } else {
                    tok.type = TOK_NOT;
                }
                return tok;
            case '&':
                advance();
                if (peek() == '&') {
                    advance();
                    tok.type = TOK_AND;
                } else {
                    tok.type = TOK_ERROR;
                }
                return tok;
            case '|':
                advance();
                if (peek() == '|') {
                    advance();
                    tok.type = TOK_OR;
                } else {
                    tok.type = TOK_ERROR;
                }
                return tok;
            default:
                advance();
                tok.type = TOK_ERROR;
                return tok;
        }
    }
};

class Parser {
private:
    vector<Token> tokens;
    size_t pos;
    vector<pair<int, string>> errors;
    set<int> errorLines;
    int loopDepth;

    Token peek(int offset = 0) {
        if (pos + offset >= tokens.size()) {
            return tokens.back();
        }
        return tokens[pos + offset];
    }

    Token advance() {
        if (pos < tokens.size()) {
            return tokens[pos++];
        }
        return tokens.back();
    }

    void addError(const string& msg) {
        Token tok = peek();
        if (errorLines.find(tok.line) == errorLines.end()) {
            errors.push_back({tok.line, msg});
            errorLines.insert(tok.line);
        }
    }

    bool match(TokenType type) {
        return peek().type == type;
    }

    bool consume(TokenType type, const string& errMsg) {
        if (match(type)) {
            advance();
            return true;
        }
        addError(errMsg);
        return false;
    }

    // Grammar rules
    void parseCompUnit() {
        while (!match(TOK_EOF)) {
            parseFuncDef();
        }
    }

    void parseFuncDef() {
        // ("int" | "void") ID "(" (Param ("," Param)*)? ")" Block
        if (!match(TOK_INT) && !match(TOK_VOID)) {
            addError("Expected function return type");
            // Skip to next function
            while (!match(TOK_EOF) && !match(TOK_INT) && !match(TOK_VOID)) {
                advance();
            }
            return;
        }
        advance(); // consume type

        if (!consume(TOK_ID, "Expected function name")) {
            while (!match(TOK_EOF) && !match(TOK_INT) && !match(TOK_VOID)) {
                advance();
            }
            return;
        }

        if (!consume(TOK_LPAREN, "Lack of '('")) {
            while (!match(TOK_EOF) && !match(TOK_LBRACE)) {
                if (match(TOK_INT) || match(TOK_VOID)) break;
                advance();
            }
        }

        // Parameters
        if (match(TOK_INT)) {
            parseParam();
            while (match(TOK_COMMA)) {
                advance();
                parseParam();
            }
        }

        if (!consume(TOK_RPAREN, "Lack of ')'")) {
            while (!match(TOK_EOF) && !match(TOK_LBRACE)) {
                if (match(TOK_INT) || match(TOK_VOID)) break;
                advance();
            }
        }

        parseBlock();
    }

    void parseParam() {
        consume(TOK_INT, "Expected parameter type");
        consume(TOK_ID, "Expected parameter name");
    }

    void parseBlock() {
        if (!consume(TOK_LBRACE, "Lack of '{'")) {
            return;
        }

        while (!match(TOK_RBRACE) && !match(TOK_EOF)) {
            parseStmt();
        }

        consume(TOK_RBRACE, "Lack of '}'");
    }

    void parseStmt() {
        if (match(TOK_INT)) {
            // 变量声明
            advance();
            if (!consume(TOK_ID, "Expected variable name")) {
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
                return;
            }
            if (match(TOK_ASSIGN)) {
                advance();
                parseExpr();
            }
            if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
            }
            return;
        } else if (match(TOK_IF)) {
            advance();
            if (!consume(TOK_LPAREN, "Lack of '('")) {
                while (!match(TOK_RPAREN) && !match(TOK_EOF) && !match(TOK_LBRACE) && !match(TOK_SEMICOLON)) advance();
            } else {
                parseExpr();
                consume(TOK_RPAREN, "Lack of ')'");
            }
            parseStmt();
            if (match(TOK_ELSE)) {
                advance();
                parseStmt();
            }
            return;
        } else if (match(TOK_WHILE)) {
            advance();
            if (!consume(TOK_LPAREN, "Lack of '('")) {
                while (!match(TOK_RPAREN) && !match(TOK_EOF) && !match(TOK_LBRACE) && !match(TOK_SEMICOLON)) advance();
            } else {
                parseExpr();
                consume(TOK_RPAREN, "Lack of ')'");
            }
            loopDepth++;
            parseStmt();
            loopDepth--;
            return;
        } else if (match(TOK_BREAK)) {
            advance();
            if (loopDepth == 0) addError("break not in loop");
            if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
            }
            return;
        } else if (match(TOK_CONTINUE)) {
            advance();
            if (loopDepth == 0) addError("continue not in loop");
            if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
            }
            return;
        } else if (match(TOK_RETURN)) {
            advance();
            if (!match(TOK_SEMICOLON)) {
                parseExpr();
            }
            if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
            }
            return;
        } else if (match(TOK_LBRACE)) {
            parseBlock();
            return;
        } else if (match(TOK_ID)) {
            // 赋值或函数调用或表达式语句
            advance();
            if (match(TOK_ASSIGN)) {
                advance();
                parseExpr();
                if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                    while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                    if (match(TOK_SEMICOLON)) advance();
                }
                return;
            } else if (match(TOK_LPAREN)) {
                advance();
                if (!match(TOK_RPAREN)) {
                    parseExpr();
                    while (match(TOK_COMMA)) {
                        advance();
                        parseExpr();
                    }
                }
                consume(TOK_RPAREN, "Lack of ')'");
                if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                    while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                    if (match(TOK_SEMICOLON)) advance();
                }
                return;
            } else {
                addError("Invalid statement");
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
                return;
            }
        } else {
            // 空语句或表达式语句
            if (match(TOK_SEMICOLON)) {
                advance();
                return;
            }
            parseExpr();
            if (!consume(TOK_SEMICOLON, "Lack of ';'")) {
                while (!match(TOK_SEMICOLON) && !match(TOK_EOF) && !match(TOK_RBRACE)) advance();
                if (match(TOK_SEMICOLON)) advance();
            }
            return;
        }
    }

    void parseExpr() {
        parseLOrExpr();
    }

    void parseLOrExpr() {
        parseLAndExpr();
        while (match(TOK_OR)) {
            advance();
            parseLAndExpr();
        }
    }

    void parseLAndExpr() {
        parseRelExpr();
        while (match(TOK_AND)) {
            advance();
            parseRelExpr();
        }
    }

    void parseRelExpr() {
        parseAddExpr();
        while (match(TOK_LT) || match(TOK_LE) || match(TOK_GT) || 
               match(TOK_GE) || match(TOK_EQ) || match(TOK_NE)) {
            advance();
            parseAddExpr();
        }
    }

    void parseAddExpr() {
        parseMulExpr();
        while (match(TOK_PLUS) || match(TOK_MINUS)) {
            advance();
            parseMulExpr();
        }
    }

    void parseMulExpr() {
        parseUnaryExpr();
        while (match(TOK_STAR) || match(TOK_DIV) || match(TOK_MOD)) {
            advance();
            parseUnaryExpr();
        }
    }

    void parseUnaryExpr() {
        if (match(TOK_PLUS) || match(TOK_MINUS) || match(TOK_NOT)) {
            advance();
            parseUnaryExpr();
        } else {
            parsePrimaryExpr();
        }
    }

    void parsePrimaryExpr() {
        if (match(TOK_ID)) {
            advance();
            if (match(TOK_LPAREN)) {
                // Function call
                advance();
                if (!match(TOK_RPAREN)) {
                    parseExpr();
                    while (match(TOK_COMMA)) {
                        advance();
                        parseExpr();
                    }
                }
                consume(TOK_RPAREN, "Lack of ')'");
            }
        } else if (match(TOK_NUMBER)) {
            advance();
        } else if (match(TOK_LPAREN)) {
            advance();
            parseExpr();
            consume(TOK_RPAREN, "Lack of ')'");
        } else {
            addError("Expected expression");
        }
    }

public:
    Parser(const vector<Token>& toks) : tokens(toks), pos(0), loopDepth(0) {}

    bool parse() {
        parseCompUnit();
        return errors.empty();
    }

    vector<pair<int, string>> getErrors() { return errors; }
};

int main() {
    // Read all input
    string input, line;
    while (getline(cin, line)) {
        input += line + "\n";
    }

    // Lexical analysis
    Lexer lexer(input);
    vector<Token> tokens;
    
    while (true) {
        Token tok = lexer.nextToken();
        if (tok.type == TOK_ERROR) continue;
        tokens.push_back(tok);
        if (tok.type == TOK_EOF) break;
    }

    auto lexErrors = lexer.getErrors();

    // Syntax analysis
    Parser parser(tokens);
    bool success = parser.parse();
    auto parseErrors = parser.getErrors();

    // Merge and output errors
    vector<pair<int, string>> allErrors = lexErrors;
    allErrors.insert(allErrors.end(), parseErrors.begin(), parseErrors.end());

    if (allErrors.empty() && success) {  // 修改这行，加入 success 检查
        cout << "accept" << endl;
    } else {
        cout << "reject" << endl;
        
        sort(allErrors.begin(), allErrors.end());
        
        for (const auto& err : allErrors) {
            cout << err.first << " " << err.second << endl;
        }
    }

    return 0;
}