#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "grammar.h"


class Lexer
{
    public:

        explicit Lexer(const Grammar& grammar, const std::string& sentence);

}