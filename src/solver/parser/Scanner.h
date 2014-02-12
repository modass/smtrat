/*
 *  SMT-RAT - Satisfiability-Modulo-Theories Real Algebra Toolbox
 * Copyright (C) 2012 Florian Corzilius, Ulrich Loup, Erika Abraham, Sebastian Junges
 *
 * This file is part of SMT-RAT.
 *
 * SMT-RAT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SMT-RAT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SMT-RAT.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


/**
 * @file Scanner.h
 *
 * @author Florian Corzilius
 * @since 2012-03-19
 * @version 2012-03-19
 */

#ifndef SMTTWO_SCANNER_H
#define SMTTWO_SCANNER_H

// Flex expects the signature of yylex to be defined in the macro YY_DECL, and
// the C++ parser expects it to be declared. We can factor both as follows.

#ifndef YY_DECL

#define YY_DECL                     \
    smtrat::Parser::token_type              \
    smtrat::Scanner::lex(               \
    smtrat::Parser::semantic_type* yylval,      \
    smtrat::Parser::location_type* yylloc       \
    )
#endif

#ifndef __FLEX_LEXER_H
#define yyFlexLexer smtratFlexLexer

#include "FlexLexer.h"

#undef yyFlexLexer
#endif

#include <vector>
#include <unordered_map>
#include "../../lib/Common.h"
#include <string>
#include <unordered_set>
CLANG_WARNING_DISABLE("-Wsign-conversion")
CLANG_WARNING_DISABLE("-Wshorten-64-to-32")
CLANG_WARNING_DISABLE("-Wconversion")
#include "Parser.tab.hh"

using namespace std;

namespace smtrat
{
    typedef std::unordered_set<std::string> FastStringSet;

    /** Scanner is a derived class to add some extra function to the scanner
     * class. Flex itself creates a class named yyFlexLexer, which is renamed using
     * macros to smtratFlexLexer. However we change the context of the generated
     * yylex() function to be contained within the Scanner class. This is required
     * because the yylex() defined in smtratFlexLexer has no parameters. */
    class Scanner:
        public smtratFlexLexer
    {
        public:
            int mInPolynomial;
            FastStringSet mTheoryVariables;
            FastStringSet mBooleanVariables;
            /** Create a new scanner object. The streams arg_yyin and arg_yyout default
             * to cin and cout, but that assignment is only made when initializing in
             * yylex(). */
            Scanner( std::istream* arg_yyin = 0, std::ostream* arg_yyout = 0 );

            /* * Required for virtual functions */
            virtual ~Scanner();

            /** This is the main lexing function. It is generated by flex according to
             * the macro declaration YY_DECL above. The generated bison parser then
             * calls this virtual function to fetch new tokens. */
            virtual Parser::token_type lex( Parser::semantic_type* yylval, Parser::location_type* yylloc );

            /* * Enable debug output (via arg_yyout) if compiled into the scanner. */
            void set_debug( bool b );
    };

}    // namespace smtrat

#endif // SMTTWO_SCANNER_H
