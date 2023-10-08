// yes this is bad code!

#include <iostream>
#include <string>
#include <optional>
#include <vector>

typedef enum {
    NUMBER, 
    PLUS,
    MINUS,
    SLASH,
    STAR,
    OPEN_PAREN,
    CLOSE_PAREN, 
    _EOF,
} TokenType;

typedef struct {
    TokenType type;
    std::optional<std::string> value;
} Token;

typedef struct {
    Token left;
    Token op;
    Token right;    
} Expression;

typedef struct ASTNode {
    TokenType type;
    std::string value;
    std::vector<ASTNode> children;
} ASTNode;


// AST

// hehe....
ASTNode parseTerm(std::vector<Token> tokens, int& idx);


ASTNode parseNumber(std::vector<Token> tokens, int& idx)
{
    ASTNode node = {tokens.at(idx).type, tokens.at(idx).value.value()};
    idx++;
    return node;
} 

ASTNode parseExpression(std::vector<Token> tokens, int& idx)
{
    ASTNode left = parseTerm(tokens, idx);

    while (idx < tokens.size() && (tokens.at(idx).type == TokenType::PLUS || tokens.at(idx).type == TokenType::MINUS)) {
        TokenType op = tokens.at(idx).type;
        idx++;
        ASTNode right = parseTerm(tokens, idx);

        ASTNode newNode = {op, ""};
        newNode.children.push_back(left);
        newNode.children.push_back(right);
        left = newNode;
    }

    return left;
}

ASTNode parseFactor(std::vector<Token> tokens, int& idx)
{
    if (tokens.at(idx).type == TokenType::OPEN_PAREN) {
        // consume '('
        idx++;
        ASTNode node = parseExpression(tokens, idx);

        if (tokens.at(idx).type != TokenType::CLOSE_PAREN) {
            std::cerr << "Parenthesis never closed! Expected ')' at: " << idx << std::endl;
            std::exit(EXIT_FAILURE);
        }
        // consume ')'
        idx++;

        return node;
    } else {
        return parseNumber(tokens, idx);
    }
}

ASTNode parseTerm(std::vector<Token> tokens, int& idx)
{
    ASTNode left = parseFactor(tokens, idx);
    while (idx < tokens.size() && (tokens.at(idx).type == TokenType::STAR || tokens.at(idx).type == TokenType::SLASH)) {
        TokenType op = tokens.at(idx).type;
        idx++;
        ASTNode right = parseFactor(tokens, idx);

        ASTNode newNode = {op, ""};
        newNode.children.push_back(left);
        newNode.children.push_back(right);
        left = newNode;
    }

    return left;
}


void freeAST(ASTNode node) {
    for (ASTNode child : node.children) {
        free(&child);
    }
    delete &node;
}

float evaluateAST(ASTNode node)
{
    if (node.type == TokenType::NUMBER) {
        return std::stoi(node.value);
    } else {
        float leftVal = evaluateAST(node.children[0]);
        float rightVal = evaluateAST(node.children[1]);

        switch (node.type) {
            case TokenType::PLUS:   return leftVal + rightVal;
            case TokenType::MINUS:  return leftVal - rightVal;
            case TokenType::STAR:   return leftVal * rightVal;
            case TokenType::SLASH:  return leftVal / rightVal;
            // oopsies
            default: return 0;
        }
    }
}

void printAST(ASTNode node) {
    if (node.type == TokenType::NUMBER) {
        std::cout << node.value;
    } else {
        std::cout << "(";
        printAST(node.children[0]);

        switch (node.type) {
            case TokenType::PLUS: std::cout << " + "; break;
            case TokenType::MINUS: std::cout << " - "; break;
            case TokenType::STAR: std::cout << " * "; break;
            case TokenType::SLASH: std::cout << " / "; break;
            // oopsies (2)
            default: std::cout << " ??? ";
        }

        printAST(node.children[1]);
        std::cout << ")";
    }
}

// Lexer
std::vector<Token> tokenize(std::string src)
{
    std::vector<Token> tokens {};
    for (int i = 0; i < src.length(); i++) {
        switch (src.at(i)) {
            case '+': tokens.push_back({.type = TokenType::PLUS}); break;
            case '-': tokens.push_back({.type = TokenType::MINUS}); break;
            case '*': tokens.push_back({.type = TokenType::STAR}); break;
            case '/': tokens.push_back({.type = TokenType::SLASH}); break;
            case '(': tokens.push_back({.type = TokenType::OPEN_PAREN}); break;
            case ')': tokens.push_back({.type = TokenType::CLOSE_PAREN}); break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                break;
            default:
                if (std::isdigit(src.at(i))) {
                    std::string buf = "";
                    buf.push_back(src.at(i));

                    while (i + 1 < src.length() && std::isdigit(src.at(i + 1))) {
                        i++;
                        buf.push_back(src.at(i));
                    }
    
                    tokens.push_back({.type = TokenType::NUMBER, .value = buf});
                    break;
                } else {
                    std::cerr << "Unknown character in position " << i << ": \"" << src.at(i) << "\"" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
        }
    }
    return tokens;
}


// other stuff
void printUsage() 
{
    std::cout << "===== GeoWhat's Super Calculator (GSCalc) =====" << std::endl;
    std::cout << "Simply type: \"gscalc <expression>\" to parse" << std::endl;
    std::cout << "the expression." << std::endl;
    std::cout << "===============================================" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        printUsage();
        return EXIT_FAILURE;
    }

    std::string src;
    for (int i = 1; i < argc; i++) {
        src.append(argv[i]);
        src.append(" ");
    }
        
    std::vector<Token> tokens = tokenize(src);
    int idx = 0;
    ASTNode root = parseExpression(tokens, idx);


    float result = evaluateAST(root);
    std::cout << src << "= " << result << std::endl;

    return EXIT_SUCCESS;
}
