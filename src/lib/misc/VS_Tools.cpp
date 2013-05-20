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
 * Class containing auxiliary methods used especially by the virtual substitution method.
 * @author Florian Corzilius
 * @since 2013-05-20
 * @version 2013-05-20
 */

#include "VS_Tools.hpp"

using namespace std;
using namespace GiNaC;

namespace vs
{
    /**
     * Simplifies a disjunction of conjunctions of constraints by deleting consistent
     * constraint and inconsistent conjunctions of constraints. If a conjunction of
     * only consistent constraints exists, the simplified disjunction contains one
     * empty conjunction.
     *
     * @param _toSimplify   The disjunction of conjunctions to simplify.
     */
    void simplify( DisjunctionOfConstraintConjunctions& _toSimplify )
    {
        bool                                          containsEmptyDisjunction = false;
        DisjunctionOfConstraintConjunctions::iterator conj                     = _toSimplify.begin();
        while( conj != _toSimplify.end() )
        {
            bool                               conjInconsistent = false;
            TS_ConstraintConjunction::iterator cons             = (*conj).begin();
            while( cons != (*conj).end() )
            {
                unsigned consConsistent = (**cons).isConsistent();
                if( consConsistent == 0 )
                {
                    conjInconsistent = true;
                    break;
                }
                else if( consConsistent == 1 )
                {
                    // Delete the constraint.
                    cons = (*conj).erase( cons );
                }
                else
                {
                    cons++;
                }
            }
            bool conjEmpty = (*conj).empty();
            if( conjInconsistent || (containsEmptyDisjunction && conjEmpty) )
            {
                // Delete the conjunction.
                (*conj).clear();
                conj = _toSimplify.erase( conj );
            }
            else
            {
                conj++;
            }
            if( !containsEmptyDisjunction && conjEmpty )
            {
                containsEmptyDisjunction = true;
            }
        }
    }

    /**
     *
     * @param _toSimplify
     */
    void splitProducts( DisjunctionOfConstraintConjunctions& _toSimplify )
    {
        unsigned toSimpSize = _toSimplify.size();
        for( unsigned pos = 0; pos < toSimpSize; )
        {
            if( !_toSimplify.begin()->empty() )
            {
                DisjunctionOfConstraintConjunctions temp = splitProducts( _toSimplify[pos] );
                _toSimplify.erase( _toSimplify.begin() );
                _toSimplify.insert( _toSimplify.end(), temp.begin(), temp.end() );
                --toSimpSize;
            }
            else
            {
                ++pos;
            }
        }
    }

    /**
     *
     * @param _constraintConjunction
     * @return
     */
    DisjunctionOfConstraintConjunctions splitProducts( const TS_ConstraintConjunction& _constraintConjunction )
    {
        DisjunctionOfConstraintConjunctions result = DisjunctionOfConstraintConjunctions();
        vector<DisjunctionOfConstraintConjunctions> toCombine = vector<DisjunctionOfConstraintConjunctions>();
        for( auto constraint = _constraintConjunction.begin(); constraint != _constraintConjunction.end(); ++constraint )
        {
            if( (*constraint)->hasFactorization() )
            {
                switch( (*constraint)->relation() )
                {
                    case smtrat::CR_EQ:
                    {
                        toCombine.push_back( DisjunctionOfConstraintConjunctions() );
                        const ex& factorization = (*constraint)->factorization();
                        for( GiNaC::const_iterator summand = factorization.begin(); summand != factorization.end(); ++summand )
                        {
                            const smtrat::Constraint* cons = smtrat::Formula::newConstraint( *summand, smtrat::CR_EQ, (*constraint)->variables() );
                            toCombine.back().push_back( TS_ConstraintConjunction() );
                            toCombine.back().back().push_back( cons );
                        }
                        break;
                    }
                    case smtrat::CR_NEQ:
                    {
                        toCombine.push_back( DisjunctionOfConstraintConjunctions() );
                        toCombine.back().push_back( TS_ConstraintConjunction() );
                        const ex& factorization = (*constraint)->factorization();
                        for( GiNaC::const_iterator summand = factorization.begin(); summand != factorization.end(); ++summand )
                        {
                            const smtrat::Constraint* cons = smtrat::Formula::newConstraint( *summand, smtrat::CR_NEQ, (*constraint)->variables() );
                            toCombine.back().back().push_back( cons );
                        }
                        break;
                    }
                    default:
                    {
                        toCombine.push_back( getSignCombinations( *constraint ) );
                    }
                    simplify( toCombine.back() );
                }
            }
            else
            {
                toCombine.push_back( DisjunctionOfConstraintConjunctions() );
                toCombine.back().push_back( TS_ConstraintConjunction() );
                toCombine.back().back().push_back( *constraint );
            }
        }
        combine( toCombine, result );
        simplify( result );
        return result;
    }

    /**
     *
     * @param _constraintConjunction
     * @return
     */
    DisjunctionOfConstraintConjunctions splitProducts( const smtrat::Constraint* _constraint )
    {
        DisjunctionOfConstraintConjunctions result = DisjunctionOfConstraintConjunctions();
        if( _constraint->hasFactorization() )
        {
            switch( _constraint->relation() )
            {
                case smtrat::CR_EQ:
                {
                    const ex& factorization = _constraint->factorization();
                    for( GiNaC::const_iterator summand = factorization.begin(); summand != factorization.end(); ++summand )
                    {
                        const smtrat::Constraint* cons = smtrat::Formula::newConstraint( *summand, smtrat::CR_EQ, _constraint->variables() );
                        result.push_back( TS_ConstraintConjunction() );
                        result.back().push_back( cons );
                    }
                    break;
                }
                case smtrat::CR_NEQ:
                {
                    result.push_back( TS_ConstraintConjunction() );
                    const ex& factorization = _constraint->factorization();
                    for( GiNaC::const_iterator summand = factorization.begin(); summand != factorization.end(); ++summand )
                    {
                        const smtrat::Constraint* cons = smtrat::Formula::newConstraint( *summand, smtrat::CR_NEQ, _constraint->variables() );
                        result.back().push_back( cons );
                    }
                    break;
                }
                default:
                {
                    result = getSignCombinations( _constraint );
                }
                simplify( result );
            }
        }
        else
        {
            result.push_back( TS_ConstraintConjunction() );
            result.back().push_back( _constraint );
        }
        return result;
    }

    /**
     *
     * @param _product
     * @param _positive
     * @param _zero
     * @param _variables
     * @return
     */
    DisjunctionOfConstraintConjunctions getSignCombinations( const smtrat::Constraint* _constraint )
    {
        DisjunctionOfConstraintConjunctions combinations = DisjunctionOfConstraintConjunctions();
        if( _constraint->hasFactorization() && _constraint->factorization().nops() <= MAX_PRODUCT_SPLIT_NUMBER )
        {
            if( !(_constraint->relation() == smtrat::CR_GREATER || _constraint->relation() == smtrat::CR_LESS
                    || _constraint->relation() == smtrat::CR_GEQ || _constraint->relation() == smtrat::CR_LEQ ))
            {
                cout << *_constraint << endl;
            }
            assert( _constraint->relation() == smtrat::CR_GREATER || _constraint->relation() == smtrat::CR_LESS
                    || _constraint->relation() == smtrat::CR_GEQ || _constraint->relation() == smtrat::CR_LEQ );
            smtrat::Constraint_Relation relPos = smtrat::CR_GREATER;
            smtrat::Constraint_Relation relNeg = smtrat::CR_LESS;
            if( _constraint->relation() == smtrat::CR_GEQ || _constraint->relation() == smtrat::CR_LEQ )
            {
                relPos = smtrat::CR_GEQ;
                relNeg = smtrat::CR_LEQ;
            }
            bool positive = (_constraint->relation() == smtrat::CR_GEQ || _constraint->relation() == smtrat::CR_GREATER);
            TS_ConstraintConjunction positives = TS_ConstraintConjunction();
            TS_ConstraintConjunction alwayspositives = TS_ConstraintConjunction();
            TS_ConstraintConjunction negatives = TS_ConstraintConjunction();
            TS_ConstraintConjunction alwaysnegatives = TS_ConstraintConjunction();
            unsigned numOfAlwaysNegatives = 0;
            const ex& product = _constraint->factorization();
            for( GiNaC::const_iterator summand = product.begin(); summand != product.end(); ++summand )
            {
                const smtrat::Constraint* consPos = smtrat::Formula::newConstraint( *summand, relPos, _constraint->variables() );
                unsigned posConsistent = consPos->isConsistent();
                if( posConsistent != 0 )
                {
                    positives.push_back( consPos );
                }
                const smtrat::Constraint* consNeg = smtrat::Formula::newConstraint( *summand, relNeg, _constraint->variables() );
                unsigned negConsistent = consNeg->isConsistent();
                if( negConsistent == 0 )
                {
                    if( posConsistent == 0 )
                    {
                        combinations.push_back( TS_ConstraintConjunction() );
                        combinations.back().push_back( consNeg );
                        return combinations;
                    }
                    if( posConsistent != 1 ) alwayspositives.push_back( positives.back() );
                    positives.pop_back();
                }
                else
                {
                    if( posConsistent == 0 )
                    {
                        ++numOfAlwaysNegatives;
                        if( negConsistent != 1 ) alwaysnegatives.push_back( consNeg );
                    }
                    else negatives.push_back( consNeg );
                }
            }
            assert( positives.size() == negatives.size() );
            if( positives.size() > 0 )
            {
                vector< bitset<MAX_PRODUCT_SPLIT_NUMBER> > combSelector = vector< bitset<MAX_PRODUCT_SPLIT_NUMBER> >();
                if( fmod( numOfAlwaysNegatives, 2.0 ) != 0.0 )
                {
                    if( positive ) getOddBitStrings( positives.size(), combSelector );
                    else getEvenBitStrings( positives.size(), combSelector );
                }
                else
                {
                    if( positive ) getEvenBitStrings( positives.size(), combSelector );
                    else getOddBitStrings( positives.size(), combSelector );
                }
                for( auto comb = combSelector.begin(); comb != combSelector.end(); ++comb )
                {
                    combinations.push_back( TS_ConstraintConjunction( alwaysnegatives ) );
                    combinations.back().insert( combinations.back().end(), alwayspositives.begin(), alwayspositives.end() );
                    for( unsigned pos = 0; pos < positives.size(); ++pos )
                    {
                        if( (*comb)[pos] ) combinations.back().push_back( negatives[pos] );
                        else combinations.back().push_back( positives[pos] );
                    }
                }
            }
            else
            {
                combinations.push_back( TS_ConstraintConjunction( alwaysnegatives ) );
                combinations.back().insert( combinations.back().end(), alwayspositives.begin(), alwayspositives.end() );
            }
        }
        else
        {
            combinations.push_back( TS_ConstraintConjunction() );
            combinations.back().push_back( _constraint );
        }
        return combinations;
    }

    /**
     *
     * @param _length
     * @param _strings
     */
    void getOddBitStrings( unsigned _length, vector< bitset<MAX_PRODUCT_SPLIT_NUMBER> >& _strings )
    {
        assert( _length > 0 );
        if( _length == 1 )  _strings.push_back( bitset<MAX_PRODUCT_SPLIT_NUMBER>( 1 ) );
        else
        {
            unsigned pos = _strings.size();
            getEvenBitStrings( _length - 1, _strings );
            for( ; pos < _strings.size(); ++pos )
            {
                _strings[pos] <<= 1;
                _strings[pos].flip(0);
            }
            getOddBitStrings( _length - 1, _strings );
            for( ; pos < _strings.size(); ++pos ) _strings[pos] <<= 1;
        }
    }

    /**
     *
     * @param _length
     * @param _strings
     */
    void getEvenBitStrings( unsigned _length, vector< bitset<MAX_PRODUCT_SPLIT_NUMBER> >& _strings )
    {
        assert( _length > 0 );
        if( _length == 1 ) _strings.push_back( bitset<MAX_PRODUCT_SPLIT_NUMBER>( 0 ) );
        else
        {
            unsigned pos = _strings.size();
            getEvenBitStrings( _length - 1, _strings );
            for( ; pos < _strings.size(); ++pos ) _strings[pos] <<= 1;
            getOddBitStrings( _length - 1, _strings );
            for( ; pos < _strings.size(); ++pos )
            {
                _strings[pos] <<= 1;
                _strings[pos].flip(0);
            }
        }
    }

    /**
     *
     * @param _expression
     * @param _variables
     * @return
     */
    ex simplify( const ex& _expression, const symtab& _variables )
    {
        for( symtab::const_iterator var = _variables.begin(); var != _variables.end(); ++var )
        {
            if( _expression.has( var->second ) )
            {
                ex un, con, prim;
                _expression.unitcontprim( var->second, un, con, prim );
                if( con.info( info_flags::rational ) )
                {
                    return prim * un;
                }
                return _expression;
            }
        }
        return _expression;
    }

    /**
     *
     * @param _substitutionResults
     */
    void print( DisjunctionOfConstraintConjunctions& _substitutionResults )
    {
        DisjunctionOfConstraintConjunctions::const_iterator conj = _substitutionResults.begin();
        while( conj != _substitutionResults.end() )
        {
            if( conj != _substitutionResults.begin() )
            {
                cout << " or (";
            }
            else
            {
                cout << "    (";
            }
            TS_ConstraintConjunction::const_iterator cons = (*conj).begin();
            while( cons != (*conj).end() )
            {
                if( cons != (*conj).begin() )
                {
                    cout << " and ";
                }
                (**cons).print( cout );
                cons++;
            }
            cout << ")" << endl;
            conj++;
        }
        cout << endl;
    }
}
