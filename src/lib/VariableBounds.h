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
 * @file VariableBounds.h
 * @author Florian Corzilius
 * @since 2012-10-04
 * @version 2013-02-05
 */

#ifndef VARIABLEBOUNDS_H
#define	VARIABLEBOUNDS_H

#include "Constraint.h"
#include <iomanip>

namespace smtrat
{
    namespace vb
    {
        template <class T> class Variable;

        /**
         * Class for the bound of a variable.
         */
        template <class T> class Bound
        {
            public:
                enum Type { STRICT_LOWER_BOUND = 0, WEAK_LOWER_BOUND = 1, EQUAL_BOUND = 2, WEAK_UPPER_BOUND = 3, STRICT_UPPER_BOUND = 4 };

            private:
                /// The type of this bound.
                Type                        mType;
                /// A pointer to bound value, which is plus or minus infinity if the pointer is NULL.
                Rational*                   mpLimit;
                /// The variable for which the bound is declared.
                Variable<T>* const          mpVariable;
                /// A set of origins of the bound, e.g., x-3<0 is the origin of the bound <3.
                std::set< const T* >* const mpOrigins;
                
            public:
                
                /**
                 * Constructor for a bound.
                 * @param _limit A pointer to the limit (rational) of the bound. It is NULL, if the limit is not finite.
                 * @param _variable The variable to which this bound belongs.
                 * @param _type The type of the bound (either it is an equal bound or it is weak resp. strict and upper resp. lower)
                 */
                Bound( Rational* const _limit, Variable<T>* const _variable, Type _type );

                /*
                 * Destructor:
                 */
                ~Bound();
                
                /**
                 * Checks whether the this bound is smaller than the given one.
                 * @param _bound The bound to compare with.
                 * @return true,    if for this bound (A) and the given bound (B) it holds that:
                 *                      A is not inf   and   B is inf,
                 *                      A smaller than B, where A and B are rationals,
                 *                      A equals B, where A and B are rationals, but A is an equal bound but B is not;
                 *          false,   otherwise.
                 */
                bool operator<( const Bound<T>& _bound ) const;
                
                /**
                 * Prints the bound on the given output stream.
                 * @param _out The output stream to print on.
                 * @param _bound The bound to print.
                 * @return The output stream after printing the bound on it.
                 */
                template <class T1> friend std::ostream& operator<<( std::ostream& _out, const Bound<T1>& _bound );
                
                /**
                 * Prints this bound on the given stream.
                 * @param _out The stream to print on.
                 * @param _withRelation A flag indicating whether to print also a relation symbol in front of the bound value.
                 */
                void print( std::ostream& _out, bool _withRelation = false ) const;

                /**
                 * @return A constant reference to the value of the limit. Note, that it must be ensured that
                 *          the bound is finite before calling this method.
                 */
                const Rational& limit() const
                {
                    return *mpLimit;
                }

                /**
                 * @return A pointer to the limit of this bound.
                 */
                const Rational* const pLimit() const
                {
                    return mpLimit;
                }

                /**
                 * @return true, if the bound infinite;
                 *          false, otherwise.
                 */
                bool isInfinite() const
                {
                    return mpLimit == NULL;
                }

                /**
                 * @return The type of this bound.
                 */
                Type type() const
                {
                    return mType;
                }

                /**
                 * @return true, if the bound is an upper bound;
                 *          false, otherwise.
                 */
                bool isUpperBound() const
                {
                    return mType > WEAK_LOWER_BOUND;
                }
                
                /**
                 * @return true, if the bound is a lower bound;
                 *          false, otherwise.
                 */
                bool isLowerBound() const
                {
                    return mType < WEAK_UPPER_BOUND;
                }

                /**
                 * @return A pointer to the variable wrapper considered by this bound.
                 */
                Variable<T>* const pVariable() const
                {
                    return mpVariable;
                }

                /**
                 * @return A constant reference to the variable wrapper considered by this bound.
                 */
                const Variable<T>& variable() const
                {
                    return *mpVariable;
                }

                /**
                 * @return true, if this bound is active, which means that there is at least one origin for it left.
                 *                Note, that origins can be removed belatedly.
                 *          false, otherwise.
                 */
                bool isActive() const
                {
                    return !mpOrigins->empty();
                }

                /**
                 * Adds an origin to this bound.
                 * @param _origin The origin to add.
                 * @return true, if this has activated this bound;
                 *          false, if the bound was already active before.
                 */
                bool activate( const T* _origin ) const
                {
                    mpOrigins->insert( _origin );
                    return mpOrigins->size() == 1;
                }
                
                /**
                 * Removes an origin from this bound.
                 * @param _origin The origin to add.
                 * @return true, if this has deactivated this bound;
                 *          false, if the bound was already inactive before.
                 */
                bool deactivate( const T* _origin ) const
                {
                    assert( mpOrigins->find( _origin ) != mpOrigins->end() );
                    mpOrigins->erase( _origin );
                    return mpOrigins->empty();
                }

                /**
                 * @return A constant reference to the set of origins of this bound.
                 */
                const std::set< const T* >& origins() const
                {
                    return *mpOrigins;
                }
        };

        /**
         * Class for a variable.
         */
        template <class T> class Variable
        {
            struct boundPointerComp
            {
                bool operator ()( const Bound<T>* const pBoundA, const Bound<T>* const pBoundB ) const
                {
                    return (*pBoundA) < (*pBoundB);
                }
            };

            typedef std::set<const Bound<T>*, boundPointerComp> BoundSet;
            private:
                /// A flag that indicates that the stored exact interval of this variable is up to date.
                bool            mUpdatedExactInterval;
                /// A flag that indicates that the stored double interval of this variable is up to date.
                bool            mUpdatedDoubleInterval;
                /// The least upper bound of this variable.
                const Bound<T>* mpSupremum;
                /// The greatest lower bound of this variable.
                const Bound<T>* mpInfimum;
                /// The set of all upper bounds of this variable.
                BoundSet        mUpperbounds;
                /// The set of all lower bounds of this variable.
                BoundSet        mLowerbounds;

            public:
                /*
                 * Constructors:
                 */
                Variable();

                /*
                 * Destructor:
                 */
                ~Variable();

                /**
                 * @return true, if there is a conflict;
                 *          false, otherwise.
                 */
                bool conflicting() const;
                
                /**
                 * Adds the bound corresponding to the constraint to the given variable. The constraint
                 * is expected to contain only one variable and this one only linearly.
                 * @param _constraint A pointer to a constraint of the form ax~b.
                 * @param _var Hence, the variable x.
                 * @param _origin The origin of the constraint. This is could be the constraint itself or anything else 
                 *                 in the data structure which uses this object.
                 * @param _limit
                 * @return The added bound.
                 */
                const Bound<T>* addBound( const Constraint* _constraint, const carl::Variable& _var, const T* _origin );
                
                /**
                 * Updates the infimum and supremum of this variable.
                 * @param _changedBound The bound, for which we certainly know that it got deactivated before.
                 * @return true, if there is a conflict;
                 *          false, otherwise.
                 */
                bool updateBounds( const Bound<T>& _changedBound );

                /**
                 * @return true, if the stored exact interval representing the variable's bounds is up to date.
                 *          false, otherwise
                 */
                bool updatedExactInterval() const
                {
                    return mUpdatedExactInterval;
                }

                /**
                 * Sets the flag indicating that the stored exact interval representing the variable's bounds is up to date to false.
                 */
                void exactIntervalHasBeenUpdated()
                {
                    mUpdatedExactInterval = false;
                }
                
                /**
                 * @return true, if the stored double interval representing the variable's bounds is up to date.
                 *          false, otherwise
                 */
                bool updatedDoubleInterval() const
                {
                    return mUpdatedDoubleInterval;
                }
                
                /**
                 * Sets the flag indicating that the stored double interval representing the variable's bounds is up to date to false.
                 */
                void doubleIntervalHasBeenUpdated()
                {
                    mUpdatedDoubleInterval = false;
                }

                /**
                 * @return A pointer to the supremum of this variable.
                 */
                const Bound<T>* pSupremum() const
                {
                    return mpSupremum;
                }

                /**
                 * @return A constant reference to the supremum of this variable.
                 */
                const Bound<T>& supremum() const
                {
                    return *mpSupremum;
                }

                /**
                 * @return A pointer to the infimum of this variable.
                 */
                const Bound<T>* pInfimum() const
                {
                    return mpInfimum;
                }

                /**
                 * @return A constant reference to the infimum of this variable.
                 */
                const Bound<T>& infimum() const
                {
                    return *mpInfimum;
                }

                /**
                 * @return A constant reference to the set of upper bounds of this variable.
                 */
                const BoundSet& upperbounds() const
                {
                    return mUpperbounds;
                }
                
                /**
                 * @return A constant reference to the set of lower bounds of this variable.
                 */
                const BoundSet& lowerbounds() const
                {
                    return mLowerbounds;
                }
        };

        /**
         * Class to manage the bounds of a variable.
         */
        template <class T> class VariableBounds
        {
            public:
                typedef FastPointerMap<Constraint,const Bound<T>*> ConstraintBoundMap;
                typedef FastMap<carl::Variable, Variable<T>*>      VariableMap;
            private:
                /// A pointer to one of the conflicting variables (its supremum is smaller than its infimum)
                /// or NULL if there is no conflict.
                Variable<T>*          mpConflictingVariable;
                /// A mapping from arithmetic variables to the variable objects storing the bounds.
                VariableMap*          mpVariableMap;
                /// A mapping from constraint pointer to the corresponding bounds.
                ConstraintBoundMap*   mpConstraintBoundMap;
                /// The stored exact interval map representing the currently tightest bounds.
                /// Note, that it is updated on demand.
                EvalIntervalMap       mEvalIntervalMap;
                /// The stored double interval map representing the currently tightest bounds.
                /// Note, that it is updated on demand.
                EvalDoubleIntervalMap mDoubleIntervalMap;
            public:
                /*
                 * Constructors:
                 */
                VariableBounds();

                /*
                 * Destructor:
                 */
                ~VariableBounds();
                
                /**
                 * Updates the variable bounds by the given constraint.
                 * @param _constraint The constraint to consider.
                 * @param _origin The origin of the given constraint.
                 * @return true, if the variable bounds have been changed;
                 *          false, otherwise.
                 */
                bool addBound( const Constraint* _constraint, const T* _origin );
                
                /**
                 * Removes all effects the given constraint has on the variable bounds.
                 * @param _constraint The constraint, which effects shall be undone for the variable bounds.
                 * @param _origin The origin of the given constraint.
                 * @return 2, if the variables supremum or infimum have been changed;
                 *          1, if the constraint was a (not the strictest) bound;
                 *          0, otherwise.
                 */
                unsigned removeBound( const Constraint* _constraint, const T* _origin );
                
                /**
                 * Removes all effects the given constraint has on the variable bounds.
                 * @param _constraint The constraint, which effects shall be undone for the variable bounds.
                 * @param _origin The origin of the given constraint.
                 * @param _changedVariable The variable whose interval domain has been changed, if the return value is 2.
                 * @return 2, if the variables supremum or infimum have been changed;
                 *          1, if the constraint was a (not the strictest) bound;
                 *          0, otherwise.
                 */
                unsigned removeBound( const Constraint* _constraint, const T* _origin, carl::Variable*& _changedVariable );
                
                /**
                 * Creates an evalintervalmap corresponding to the variable bounds.
                 * @return The variable bounds as an evalintervalmap.
                 */
                const EvalIntervalMap& getEvalIntervalMap();
                
                /**
                 * Creates an interval corresponding to the variable bounds of the given variable.
                 * @param _var The variable to compute the variable bounds as interval for.
                 * @return The variable bounds as an interval.
                 */
                const Interval& getInterval( const carl::Variable& _var );
                
                /**
                 * Creates an interval map corresponding to the variable bounds.
                 * @return The variable bounds as an interval map.
                 */
                const smtrat::EvalDoubleIntervalMap& getIntervalMap();
                
                /**
                 * Creates a double interval corresponding to the variable bounds of the given variable.
                 * @param _var The variable to compute the variable bounds as double interval for.
                 * @return The variable bounds as a double interval.
                 */
                const carl::DoubleInterval& getDoubleInterval( const carl::Variable& _var );
                
                /**
                 * @param _var The variable to get origins of the bounds for.
                 * @return The origin constraints of the supremum and infimum of the given variable.
                 */
                std::set< const T* > getOriginsOfBounds( const carl::Variable& _var ) const;
                
                /**
                 * @param _variables The variables to get origins of the bounds for.
                 * @return The origin constraints of the supremum and infimum of the given variables.
                 */
                std::set< const T* > getOriginsOfBounds( const Variables& _variables ) const;
                
                /**
                 * Collect the origins to the supremums and infimums of all variables.
                 * @return A set of origins corresponding to the supremums and infimums of all variables.
                 */
                std::set< const T* > getOriginsOfBounds() const;
                
                /**
                 * Prints the variable bounds.
                 * @param _out The output stream to print on.
                 * @param _init A string which is displayed at the beginning of every row.
                 * @param _printAllBounds A flag that indicates whether to print also the bounds of each variable (=true).
                 */
                void print( std::ostream& _out = std::cout, const std::string _init = "", bool _printAllBounds = false ) const;

                /**
                 * @return true, if there is a conflicting variable;
                 *          false, otherwise.
                 */
                bool isConflicting() const
                {
                    return mpConflictingVariable != NULL;
                }

                /**
                 * @return The origins which cause the conflict. This method can only be called, if there is a conflict.
                 */
                std::set< const T* > getConflict() const
                {
                    assert( isConflicting() );
                    assert( !mpConflictingVariable->infimum().isInfinite() && !mpConflictingVariable->supremum().isInfinite() );
                    std::set< const T* > conflict = std::set< const T* >();
                    conflict.insert( *mpConflictingVariable->infimum().origins().begin() );
                    conflict.insert( *mpConflictingVariable->supremum().origins().begin() );
                    return conflict;
                }
        };
        
        template<class T>
        Bound<T>::Bound( Rational* const _limit, Variable<T>* const _variable, Type _type ):
            mType( _type ),
            mpLimit( _limit ),
            mpVariable( _variable ),
            mpOrigins( new std::set< const T* >() )
        {
            if( _limit == NULL )
                mpOrigins->insert( NULL );
        }

        template<class T>
        Bound<T>::~Bound()
        {
            delete mpOrigins;
            delete mpLimit;
        }

        template<class T>
        bool Bound<T>::operator<( const Bound<T>& _bound ) const
        {
            if( isUpperBound() && _bound.isUpperBound() )
            {
                if( isInfinite() )
                    return false;
                else
                {
                    if( _bound.isInfinite() )
                        return true;
                    else
                    {
                        if( *mpLimit < _bound.limit() )
                            return true;
                        else if( *mpLimit == _bound.limit() )
                        {
                            if( mType == STRICT_UPPER_BOUND )
                                return (_bound.type() != STRICT_UPPER_BOUND);
                            else if( mType == EQUAL_BOUND )
                                return (_bound.type() == WEAK_UPPER_BOUND);
                        }
                        return false;
                    }
                }
            }
            else
            {
                assert( isLowerBound() && _bound.isLowerBound() );
                if( _bound.isInfinite() )
                    return false;
                else
                {
                    if( isInfinite() )
                        return true;
                    else
                    {
                        if( *mpLimit < _bound.limit() )
                            return true;
                        else if( *mpLimit == _bound.limit() )
                        {
                            if( mType == WEAK_LOWER_BOUND )
                                return (_bound.type() != WEAK_LOWER_BOUND);
                            else if( mType == EQUAL_BOUND )
                                return (_bound.type() == STRICT_LOWER_BOUND);
                        }
                        return false;
                    }
                }
            }
        }
        
        template<class T>
        std::ostream& operator<<( std::ostream& _out, const Bound<T>& _bound )
        {
            if( _bound.isInfinite() )
            {
                if( _bound.type() == Bound<T>::STRICT_LOWER_BOUND )
                    _out << "-inf";
                else
                    _out << "inf";
            }
            else
                _out << _bound.limit();
            return _out;
        }

        template<class T>
        void Bound<T>::print( std::ostream& _out, bool _withRelation ) const
        {
            if( _withRelation )
            {
                switch( mType )
                {
                    case STRICT_LOWER_BOUND:
                        _out << ">";
                        break;
                    case WEAK_LOWER_BOUND:
                        _out << ">=";
                        break;
                    case EQUAL_BOUND:
                        _out << "=";
                        break;
                    case WEAK_UPPER_BOUND:
                        _out << "<=";
                        break;
                    case STRICT_UPPER_BOUND:
                        _out << "<";
                        break;
                    default:
                        assert( false );
                        _out << "~";
                        break;
                }       
            }
            _out << *this;
        }

        template<class T>
        Variable<T>::Variable():
            mUpdatedExactInterval( true ),
            mUpdatedDoubleInterval( true ),
            mUpperbounds( Variable<T>::BoundSet() ),
            mLowerbounds( Variable<T>::BoundSet() )
        {
            mUpperbounds.insert( new Bound<T>( NULL, this, Bound<T>::STRICT_UPPER_BOUND ) );
            mpSupremum = *mUpperbounds.begin();
            mLowerbounds.insert( new Bound<T>( NULL, this, Bound<T>::STRICT_LOWER_BOUND ) );
            mpInfimum = *mLowerbounds.begin();
        }

        template<class T>
        Variable<T>::~Variable()
        {
            while( !mLowerbounds.empty() )
            {
                const Bound<T>* toDelete = *mLowerbounds.begin();
                mLowerbounds.erase( mLowerbounds.begin() );
                if( toDelete->type() != Bound<T>::EQUAL_BOUND )
                    delete toDelete;
            }
            while( !mUpperbounds.empty() )
            {
                const Bound<T>* toDelete = *mUpperbounds.begin();
                mUpperbounds.erase( mUpperbounds.begin() );
                delete toDelete;
            }
        }
        
        template<class T>
        bool Variable<T>::conflicting() const
        {
            if( mpSupremum->isInfinite() || mpInfimum->isInfinite() )
                return false;
            else
            {
                if( mpSupremum->limit() < mpInfimum->limit() )
                    return true;
                else if( mpInfimum->limit() == mpSupremum->limit() )
                    return ( mpInfimum->type() == Bound<T>::STRICT_LOWER_BOUND || mpSupremum->type() == Bound<T>::STRICT_UPPER_BOUND );
                return false;
            }
        }

        template<class T>
        const Bound<T>* Variable<T>::addBound( const Constraint* _constraint, const carl::Variable& _var, const T* _origin )
        {
            assert( _constraint->variables().size() == 1 && _constraint->maxDegree( _var ) == 1 );
            const Rational& coeff = _constraint->lhs().lterm()->coeff();
            Constraint::Relation rel = _constraint->relation();
            Rational* limit = new Rational( -_constraint->constantPart()/coeff );
            std::pair< class Variable<T>::BoundSet::iterator, bool> result;
            if( rel == Constraint::EQ )
            {
                Bound<T>* newBound = new Bound<T>( limit, this, Bound<T>::EQUAL_BOUND );
                result = mUpperbounds.insert( newBound );
                if( !result.second )
                    delete newBound;
                else
                    mLowerbounds.insert( newBound );
            }
            else if( ( rel == Constraint::GEQ && coeff < 0 ) || ( rel == Constraint::LEQ && coeff > 0 ) )
            {
                Bound<T>* newBound = new Bound<T>( limit, this, Bound<T>::WEAK_UPPER_BOUND );
                result = mUpperbounds.insert( newBound );
                if( !result.second )
                    delete newBound;
            }
            else if( ( rel == Constraint::LEQ && coeff < 0 ) || ( rel == Constraint::GEQ && coeff > 0 ) )
            {
                Bound<T>* newBound = new Bound<T>( limit, this, Bound<T>::WEAK_LOWER_BOUND );
                result = mLowerbounds.insert( newBound );
                if( !result.second )
                    delete newBound;
            }
            else if( ( rel == Constraint::GREATER && coeff < 0 ) || ( rel == Constraint::LESS && coeff > 0 ) )
            {
                Bound<T>* newBound = new Bound<T>( limit, this, Bound<T>::STRICT_UPPER_BOUND );
                result = mUpperbounds.insert( newBound );
                if( !result.second )
                    delete newBound;
            }
            else if( ( rel == Constraint::LESS && coeff < 0 ) || ( rel == Constraint::GREATER && coeff > 0 ) )
            {
                Bound<T>* newBound = new Bound<T>( limit, this, Bound<T>::STRICT_LOWER_BOUND );
                result = mLowerbounds.insert( newBound );
                if( !result.second )
                    delete newBound;
            }
            else
                assert( false );
            (*result.first)->activate( _origin );
            return *result.first;
        }

        template<class T>
        bool Variable<T>::updateBounds( const Bound<T>& _changedBound )
        {
            mUpdatedExactInterval = true;
            mUpdatedDoubleInterval = true;
            if( _changedBound.isUpperBound() )
            {
                // Update the supremum.
                class Variable<T>::BoundSet::iterator newBound = mUpperbounds.begin();
                while( newBound != mUpperbounds.end() )
                {
                    if( (*newBound)->isActive() )
                    {
                        mpSupremum = *newBound;
                        break;
                    }
                    ++newBound;
                }
            }
            if( _changedBound.isLowerBound() )
            {
                // Update the infimum.
                class Variable<T>::BoundSet::reverse_iterator newBound = mLowerbounds.rbegin();
                while( newBound != mLowerbounds.rend() )
                {
                    if( (*newBound)->isActive() )
                    {
                        mpInfimum = *newBound;
                        break;
                    }
                    ++newBound;
                }
            }
            return conflicting();
        }

        template<class T>
        VariableBounds<T>::VariableBounds():
            mpConflictingVariable( NULL ),
            mpVariableMap( new VariableMap() ),
            mpConstraintBoundMap( new ConstraintBoundMap() ),
            mEvalIntervalMap(),
            mDoubleIntervalMap()
        {}

        template<class T>
        VariableBounds<T>::~VariableBounds()
        {
            delete mpConstraintBoundMap;
            while( !mpVariableMap->empty() )
            {
                Variable<T>* toDelete = mpVariableMap->begin()->second;
                mpVariableMap->erase( mpVariableMap->begin() );
                delete toDelete;
            }
            delete mpVariableMap;
        }

        template<class T>
        bool VariableBounds<T>::addBound( const Constraint* _constraint, const T* _origin )
        {
            if( _constraint->relation() != Constraint::NEQ && _constraint->variables().size() == 1 )
            {
                const carl::Variable& var = *_constraint->variables().begin();
                if( _constraint->maxDegree( var ) == 1 )
                {
                    class VariableBounds<T>::ConstraintBoundMap::iterator cbPair = mpConstraintBoundMap->find( _constraint );
                    if( cbPair != mpConstraintBoundMap->end() )
                    {
                        const Bound<T>& bound = *cbPair->second;
                        if( bound.activate( _origin ) )
                        {
                            if( bound.pVariable()->updateBounds( bound ) )
                                mpConflictingVariable = bound.pVariable();
                        }
                    }
                    else
                    {
                        Variable<T>* variable;
                        const Bound<T>* bound;
                        class VariableBounds<T>::VariableMap::iterator evPair = mpVariableMap->find( var );
                        if( evPair != mpVariableMap->end() )
                        {
                            variable = evPair->second;
                            bound = variable->addBound( _constraint, evPair->first, _origin );
                        }
                        else
                        {
                            variable = new Variable<T>();
                            (*mpVariableMap)[var] = variable;
                            bound = variable->addBound( _constraint, var, _origin );
                        }
                        mpConstraintBoundMap->insert( std::pair< const Constraint*, const Bound<T>* >( _constraint, bound ) );
                        if( variable->updateBounds( *bound ) )
                            mpConflictingVariable = bound->pVariable();
                    }
                    return true;
                }
                else
                    mpVariableMap->insert( std::pair< const carl::Variable, Variable<T>* >( var, new Variable<T>() ) );
            }
            else
            {
                for( auto sym = _constraint->variables().begin(); sym !=  _constraint->variables().end(); ++sym )
                {
                    Variable<T>* variable = new Variable<T>();
                    if( !mpVariableMap->insert( std::pair< const carl::Variable, Variable<T>* >( *sym, variable ) ).second )
                        delete variable;
                }
            }
            return false;
        }

        template<class T>
        unsigned VariableBounds<T>::removeBound( const Constraint* _constraint, const T* _origin )
        {
            if( _constraint->relation() != Constraint::NEQ && _constraint->variables().size() == 1 )
            {
                const carl::Variable& var = *_constraint->variables().begin();
                if( _constraint->maxDegree( var ) == 1 )
                {
                    assert( mpConstraintBoundMap->find( _constraint ) != mpConstraintBoundMap->end() );
                    const Bound<T>& bound = *(*mpConstraintBoundMap)[_constraint];
                    if( bound.deactivate( _origin ) )
                    {
                        if( bound.pVariable()->updateBounds( bound ) )
                            mpConflictingVariable = bound.pVariable();
                        else
                            mpConflictingVariable = NULL;
                        return 2;
                    }
                    return 1;
                }
            }
            return 0;
        }

        template<class T>
        unsigned VariableBounds<T>::removeBound( const Constraint* _constraint, const T* _origin, carl::Variable*& _changedVariable )
        {
            if( _constraint->relation() != Constraint::NEQ && _constraint->variables().size() == 1 )
            {
                const carl::Variable& var = *_constraint->variables().begin();
                if( _constraint->maxDegree( var ) == 1 )
                {
                    assert( mpConstraintBoundMap->find( _constraint ) != mpConstraintBoundMap->end() );
                    const Bound<T>& bound = *(*mpConstraintBoundMap)[_constraint];
                    if( bound.deactivate( _origin ) )
                    {
                        if( bound.pVariable()->updateBounds( bound ) )
                            mpConflictingVariable = bound.pVariable();
                        else
                            mpConflictingVariable = NULL;
                        _changedVariable = new carl::Variable( var );
                        return 2;
                    }
                    return 1;
                }
            }
            return 0;
        }

        #define CONVERT_BOUND(type, namesp) (type != Bound<T>::WEAK_UPPER_BOUND && type != Bound<T>::WEAK_LOWER_BOUND && type != Bound<T>::EQUAL_BOUND ) ? namesp::STRICT : namesp::WEAK

        template<class T>
        const smtrat::EvalIntervalMap& VariableBounds<T>::getEvalIntervalMap()
        {
            assert( mpConflictingVariable == NULL );
            for( auto varVarPair = mpVariableMap->begin(); varVarPair != mpVariableMap->end(); ++varVarPair )
            {
                Variable<T>& var = *varVarPair->second;
                if( var.updatedExactInterval() )
                {
                    carl::BoundType lowerBoundType;
                    Rational lowerBoundValue;
                    carl::BoundType upperBoundType;
                    Rational upperBoundValue;
                    if( var.infimum().isInfinite() )
                    {
                        lowerBoundType = carl::BoundType::INFTY;
                        lowerBoundValue = 0;
                    }
                    else
                    {
                        lowerBoundType = CONVERT_BOUND( var.infimum().type(), carl::BoundType );
                        lowerBoundValue = var.infimum().limit();
                    }
                    if( var.supremum().isInfinite() )
                    {
                        upperBoundType = carl::BoundType::INFTY;
                        upperBoundValue = 0;
                    }
                    else
                    {
                        upperBoundType = CONVERT_BOUND( var.supremum().type(), carl::BoundType );
                        upperBoundValue = var.supremum().limit();
                    }
                    mEvalIntervalMap[varVarPair->first] = Interval( lowerBoundValue, lowerBoundType, upperBoundValue, upperBoundType );
                    var.exactIntervalHasBeenUpdated();
                }
            }
            return mEvalIntervalMap;
        }

        template<class T>
        const Interval& VariableBounds<T>::getInterval( const carl::Variable& _var )
        {
            assert( mpConflictingVariable == NULL );
            class VariableMap::iterator varVarPair = mpVariableMap->find( _var );
            assert( varVarPair != mpVariableMap->end() );
            Variable<T>& var = *varVarPair->second;
            if( var.updatedExactInterval() )
            {
                carl::BoundType lowerBoundType;
                Rational lowerBoundValue;
                carl::BoundType upperBoundType;
                Rational upperBoundValue;
                if( var.infimum().isInfinite() )
                {
                    lowerBoundType = carl::BoundType::INFTY;
                    lowerBoundValue = 0;
                }
                else
                {
                    lowerBoundType = CONVERT_BOUND( var.infimum().type(), carl::BoundType );
                    lowerBoundValue = var.infimum().limit();
                }
                if( var.supremum().isInfinite() )
                {
                    upperBoundType = carl::BoundType::INFTY;
                    upperBoundValue = 0;
                }
                else
                {
                    upperBoundType = CONVERT_BOUND( var.supremum().type(), carl::BoundType );
                    upperBoundValue = var.supremum().limit();
                }
                mEvalIntervalMap[_var] = Interval( lowerBoundValue, lowerBoundType, upperBoundValue, upperBoundType );
                var.exactIntervalHasBeenUpdated();
            }
            return mEvalIntervalMap[_var];
        }

        template<class T>
        const smtrat::EvalDoubleIntervalMap& VariableBounds<T>::getIntervalMap()
        {
            assert( mpConflictingVariable == NULL );
            for( auto varVarPair = mpVariableMap->begin(); varVarPair != mpVariableMap->end(); ++varVarPair )
            {
                Variable<T>& var = *varVarPair->second;
                if( var.updatedDoubleInterval() )
                {
                    carl::BoundType lowerBoundType;
                    Rational lowerBoundValue;
                    carl::BoundType upperBoundType;
                    Rational upperBoundValue;
                    if( var.infimum().isInfinite() )
                    {
                        lowerBoundType = carl::BoundType::INFTY;
                        lowerBoundValue = 0;
                    }
                    else
                    {
                        lowerBoundType = CONVERT_BOUND( var.infimum().type(), carl::BoundType );
                        lowerBoundValue = var.infimum().limit();
                    }
                    if( var.supremum().isInfinite() )
                    {
                        upperBoundType = carl::BoundType::INFTY;
                        upperBoundValue = 0;
                    }
                    else
                    {
                        upperBoundType = CONVERT_BOUND( var.supremum().type(), carl::BoundType );
                        upperBoundValue = var.supremum().limit();
                    }
                    mDoubleIntervalMap[varVarPair->first] = carl::DoubleInterval( lowerBoundValue, lowerBoundType, upperBoundValue, upperBoundType );
                    var.doubleIntervalHasBeenUpdated();
                }
            }
            return mDoubleIntervalMap;
        }

        template<class T>
        const carl::DoubleInterval& VariableBounds<T>::getDoubleInterval( const carl::Variable& _var )
        {
            assert( mpConflictingVariable == NULL );
            class VariableMap::iterator varVarPair = mpVariableMap->find( _var );
            assert( varVarPair != mpVariableMap->end() );
            Variable<T>& var = *varVarPair->second;
            if( var.updatedDoubleInterval() )
            {
                carl::BoundType lowerBoundType;
                Rational lowerBoundValue;
                carl::BoundType upperBoundType;
                Rational upperBoundValue;
                if( var.infimum().isInfinite() )
                {
                    lowerBoundType = carl::BoundType::INFTY;
                    lowerBoundValue = 0;
                }
                else
                {
                    lowerBoundType = CONVERT_BOUND( var.infimum().type(), carl::BoundType );
                    lowerBoundValue = var.infimum().limit();
                }
                if( var.supremum().isInfinite() )
                {
                    upperBoundType = carl::BoundType::INFTY;
                    upperBoundValue = 0;
                }
                else
                {
                    upperBoundType = CONVERT_BOUND( var.supremum().type(), carl::BoundType );
                    upperBoundValue = var.supremum().limit();
                }
                mDoubleIntervalMap[_var] = carl::DoubleInterval( lowerBoundValue, lowerBoundType, upperBoundValue, upperBoundType );
                var.doubleIntervalHasBeenUpdated();
            }
            return mDoubleIntervalMap[_var];
        }

        template<class T>
        std::set< const T* > VariableBounds<T>::getOriginsOfBounds( const carl::Variable& _var ) const
        {
            std::set< const T* > originsOfBounds = std::set< const T* >();
            auto varVarPair = mpVariableMap->find( _var );
            assert( varVarPair != mpVariableMap->end() );
            if( !varVarPair->second->infimum().isInfinite() ) originsOfBounds.insert( *varVarPair->second->infimum().origins().begin() );
            if( !varVarPair->second->supremum().isInfinite() ) originsOfBounds.insert( *varVarPair->second->supremum().origins().begin() );
            return originsOfBounds;
        }

        template<class T>
        std::set< const T* > VariableBounds<T>::getOriginsOfBounds( const Variables& _variables ) const
        {
            std::set< const T* > originsOfBounds = std::set< const T* >();
            for( auto var = _variables.begin(); var != _variables.end(); ++var )
            {
                auto varVarPair = mpVariableMap->find( *var );
                assert( varVarPair != mpVariableMap->end() );
                if( !varVarPair->second->infimum().isInfinite() ) originsOfBounds.insert( *varVarPair->second->infimum().origins().begin() );
                if( !varVarPair->second->supremum().isInfinite() ) originsOfBounds.insert( *varVarPair->second->supremum().origins().begin() );
            }
            return originsOfBounds;
        }

        template<class T>
        std::set< const T* > VariableBounds<T>::getOriginsOfBounds() const
        {
            std::set< const T* > originsOfBounds = std::set< const T* >();
            for( auto varVarPair = mpVariableMap->begin(); varVarPair != mpVariableMap->end(); ++varVarPair )
            {
                const Variable<T>& var = *varVarPair->second;
                if( !var.infimum().isInfinite() ) originsOfBounds.insert( *var.infimum().origins().begin() );
                if( !var.supremum().isInfinite() ) originsOfBounds.insert( *var.supremum().origins().begin() );
            }
            return originsOfBounds;
        }

        template<class T>
        void VariableBounds<T>::print( std::ostream& _out, const std::string _init, bool _printAllBounds ) const
        {
            for( auto varVarPair = mpVariableMap->begin(); varVarPair != mpVariableMap->end(); ++varVarPair )
            {
                _out << _init;
                std::stringstream outA;
                outA << varVarPair->first;
                _out << std::setw( 15 ) << outA.str();
                _out << "  in  ";
                if( varVarPair->second->infimum().type() == Bound<T>::STRICT_LOWER_BOUND )
                    _out << "] ";
                else
                    _out << "[ ";
                std::stringstream outB;
                outB << varVarPair->second->infimum();
                _out << std::setw( 12 ) << outB.str();
                _out << ", ";
                std::stringstream outC;
                outC << varVarPair->second->supremum();
                _out << std::setw( 12 ) << outC.str();
                if( varVarPair->second->supremum().type() == Bound<T>::STRICT_UPPER_BOUND )
                    _out << " [";
                else
                    _out << " ]";
                _out << std::endl;
                if( _printAllBounds )
                {
                    _out << _init << "         Upper bounds:" << std::endl;
                    for( auto _uBound = varVarPair->second->upperbounds().begin(); _uBound != varVarPair->second->upperbounds().end(); ++_uBound )
                    {
                        _out << _init << "            ";
                        (*_uBound)->print( _out, true );
                        _out << "   {";
                        for( auto origin = (*_uBound)->origins().begin(); origin != (*_uBound)->origins().end(); ++origin )
                            _out << " " << *origin;
                        _out << " }" << std::endl;
                    }
                    _out << _init << "         Lower bounds:" << std::endl;
                    for( auto _lBound = varVarPair->second->lowerbounds().rbegin(); _lBound != varVarPair->second->lowerbounds().rend(); ++_lBound )
                    {
                        _out << _init << "            ";
                        (*_lBound)->print( _out, true );
                        _out << "    {";
                        for( auto origin = (*_lBound)->origins().begin(); origin != (*_lBound)->origins().end(); ++origin )
                            _out << " " << *origin;
                        _out << " }" << std::endl;
                    }
                }
            }
        }
    }   // namespace vb
}    // namespace smtrat

#endif	/* VARIABLEBOUNDS_H */
