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
 * @file ConstraintPool.h
 *
 * @author Florian Corzilius
 * @author Sebastian Junges
 * @author Ulrich Loup
 * @version 2013-10-21
 */
#include "Constraint.h"
#include "datastructures/vs/SqrtEx.h"
#include <mutex>

#ifndef CONSTRAINTPOOL_H
#define CONSTRAINTPOOL_H

namespace smtrat
{
    class ConstraintPool : public carl::Singleton<ConstraintPool>
    {
        friend carl::Singleton<ConstraintPool>;
        private:
            // Members:

            /// A flag indicating whether the prefix of the internally created external variable names has already been initialized.
            bool mExternalPrefixInitialized;
            ///
            bool mLastConstructedConstraintWasKnown;
            /// id allocator
            unsigned mIdAllocator;
            /// A counter for the auxiliary Boolean valued variables.
            unsigned mAuxiliaryBoolVarCounter;
            /// A counter for the auxiliary real valued variables.
            unsigned mAuxiliaryRealVarCounter;
            /// A counter for the auxiliary integer valued variables.
            unsigned mAuxiliaryIntVarCounter;
            /// The constraint (0=0) representing a valid constraint.
            const Constraint* mConsistentConstraint;
            /// The constraint (0>0) representing an inconsistent constraint.
            const Constraint* mInconsistentConstraint;
            /// Mutex to avoid multiple access to the map of arithmetic variables
            mutable std::mutex mMutexArithmeticVariables;
            /// Mutex to avoid multiple access to the set of Boolean variables
            mutable std::mutex mMutexBooleanVariables;
            /// Mutex to avoid multiple access to the pool
            mutable std::mutex mMutexPool;
            /// The external prefix for a variable.
            std::string mExternalVarNamePrefix;
            /// The map of external variable names to internal variable names.
            std::map<std::string,carl::Variable> mExternalNamesToVariables;
            /// The collection of Boolean variables in use.
            Variables mBooleanVariables;
            /// The constraint pool.
            FastPointerSet<Constraint> mConstraints;
            /// All external variable names which have been created during parsing.
            std::vector<std::string> mParsedVarNames;
            
            #ifdef SMTRAT_STRAT_PARALLEL_MODE
            #define CONSTRAINT_POOL_LOCK_GUARD std::lock_guard<std::mutex> lock1( mMutexPool );
            #define CONSTRAINT_POOL_LOCK mMutexPool.lock();
            #define CONSTRAINT_POOL_UNLOCK mMutexPool.unlock();
            #define ARITHMETIC_VAR_LOCK_GUARD std::lock_guard<std::mutex> lock2( mMutexArithmeticVariables );
            #define ARITHMETIC_VAR_LOCK mMutexArithmeticVariables.lock();
            #define ARITHMETIC_VAR_UNLOCK mMutexArithmeticVariables.unlock();
            #define BOOLEAN_VAR_LOCK_GUARD std::lock_guard<std::mutex> lock3( mMutexBooleanVariables );
            #define BOOLEAN_VAR_LOCK mMutexBooleanVariables.lock();
            #define BOOLEAN_VAR_UNLOCK mMutexBooleanVariables.unlock();
            #else
            #define CONSTRAINT_POOL_LOCK_GUARD
            #define CONSTRAINT_POOL_LOCK
            #define CONSTRAINT_POOL_UNLOCK
            #define ARITHMETIC_VAR_LOCK_GUARD
            #define ARITHMETIC_VAR_LOCK
            #define ARITHMETIC_VAR_UNLOCK
            #define BOOLEAN_VAR_LOCK_GUARD
            #define BOOLEAN_VAR_LOCK
            #define BOOLEAN_VAR_UNLOCK
            #endif
            
            /**
             * Creates a normalized constraint, which has the same solutions as the constraint consisting of the given
             * left-hand side and relation symbol.
             * Note, that this method uses the allocator which is locked before calling.
             * @param _lhs The left-hand side of the constraint before normalization,
             * @param _rel The relation symbol of the constraint before normalization,
             * @param _variables An over-approximation of the variables occurring in the given left-hand side.
             * @return The constructed constraint.
             */
            Constraint* createNormalizedBound( const carl::Variable& _var, const Relation _rel, const Rational& _bound ) const;
            
            /**
             * Creates a normalized constraint, which has the same solutions as the constraint consisting of the given
             * left-hand side and relation symbol.
             * Note, that this method uses the allocator which is locked before calling.
             * @param _lhs The left-hand side of the constraint before normalization,
             * @param _rel The relation symbol of the constraint before normalization,
             * @param _variables An over-approximation of the variables occurring in the given left-hand side.
             * @return The constructed constraint.
             */
            Constraint* createNormalizedConstraint( const Polynomial& _lhs, const Relation _rel ) const;
            
            /**
             * Adds the given constraint to the pool, if it does not yet occur in there.
             * Note, that this method uses the allocator which is locked before calling.
             * @sideeffect The given constraint will be deleted, if it already occurs in the pool.
             * @param _constraint The constraint to add to the pool.
             * @return The given constraint, if it did not yet occur in the pool;
             *          The equivalent constraint already occurring in the pool.
             */
            const Constraint* addConstraintToPool( Constraint* _constraint );

        protected:
            
            /**
             * Constructor of the constraint pool.
             * @param _capacity Expected necessary capacity of the pool.
             */
            ConstraintPool( unsigned _capacity = 10000 );

        public:

            /**
             * Destructor.
             */
            ~ConstraintPool();

            /**
             * @return An iterator to the first constraint in this pool.
             */
            FastPointerSet<Constraint>::const_iterator begin() const
            {
                // TODO: Will begin() be valid if we insert elements?
                CONSTRAINT_POOL_LOCK_GUARD
                auto result = mConstraints.begin();
                return result;
            }

            /**
             * @return An iterator to the end of the container of the constraints in this pool.
             */
            FastPointerSet<Constraint>::const_iterator end() const
            {
                // TODO: Will end() be changed if we insert elements?
                CONSTRAINT_POOL_LOCK_GUARD
                auto result = mConstraints.end();
                return result;
            }

            /**
             * @return The number of constraint in this pool.
             */
            size_t size() const
            {
                CONSTRAINT_POOL_LOCK_GUARD
                size_t result = mConstraints.size();
                return result;
            }
            
            /*
             */
            bool lastConstructedConstraintWasKnown() const
            {
                return mLastConstructedConstraintWasKnown;
            }

            /**
             * Returns all constructed Boolean variables. Note, that it does not
             * return the reference to the member, but a copy of it instead. This is
             * due to mutual exclusion and an expensive operation which should only
             * used for debugging or outputting purposes.
             * @return All constructed Boolean variables.
             */
            Variables booleanVariables() const
            {
                return mBooleanVariables;
            }
            
            /**
             * Returns all constructed arithmetic variables. This method constructs a new
             * container of the demanded variables due to mutual exclusion which forms an
             * expensive operation and should only used for debugging or outputting purposes.
             * @return All constructed arithmetic variables.
             */
            Variables arithmeticVariables() const
            {
                Variables result = Variables();
                for( auto nameVarPair = mExternalNamesToVariables.begin(); nameVarPair != mExternalNamesToVariables.end(); ++nameVarPair )
                {
                    result.insert( nameVarPair->second );
                }
                return result;
            }
            
            /**
             * @return A pointer to the constraint which represents any constraint for which it is easy to 
             *          decide, whether it is consistent, e.g. 0=0, -1!=0, x^2+1>0
             */
            const Constraint* consistentConstraint() const
            {
                return mConsistentConstraint;
            }
                        
            /**
             * @return A pointer to the constraint which represents any constraint for which it is easy to 
             *          decide, whether it is consistent, e.g. 1=0, 0!=0, x^2+1=0
            */
            const Constraint* inconsistentConstraint() const
            {
                return mInconsistentConstraint;
            }
            
            /**
             * @return The string being the prefix of the external name of any internally declared (not parsed) variable.
             */
            std::string externalVarNamePrefix() const
            {
                return mExternalVarNamePrefix;
            }
    
            /**
             * @param _var The variable to get the name for.
             * @param _friendlyName A flag that indicates whether to print the given variables name with its 
             *                       internal representation (false) or with its dedicated name.
             * @return The name of the given variable.
             */
            std::string getVariableName( const carl::Variable& _var, bool _friendlyName = true ) const
            {
                return carl::VariablePool::getInstance().getName( _var, _friendlyName );
            }
            
            /**
             * Gets the variable by its name. Note that this is expensive and should only be used
             * for outputting reasons. In the actual implementations you should store the variables instead.
             * @param _varName The name of the variable to search for.
             * @return The found variable.
             */
            carl::Variable getArithmeticVariableByName( const std::string& _varName, bool _byFriendlyName = false ) const
            {
                for( auto nameVarPair = mExternalNamesToVariables.begin(); nameVarPair != mExternalNamesToVariables.end(); ++nameVarPair )
                {
                    if( carl::VariablePool::getInstance().getName( nameVarPair->second, _byFriendlyName ) == _varName )
                        return nameVarPair->second;
                }
                assert( false );
                return mExternalNamesToVariables.begin()->second;
            }

            /**
             * Note: This method makes the other accesses to the constraint pool waiting.
             * @return The highest degree occurring in all constraints
             */
            carl::exponent maxDegree() const
            {
                carl::exponent result = 0;
                CONSTRAINT_POOL_LOCK_GUARD
                for( auto constraint = mConstraints.begin(); constraint != mConstraints.end(); ++constraint )
                {
                    carl::exponent maxdeg = (*constraint)->lhs().totalDegree();
                    if(maxdeg > result) 
                        result = maxdeg;
                }
                return result;
            }
            
            /**
             * Note: This method makes the other accesses to the constraint pool waiting.
             * @return The number of non-linear constraints in the pool.
             */
            unsigned nrNonLinearConstraints() const
            {
                unsigned nonlinear = 0;
                CONSTRAINT_POOL_LOCK_GUARD
                for( auto constraint = mConstraints.begin(); constraint != mConstraints.end(); ++constraint )
                {
                    if( !(*constraint)->lhs().isLinear() ) 
                        ++nonlinear;
                }
                return nonlinear;
            }
            
            /**
             * @return The number of Boolean variables which have been generated.
             */
            size_t numberOfBooleanVariables() const
            {
                return mBooleanVariables.size();
            }
            
            
            /**
             * @return The number of real variables which have been generated.
             */
            unsigned numberOfRealVariables() const
            {
                unsigned result = 0;
                for( auto var = mExternalNamesToVariables.begin(); var != mExternalNamesToVariables.end(); ++var )
                    if( var->second.getType() == carl::VariableType::VT_REAL )
                        ++result;
                return result;
            }
            
            /**
             * @return The number of integer variables which have been generated.
             */
            unsigned numberOfIntVariables() const
            {
                unsigned result = 0;
                for( auto var = mExternalNamesToVariables.begin(); var != mExternalNamesToVariables.end(); ++var )
                    if( var->second.getType() == carl::VariableType::VT_INT )
                        ++result;
                return result;
            }
               
            /**
             * @param _varName The Boolean variable name to check.
             * @return true, if the given Boolean variable name already exists. 
             */
            bool booleanExistsAlready( const std::string& _booleanName ) const
            {
                for( auto iter = mBooleanVariables.begin(); iter != mBooleanVariables.end(); ++iter )
                    if( _booleanName == carl::VariablePool::getInstance().getName( *iter, true ) ) return true;
                return false;
            }
            
            /**
             * Creates an auxiliary integer valued variable.
             * @param _externalPrefix The prefix of the external name of the auxiliary variable to construct.
             * @return A pair of the internal name of the variable and the a variable as an expression.
             */
            carl::Variable newAuxiliaryIntVariable( const std::string& _externalPrefix = "h_i" )
            {
                std::stringstream out;
                if( !mExternalPrefixInitialized ) initExternalPrefix();
                out << mExternalVarNamePrefix << _externalPrefix << mAuxiliaryIntVarCounter++;
                return newArithmeticVariable( out.str(), carl::VariableType::VT_INT );
            }
            
            /**
             * Creates an auxiliary real valued variable.
             * @param _externalPrefix The prefix of the external name of the auxiliary variable to construct.
             * @return A pair of the internal name of the variable and the a variable as an expression.
             */
            carl::Variable newAuxiliaryRealVariable( const std::string& _externalPrefix = "h_r" )
            {
                std::stringstream out;
                if( !mExternalPrefixInitialized ) initExternalPrefix();
                out << mExternalVarNamePrefix << _externalPrefix << "_" << mAuxiliaryRealVarCounter++;
                return newArithmeticVariable( out.str(), carl::VariableType::VT_REAL );
            }
            
            /**
             * Resets the constraint pool.
             * Note: Do not use it. It is only made for the Benchmax-Tool.
             */
            void clear();
            
            /**
             * Constructs a new constraint and adds it to the pool, if it is not yet a member. If it is a
             * member, this will be returned instead of a new constraint.
             * Note, that the left-hand side of the constraint is simplified and normalized, hence it is
             * not necessarily equal to the given left-hand side. The same holds for the relation symbol.
             * However, it is assured that the returned constraint has the same solutions as
             * the expected one.
             * @param _lhs The left-hand side of the constraint.
             * @param _rel The relation symbol of the constraint.
             * @param _variables An over-approximation of the variables which occur on the left-hand side.
             * @return The constructed constraint.
             */
            const Constraint* newBound( const carl::Variable& _var, const Relation _rel, const Rational& _bound );
            
            /**
             * Constructs a new constraint and adds it to the pool, if it is not yet a member. If it is a
             * member, this will be returned instead of a new constraint.
             * Note, that the left-hand side of the constraint is simplified and normalized, hence it is
             * not necessarily equal to the given left-hand side. The same holds for the relation symbol.
             * However, it is assured that the returned constraint has the same solutions as
             * the expected one.
             * @param _lhs The left-hand side of the constraint.
             * @param _rel The relation symbol of the constraint.
             * @param _variables An over-approximation of the variables which occur on the left-hand side.
             * @return The constructed constraint.
             */
            const Constraint* newConstraint( const Polynomial& _lhs, const Relation _rel );
            
            /**
             * Creates an arithmetic variable.
             * @param _name The external name of the variable to construct.
             * @param _domain The domain of the variable to construct.
             * @param _parsed A special flag indicating whether this variable is constructed during parsing.
             * @return A pair of the internal name of the variable and the variable as an expression.
             */
            carl::Variable newArithmeticVariable( const std::string& _name, carl::VariableType _domain, bool _parsed = false );
            
            /**
             * Creates a new Boolean variable.
             * @param _name The external name of the variable to construct.
             * @param _parsed A special flag indicating whether this variable is constructed during parsing.
             */
            const carl::Variable newBooleanVariable( const std::string& _name, bool _parsed = false );
            
            /**
             * Creates an auxiliary Boolean variable.
             * @param _externalPrefix The prefix of the external name of the auxiliary variable to construct.
             * @return The internal name of the variable.
             */
            const carl::Variable newAuxiliaryBooleanVariable( const std::string& _externalPrefix = "h_b" );
            
            /**
             * Initializes the prefix of the external variable names of internally declared (not parsed) variables.
             */
            void initExternalPrefix();
            
            /**
             * Prints all constraints in the constraint pool on the given stream.
             *
             * @param _out The stream to print on.
             */
            void print( std::ostream& _out = std::cout ) const;
    };
    
    /**
      * Constructs a new constraint and adds it to the shared constraint pool, if it is not yet a member. If it is a
      * member, this will be returned instead of a new constraint.
      * Note, that the left-hand side of the constraint is simplified and normalized, hence it is
      * not necessarily equal to the given left-hand side. The same holds for the relation symbol.
      * However, it is assured that the returned constraint has the same solutions as
      * the expected one.
      * @param _lhs The left-hand side of the constraint.
      * @param _rel The relation symbol of the constraint.
      * @param _variables An over-approximation of the variables which occur on the left-hand side.
      * @return The constructed constraint.
      */
     const Constraint* newBound( const carl::Variable& _var, const Relation _rel, const Rational& _bound );

     /**
      * Constructs a new constraint and adds it to the shared constraint pool, if it is not yet a member. If it is a
      * member, this will be returned instead of a new constraint.
      * Note, that the left-hand side of the constraint is simplified and normalized, hence it is
      * not necessarily equal to the given left-hand side. The same holds for the relation symbol.
      * However, it is assured that the returned constraint has the same solutions as
      * the expected one.
      * @param _lhs The left-hand side of the constraint.
      * @param _rel The relation symbol of the constraint.
      * @param _variables An over-approximation of the variables which occur on the left-hand side.
      * @return The constructed constraint.
      */
     const Constraint* newConstraint( const Polynomial& _lhs, const Relation _rel );

     /**
      * Constructs a new real variable.
      * @param _name The intended name of the real variable.
      * @return The constructed real variable.
      */
     carl::Variable newRealVariable( const std::string& _name );

     /**
      * Constructs a new arithmetic variable of the given domain.
      * @param _name The intended name of the arithmetic variable.
      * @param _domain The domain of the arithmetic variable.
      * @return The constructed arithmetic variable.
      */
     carl::Variable newArithmeticVariable( const std::string& _name, carl::VariableType _domain, bool _parsed = false );

     /**
      * Constructs a new Boolean variable.
      * @param _name The intended name of the variable.
      * @return A pointer to the name of the constructed Boolean variable.
      */
     const carl::Variable newBooleanVariable( const std::string& _name, bool _parsed = false );

     /**
      * @return A constant reference to the shared constraint pool.
      */
     const ConstraintPool& constraintPool();

     /**
      * Generates a fresh real variable and returns its identifier.
      * @return The fresh real variable.
      */
     carl::Variable newAuxiliaryIntVariable();

     /**
      * Generates a fresh real variable and returns its identifier.
      * @param _varName The dedicated name of the real variable.
      * @return The fresh real variable.
      */
     carl::Variable newAuxiliaryIntVariable( const std::string& _varName );

     /**
      * Generates a fresh real variable and returns its identifier.
      * @return The fresh real variable.
      */
     carl::Variable newAuxiliaryRealVariable();

     /**
      * Generates a fresh real variable and returns its identifier.
      * @param _varName The dedicated name of the real variable.
      * @return The fresh real variable.
      */
     carl::Variable newAuxiliaryRealVariable( const std::string& _varName );

     /**
      * Generates a fresh Boolean variable and returns its identifier.
      * @return The identifier of a fresh Boolean variable.
      */
     const carl::Variable newAuxiliaryBooleanVariable();
    
}    // namespace smtrat

#endif   /* CONSTRAINTPOOL_H */
