#include "stringparsers.h"
#include <vector>
#include <string.h>

#include "symboltable.h"
#include "registers.h"
//-----------------------------------------------------------------------------
bool StringParsers::ParseHexChar(char c, uint8_t& result)
{
    if (c >= '0' && c <= '9')
    {
        result = (uint8_t)(c - '0');
        return true;
    }
    if (c >= 'a' && c <= 'f')
    {
        result = (uint8_t)(10 + c - 'a');
        return true;
    }
    if (c >= 'A' && c <= 'F')
    {
        result = (uint8_t)(10 + c - 'A');
        return true;
    }
    result = 0;
    return false;
}

//-----------------------------------------------------------------------------
static bool ParseDecChar(char c, uint8_t& result)
{
    if (c >= '0' && c <= '9')
    {
        result = (uint8_t)(c - '0');
        return true;
    }
    result = 0;
    return false;
}

//-----------------------------------------------------------------------------
bool StringParsers::ParseHexString(const char *pText, uint32_t& result)
{
    result = 0;
    while (*pText != 0)
    {
        uint8_t val;
        if (!ParseHexChar(*pText, val))
        {
            result = 0;
            return false;
        }
        result <<= 4;
        result |= val;
        ++pText;
    }
    return true;
}

struct Token
{
    enum Type
    {
        CONSTANT,
        ADD,
        SUB,
        MUL,
        DIV,
        LEFT_BRACE,
        RIGHT_BRACE
    };

    uint64_t    val;        // for constants
    Type        type;
};

static bool IsWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

static bool IsDecimalDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool IsAlphaLower(char c)
{
    return c >= 'a' && c <= 'z';
}

static bool IsAlphaUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static bool IsAlpha(char c)
{
    return IsAlphaLower(c) || IsAlphaUpper(c);
}

static bool IsSymbolStart(char c)
{
    return IsAlpha(c) || c == '_';
}

static bool IsSymbolMain(char c)
{
    return IsAlpha(c) || IsDecimalDigit(c) || c == '_';
}

static bool IsRegisterName(const char* name, int& regId)
{
    for (int i = 0; i < Registers::REG_COUNT; ++i)
    {
        if (strcasecmp(name, Registers::s_names[i]) != 0)
            continue;

        regId = i;
        return true;
    }
    regId = Registers::REG_COUNT;
    return false;
}

uint64_t ApplyOp(uint64_t val1, uint64_t val2, Token::Type op)
{
    if (op == Token::ADD)
        return val1 + val2;
    if (op == Token::SUB)
        return val1 - val2;
    if (op == Token::MUL)
        return val1 * val2;
    if (op == Token::DIV)
        return val1 / val2;
    assert(0);
    return 0;
}

// Bigger the number, higher the precedence
static int Precedence(Token::Type type)
{
    switch (type) {
    case Token::ADD: case Token::SUB:
        return 1;
    case Token::MUL: case Token::DIV:
        return 2;
    case Token::CONSTANT:
        assert(0);
        return 0;
    default:
        break;
    }
    // Braces?
    return 0;
}

//-----------------------------------------------------------------------------
// Straight port of https://www.geeksforgeeks.org/expression-evaluation/
bool Evaluate(std::vector<Token>& tokens, uint64_t& result)
{
    // stack to store integer values.
    std::vector<uint64_t> values;

    // stack to store operators.
    std::vector<Token::Type> ops;

    for(size_t i = 0; i < tokens.size(); i++)
    {
        // Current token is an opening
        // brace, push it to 'ops'
        if (tokens[i].type == Token::LEFT_BRACE)
        {
            ops.push_back(tokens[i].type);
        }
        else if (tokens[i].type == Token::CONSTANT)
        {
            values.push_back(tokens[i].val);
        }

        // Closing brace encountered, solve
        // entire brace.
        else if(tokens[i].type == Token::RIGHT_BRACE)
        {
            while(!ops.empty() && ops.back() != Token::LEFT_BRACE)
            {
                if (values.size() < 2)
                    return false;
                uint64_t val2 = values.back();
                values.pop_back();

                uint64_t val1 = values.back();
                values.pop_back();

                Token::Type op = ops.back();
                ops.pop_back();

                uint64_t res = ApplyOp(val1, val2, op);
                values.push_back(res);
            }

            // pop opening brace.
            if(ops.size() != 0)
                ops.pop_back();
        }
        // Current token is an operator.
        else
        {
            // While top of 'ops' has same or greater
            // precedence to current token, which
            // is an operator. Apply operator on top
            // of 'ops' to top two elements in values stack.
            while((ops.size() != 0) &&
                  Precedence(ops.back()) >= Precedence(tokens[i].type))
            {
                if (values.size() < 2)
                    return false;
                uint64_t val2 = values.back();
                values.pop_back();

                uint64_t val1 = values.back();
                values.pop_back();

                Token::Type op = ops.back();
                ops.pop_back();

                values.push_back(ApplyOp(val1, val2, op));
            }

            // Push current token to 'ops'.
            ops.push_back(tokens[i].type);
        }
    }

    // Entire expression has been parsed at this
    // point, apply remaining ops to remaining
    // values.
    while(!ops.empty()){
        if (values.size() < 2)
            return false;

        uint64_t val2 = values.back();
        values.pop_back();

        uint64_t val1 = values.back();
        values.pop_back();

        Token::Type op = ops.back();
        ops.pop_back();

        values.push_back(ApplyOp(val1, val2, op));
    }
    result = values.back();
    return true;
}

void AddOperator(std::vector<Token>& tokens, Token::Type type)
{
    Token t;
    t.type = type;
    t.val = 0;
    tokens.push_back(t);
}

//-----------------------------------------------------------------------------
bool StringParsers::ParseExpression(const char *pText, uint32_t &result, const SymbolTable& syms, const Registers& regs)
{
    std::vector<Token> tokens;
    while (*pText != 0)
    {
        char head = *pText++;
        // Decide on next token type
        if (IsWhitespace(head))
        {
            ++pText;
            continue;
        }

        if (head == '$')
        {
            // Hex constant
            Token t;
            t.type = Token::CONSTANT;
            t.val = 0;
            uint8_t ch;
            while (ParseHexChar(*pText, ch))
            {
                t.val <<= 4;
                t.val |= ch;
                ++pText;
            }
            tokens.push_back(t);
            continue;
        }
        else if (IsDecimalDigit(head))
        {
            // Hex constant
            Token t;
            t.type = Token::CONSTANT;
            t.val = 0;
            uint8_t ch;
            --pText;        // go back to the first char
            while (ParseDecChar(*pText, ch))
            {
                t.val *= 10;
                t.val += ch;
                ++pText;
            }
            tokens.push_back(t);
            continue;
        }
        else if (IsSymbolStart(head))
        {
            // Possible symbol
            std::string name;
            name += head;
            while (IsSymbolMain(*pText))
                name += *pText++;

            // This might be a register name, use in preference to symbols
            int regId = 0;
            if (IsRegisterName(name.c_str(), regId))
            {
                Token t;
                t.type = Token::CONSTANT;
                t.val = regs.m_value[regId];
                tokens.push_back(t);
                continue;
            }

            // Try to look up symbols by name
            Symbol s;
            if (syms.Find(name, s))
            {
                Token t;
                t.type = Token::CONSTANT;
                t.val = s.address;
                tokens.push_back(t);
                continue;
            }
        }
        else if (head == '+')
        {
            AddOperator(tokens, Token::ADD);
            continue;
        }
        else if (head == '-')
        {
            AddOperator(tokens, Token::SUB);
            continue;
        }
        else if (head == '*')
        {
            AddOperator(tokens, Token::MUL);
            continue;
        }
        else if (head == '/')
        {
            AddOperator(tokens, Token::DIV);
            continue;
        }
        else if (head == '(')
        {
            AddOperator(tokens, Token::LEFT_BRACE);
            continue;
        }
        else if (head == ')')
        {
            AddOperator(tokens, Token::RIGHT_BRACE);
            continue;
        }
        // Couldn't match
        return false;
    }

    if (tokens.size() != 0)
    {
        uint64_t res2;
        bool success = Evaluate(tokens, res2);
        if (success)
        {
            result = (uint32_t)res2;
            return success;
        }
    }
    return false;
}
