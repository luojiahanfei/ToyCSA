#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <cctype>

// ======================================================================
// 1. LEXER DEFINITIONS
// ======================================================================

// All the token types your grammar needs
enum class TokenType {
    // Keywords
    INT, VOID, IF, ELSE, WHILE, BREAK, CONTINUE, RETURN,
    
    // Identifiers and Literals
    ID, NUMBER,
    
    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    ASSIGN, // =
    EQ,     // ==
    NE,     // !=
    LT,     // <
    LE,     // <=
    GT,     // >
    GE,     // >=
    AND,    // &&
    OR,     // ||
    NOT,    // !
    
    // Punctuation
    LPAREN,   // (
    RPAREN,   // )
    LBRACE,   // {
    RBRACE,   // }
    SEMICOLON, // ;
    COMMA,     // ,
    
    // End of File
    EOFT
};

// A map to find keywords
const std::unordered_map<std::string, TokenType> keywords = {
    {"int", TokenType::INT},
    {"void", TokenType::VOID},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"return", TokenType::RETURN}
};

// Represents a single token
struct Token {
    TokenType type;
    std::string lexeme; // The actual text (e.g., "myVar", "123", "+")
    int line;           // Line number for error reporting

    // Helper for debugging
    std::string typeToString() const {
        switch (type) {
            case TokenType::INT: return "INT";
            case TokenType::VOID: return "VOID";
            case TokenType::IF: return "IF";
            case TokenType::ELSE: return "ELSE";
            case TokenType::WHILE: return "WHILE";
            case TokenType::BREAK: return "BREAK";
            case TokenType::CONTINUE: return "CONTINUE";
            case TokenType::RETURN: return "RETURN";
            case TokenType::ID: return "ID";
            case TokenType::NUMBER: return "NUMBER";
            case TokenType::PLUS: return "PLUS";
            case TokenType::MINUS: return "MINUS";
            case TokenType::STAR: return "STAR";
            case TokenType::SLASH: return "SLASH";
            case TokenType::PERCENT: return "PERCENT";
            case TokenType::ASSIGN: return "ASSIGN";
            case TokenType::EQ: return "EQ";
            case TokenType::NE: return "NE";
            case TokenType::LT: return "LT";
            case TokenType::LE: return "LE";
            case TokenType::GT: return "GT";
            case TokenType::GE: return "GE";
            case TokenType::AND: return "AND";
            case TokenType::OR: return "OR";
            case TokenType::NOT: return "NOT";
            case TokenType::LPAREN: return "LPAREN";
            case TokenType::RPAREN: return "RPAREN";
            case TokenType::LBRACE: return "LBRACE";
            case TokenType::RBRACE: return "RBRACE";
            case TokenType::SEMICOLON: return "SEMICOLON";
            case TokenType::COMMA: return "COMMA";
            case TokenType::EOFT: return "EOF";
            default: return "UNKNOWN";
        }
    }
};

// ======================================================================
// 2. LEXER CLASS
// ======================================================================

class Lexer {
public:
    Lexer(const std::string& source) : source(source), line(1), start(0), current(0) {}

    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.push_back({TokenType::EOFT, "", line});
        return tokens;
    }

private:
    const std::string& source;
    std::vector<Token> tokens;
    int start;
    int current;
    int line;

    bool isAtEnd() { return current >= source.length(); }
    char advance() { return source[current++]; }
    char peek() { return isAtEnd() ? '\0' : source[current]; }
    char peekNext() { return (current + 1 >= source.length()) ? '\0' : source[current + 1]; }

    bool match(char expected) {
        if (isAtEnd() || source[current] != expected) return false;
        current++;
        return true;
    }

    void addToken(TokenType type) {
        std::string text = source.substr(start, current - start);
        tokens.push_back({type, text, line});
    }

    void blockComment() {
        while (peek() != '*' || peekNext() != '/') {
            if (isAtEnd()) {
                // We'll let the parser report this error, but we need to stop
                std::cerr << "Line " << line << ": Unterminated block comment." << std::endl;
                return;
            }
            if (peek() == '\n') line++;
            advance();
        }
        advance(); // Consume '*'
        advance(); // Consume '/'
    }

    void identifier() {
        while (std::isalnum(peek()) || peek() == '_') advance();
        std::string text = source.substr(start, current - start);
        TokenType type = keywords.count(text) ? keywords.at(text) : TokenType::ID;
        addToken(type);
    }

    void number() {
        while (std::isdigit(peek())) advance();
        addToken(TokenType::NUMBER);
    }

    void scanToken() {
        char c = advance();
        switch (c) {
            case '(': addToken(TokenType::LPAREN); break;
            case ')': addToken(TokenType::RPAREN); break;
            case '{': addToken(TokenType::LBRACE); break;
            case '}': addToken(TokenType::RBRACE); break;
            case ';': addToken(TokenType::SEMICOLON); break;
            case ',': addToken(TokenType::COMMA); break;
            case '+': addToken(TokenType::PLUS); break;
            case '-': addToken(TokenType::MINUS); break;
            case '*': addToken(TokenType::STAR); break;
            case '%': addToken(TokenType::PERCENT); break;
            case '!': addToken(match('=') ? TokenType::NE : TokenType::NOT); break;
            case '=': addToken(match('=') ? TokenType::EQ : TokenType::ASSIGN); break;
            case '<': addToken(match('=') ? TokenType::LE : TokenType::LT); break;
            case '>': addToken(match('=') ? TokenType::GE : TokenType::GT); break;
            case '&': if (match('&')) addToken(TokenType::AND); else /* error */; break;
            case '|': if (match('|')) addToken(TokenType::OR); else /* error */; break;
            case '/':
                if (match('/')) while (peek() != '\n' && !isAtEnd()) advance();
                else if (match('*')) blockComment();
                else addToken(TokenType::SLASH);
                break;
            case ' ': case '\r': case '\t': break;
            case '\n': line++; break;
            default:
                if (std::isdigit(c)) number();
                else if (std::isalpha(c) || c == '_') identifier();
                else std::cerr << "Line " << line << ": Unexpected character '" << c << "'." << std::endl;
                break;
        }
    }
};

// ======================================================================
// 3. PARSER DEFINITIONS
// ======================================================================

// A struct to hold error information
struct ParseError {
    int line;
    std::string message;
};

// A "dummy" exception to unwind the stack for error recovery
class ParserException : public std::runtime_error {
public:
    ParserException() : std::runtime_error("") {}
};

// ======================================================================
// 4. PARSER CLASS
// ======================================================================

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

    // Getter for the errors
    const std::vector<ParseError>& getErrors() const { return errors; }

    // The main function to start parsing
    void parse() {
        try {
            parseCompUnit();
        } catch (const ParserException& e) {
            // This just means we stopped parsing.
            // The errors have already been logged.
        }
    }

private:
    std::vector<Token> tokens;
    int current;
    std::vector<ParseError> errors;

    // --- Grammar Functions ---

    // CompUnit → FuncDef+
    // (Assuming at least one function)
    void parseCompUnit() {
        while (!isAtEnd()) {
            parseFuncDef();
        }
    }
    
    // FuncDef → ("int" | "void") ID "(" (Param)? ")" Block
    void parseFuncDef() {
        // TODO: Implement this function.
        // 1. Check for 'int' or 'void'. Consume it.
        // 2. Consume an ID (the function name).
        //    -> THIS IS WHERE YOU ADD TO YOUR SYMBOL TABLE
        // 3. Consume an '('.
        // 4. Check if the next token is 'int' (meaning params). 
        //    If so, parseParam() (and handle commas for more params)
        // 5. Consume a ')'.
        // 6. Call parseBlock().
        //
        // Remember to use consume() and synchronize() on errors.

        // Example start:
        try {
            if (check(TokenType::INT) || check(TokenType::VOID)) {
                advance(); // Consume return type
            } else {
                logError(peek(), "Expected 'int' or 'void' function return type.");
                // We're totally lost, just stop? Or try to find next function?
                // synchronize(); // Let's try to recover
                return; // Let's just stop this function parse
            }

            consume(TokenType::ID, "Expected function name.");
            // TODO: Add to symbol table here.

            consume(TokenType::LPAREN, "Expected '(' after function name.");

            // TODO: Parse parameters here if they exist
            // if (check(TokenType::INT)) { ... }

            consume(TokenType::RPAREN, "Expected ')' after parameters.");
            parseBlock();

        } catch (const ParserException& e) {
            // An error was logged, try to recover
            synchronize();
        }
    }

    // Block → "{" Stmt* "}"
    void parseBlock() {
        consume(TokenType::LBRACE, "Expected '{' to start block.");
        
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            parseStmt();
        }
        
        consume(TokenType::RBRACE, "Expected '}' to end block.");
    }

    // Stmt → ... (this one is big)
    void parseStmt() {
        try {
            if (check(TokenType::IF)) {
                // TODO: Implement 'if' statement
                // consume(TokenType::IF);
                // consume(TokenType::LPAREN);
                // parseExpr();
                // consume(TokenType::RPAREN);
                // parseStmt();
                // if (check(TokenType::ELSE)) { ... }
            } 
            else if (check(TokenType::WHILE)) {
                // TODO: Implement 'while' statement
            }
            else if (check(TokenType::RETURN)) {
                // TODO: Implement 'return' statement
            }
            else if (check(TokenType::BREAK)) {
                // TODO: Implement 'break'
            }
            else if (check(TokenType::CONTINUE)) {
                // TODO: Implement 'continue'
            }
            else if (check(TokenType::LBRACE)) {
                // It's a new block
                parseBlock();
            }
            else if (check(TokenType::SEMICOLON)) {
                // Empty statement
                advance();
            }
            else {
                // Must be an expression statement
                // TODO: parseExpr();
                consume(TokenType::SEMICOLON, "Expected ';' after expression.");
            }
        } catch (const ParserException& e) {
            // Error was logged, recover to next statement
            synchronize();
        }
    }

    // Expr → LOrExpr
    void parseExpr() {
        // TODO: Implement this. It just calls the next level down.
        parseLOrExpr();
    }
    
    // ... all the other expression levels ...
    void parseLOrExpr()  { /* TODO: parseLAndExpr ( "||" parseLAndExpr )* */ }
    void parseLAndExpr() { /* TODO: parseRelExpr ( "&&" parseRelExpr )* */ }
    void parseRelExpr()  { /* TODO: parseAddExpr ( ("<" | ">" | ...) parseAddExpr )* */ }
    void parseAddExpr()  { /* TODO: parseMulExpr ( ("+" | "-") parseMulExpr )* */ }
    void parseMulExpr()  { /* TODO: parseUnaryExpr ( ("*" | "/" | "%") parseUnaryExpr )* */ }
    void parseUnaryExpr(){ /* TODO: ( ("+" | "-" | "!") parseUnaryExpr ) | parsePrimaryExpr */ }
    
    // PrimaryExpr → NUMBER | ID | ID "(" (Expr ("," Expr)*)? ")" | "(" Expr ")"
    void parsePrimaryExpr() { 
        // TODO: This is where you check for function calls!
        // if (check(TokenType::NUMBER)) { advance(); return; }
        // if (check(TokenType::LPAREN)) { ... }
        // if (check(TokenType::ID)) {
        //     Token id = advance();
        //     if (check(TokenType::LPAREN)) {
        //         // It's a function call!
        //         // TODO: Parse args, then check symbol table
        //         // against 'id.lexeme'
        //     } else {
        //         // It's just a variable
        //     }
        // }
    }


    // --- Utility Functions ---

    bool check(TokenType type) {
        return !isAtEnd() && peek().type == type;
    }
    
    bool isAtEnd() { return peek().type == TokenType::EOFT; }

    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    Token peek() { return tokens[current]; }
    Token previous() { return tokens[current - 1]; }

    Token consume(TokenType type, const std::string& errorMessage) {
        if (check(type)) return advance();
        logError(peek(), errorMessage);
        throw ParserException(); // Unwind stack
    }

    // --- Error Handling ---
    
    void logError(const Token& token, const std::string& message) {
        std::string fullMessage;
        if (token.type == TokenType::EOFT) {
            fullMessage = "at end: " + message;
        } else {
            fullMessage = "at '" + token.lexeme + "': " + message;
        }
        
        // Don't log duplicate errors for the same line
        if (errors.empty() || errors.back().line != token.line) {
             errors.push_back({token.line, fullMessage});
        }
    }

    void synchronize() {
        advance(); // Consume the bad token
        while (!isAtEnd()) {
            if (previous().type == TokenType::SEMICOLON) return; // Found end of a statement
            switch (peek().type) {
                // These tokens often start a new "thing"
                case TokenType::RBRACE:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::RETURN:
                case TokenType::INT:
                case TokenType::VOID:
                    return;
                default:
                    advance(); // Keep consuming
            }
        }
    }
};

// ======================================================================
// 5. MAIN FUNCTION
// ======================================================================

int main() {
    // Read all of standard input into a string
    std::stringstream buffer;
    buffer << std::cin.rdbuf();
    std::string source = buffer.str();

    // 1. Lexing
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    // 2. Parsing
    Parser parser(tokens);
    parser.parse(); // Run the parser

    // 3. Report results
    const auto& errors = parser.getErrors();
    if (errors.empty()) {
        std::cout << "accept" << std::endl;
    } else {
        std::cout << "reject" << std::endl;
        for (const auto& err : errors) {
            // This matches the format: <行号> [空格] <报错信息>
            // std::cout << err.line << " " << err.message << std::endl;
            
            // This matches the minimal format: <行号>
            std::cout << err.line << std::endl;
        }
    }

    return 0;
}