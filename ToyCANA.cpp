#include <bits/stdc++.h>
using namespace std;

enum TokenType {
    TOK_INT, TOK_RETURN, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_CONTINUE, TOK_BREAK,
    TOK_IDENTIFIER, TOK_NUMBER,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_SEMI, TOK_COMMA,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_LE, TOK_GT, TOK_GE,
    TOK_AND, TOK_OR, TOK_NOT, TOK_ASSIGN,
    TOK_EOF, TOK_UNKNOWN
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
};

class Lexer {
    string src;
    int pos = 0;
    int line = 1;
public:
    Lexer(const string& s): src(s) {}
    vector<Token> tokens;

    bool isAlpha(char c){ return isalpha(c) || c=='_'; }
    bool isDigit(char c){ return isdigit(c); }

    void add(TokenType t, string lex="") {
        tokens.push_back({t, lex, line});
    }

    void scan() {
        while (pos < src.size()) {
            char c = src[pos];
            if (c==' '||c=='\r'||c=='\t') { pos++; continue; }
            if (c=='\n') { line++; pos++; continue; }
            if (isAlpha(c)) {
                int start=pos;
                while(pos<src.size()&&(isAlpha(src[pos])||isDigit(src[pos]))) pos++;
                string w=src.substr(start,pos-start);
                if(w=="int") add(TOK_INT);
                else if(w=="return") add(TOK_RETURN);
                else if(w=="if") add(TOK_IF);
                else if(w=="else") add(TOK_ELSE);
                else if(w=="while") add(TOK_WHILE);
                else if(w=="continue") add(TOK_CONTINUE);
                else if(w=="break") add(TOK_BREAK);
                else add(TOK_IDENTIFIER,w);
            } else if (isDigit(c)) {
                int start=pos;
                while(pos<src.size()&&isDigit(src[pos])) pos++;
                add(TOK_NUMBER, src.substr(start,pos-start));
            } else {
                switch(c) {
                    case '(': add(TOK_LPAREN); break;
                    case ')': add(TOK_RPAREN); break;
                    case '{': add(TOK_LBRACE); break;
                    case '}': add(TOK_RBRACE); break;
                    case ';': add(TOK_SEMI); break;
                    case ',': add(TOK_COMMA); break;
                    case '+': add(TOK_PLUS); break;
                    case '-': add(TOK_MINUS); break;
                    case '*': add(TOK_STAR); break;
                    case '/': add(TOK_SLASH); break;
                    case '%': add(TOK_PERCENT); break;
                    case '!':
                        if (src[pos+1]=='='){ add(TOK_NEQ); pos++; }
                        else add(TOK_NOT);
                        break;
                    case '=':
                        if (src[pos+1]=='='){ add(TOK_EQ); pos++; }
                        else add(TOK_ASSIGN);
                        break;
                    case '<':
                        if (src[pos+1]=='='){ add(TOK_LE); pos++; }
                        else add(TOK_LT);
                        break;
                    case '>':
                        if (src[pos+1]=='='){ add(TOK_GE); pos++; }
                        else add(TOK_GT);
                        break;
                    case '&':
                        if (src[pos+1]=='&'){ add(TOK_AND); pos++; }
                        break;
                    case '|':
                        if (src[pos+1]=='|'){ add(TOK_OR); pos++; }
                        break;
                    default:
                        add(TOK_UNKNOWN,string(1,c));
                }
                pos++;
            }
        }
        add(TOK_EOF);
    }
};

class Parser {
    vector<Token> toks;
    int idx = 0;
    bool accept = true;
    set<int> errLines;

    Token& cur(){ return toks[idx]; }
    bool match(TokenType t){ return cur().type==t; }
    void adv(){ if(idx<toks.size()-1) idx++; }

    void error(string msg){
        errLines.insert(cur().line);
        accept = false;
    }

    bool consume(TokenType t,const string& msg){
        if(match(t)){ adv(); return true; }
        error(msg);
        if(!match(TOK_EOF)) adv(); // 防止死循环
        return false;
    }

    void sync(){
        while(!match(TOK_EOF)){
            if(match(TOK_SEMI)||match(TOK_RBRACE)){ adv(); break; }
            adv();
        }
    }

public:
    Parser(vector<Token>& v): toks(v) {}

    void parse(){
        while(!match(TOK_EOF)){
            parseFunc();
        }
    }

    void parseFunc(){
        if(!match(TOK_INT)) { error("Missing return type"); sync(); return; }
        adv();
        if(!match(TOK_IDENTIFIER)) { error("Missing function name"); sync(); return; }
        adv();
        consume(TOK_LPAREN,"Lack of '('");
        if(match(TOK_INT)){
            parseParam();
            while(match(TOK_COMMA)){
                adv();
                parseParam();
            }
        }
        consume(TOK_RPAREN,"Lack of ')'");
        parseBlock();
    }

    void parseParam(){
        consume(TOK_INT,"Lack of 'int'");
        if(!match(TOK_IDENTIFIER)){ error("Lack of param name"); if(!match(TOK_EOF)) adv(); }
        else adv();
    }

    void parseBlock(){
        consume(TOK_LBRACE,"Lack of '{'");
        while(!match(TOK_RBRACE) && !match(TOK_EOF)){
            parseStmt();
        }
        consume(TOK_RBRACE,"Lack of '}'");
    }

    void parseStmt(){
        if(match(TOK_INT)){
            adv();
            consume(TOK_IDENTIFIER,"Missing var name");
            if(match(TOK_ASSIGN)){
                adv(); parseExpr();
            }
            consume(TOK_SEMI,"Lack of ';'");
        }
        else if(match(TOK_IDENTIFIER)){
            adv();
            if(match(TOK_ASSIGN)){
                adv(); parseExpr();
            } else if(match(TOK_LPAREN)){
                parseCall();
            }
            consume(TOK_SEMI,"Lack of ';'");
        }
        else if(match(TOK_RETURN)){
            adv(); parseExpr();
            consume(TOK_SEMI,"Lack of ';'");
        }
        else if(match(TOK_IF)){
            adv();
            consume(TOK_LPAREN,"Lack of '('");
            parseExpr();
            consume(TOK_RPAREN,"Lack of ')'");
            parseStmt();
            if(match(TOK_ELSE)){ adv(); parseStmt(); }
        }
        else if(match(TOK_WHILE)){
            adv();
            consume(TOK_LPAREN,"Lack of '('");
            parseExpr();
            consume(TOK_RPAREN,"Lack of ')'");
            parseStmt();
        }
        else if(match(TOK_CONTINUE)||match(TOK_BREAK)){
            adv(); consume(TOK_SEMI,"Lack of ';'");
        }
        else if(match(TOK_LBRACE)){
            parseBlock();
        }
        else {
            // 避免循环卡住
            error("Unexpected token");
            if(!match(TOK_EOF)) adv();
        }
    }

    void parseCall(){
        consume(TOK_LPAREN,"Lack of '('");
        if(!match(TOK_RPAREN)){
            parseExpr();
            while(match(TOK_COMMA)){
                adv();
                parseExpr();
            }
        }
        consume(TOK_RPAREN,"Lack of ')'");
    }

    void parseExpr(){
        parseTerm();
        while(match(TOK_PLUS)||match(TOK_MINUS)||match(TOK_OR)){
            adv();
            parseTerm();
        }
    }

    void parseTerm(){
        parseFactor();
        while(match(TOK_STAR)||match(TOK_SLASH)||match(TOK_PERCENT)||match(TOK_AND)){
            adv();
            parseFactor();
        }
    }

    void parseFactor(){
        if(match(TOK_NUMBER)||match(TOK_IDENTIFIER)){
            adv();
        } else if(match(TOK_LPAREN)){
            adv(); parseExpr(); consume(TOK_RPAREN,"Lack of ')'");
        } else if(match(TOK_MINUS)||match(TOK_NOT)||match(TOK_PLUS)){
            adv(); parseFactor();
        } else {
            error("Unexpected factor");
            if(!match(TOK_EOF)) adv();
        }
    }

    void printResult(){
        if(accept) cout << "accept\n";
        else {
            cout << "reject\n";
            for(int ln: errLines) cout << ln << " ";
            cout << "\n";
        }
    }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    stringstream buffer;
    buffer << cin.rdbuf();
    string code = buffer.str();

    Lexer lex(code);
    lex.scan();

    Parser parser(lex.tokens);
    parser.parse();
    parser.printResult();
    return 0;
}
