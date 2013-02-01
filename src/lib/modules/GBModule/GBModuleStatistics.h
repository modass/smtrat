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


#ifndef GBMODULESTATISTICS_H
#define	GBMODULESTATISTICS_H

#include "../../config.h"
#ifdef SMTRAT_DEVOPTION_Statistics
#include <vector>
#include <map>
#include <iostream>

#include "../../Constraint.h"
#include "../../utilities/stats/Statistics.h"

namespace smtrat {
class GroebnerModuleStats : public Statistics
{
   public:
     static GroebnerModuleStats* getInstance(unsigned key);
     
     static void printAll(std::ostream& = std::cout);
     
     /**
      * Override Statistics::collect
      */
     void collect() {
         Statistics::addKeyValuePair("Number calls", mNrCalls);
         Statistics::addKeyValuePair("Constant GB", mNrConstantGBs);
         Statistics::addKeyValuePair("Infeasible inequalities", mNrInfeasibleInequalities);
         Statistics::addKeyValuePair("Backend false", mNrBackendReturnsFalse);
         Statistics::addKeyValuePair("Deduced equalities", mNrDeducedEqualities);
         Statistics::addKeyValuePair("Deduced inequalities", mNrDeducedInequalities);
         Statistics::addKeyValuePair("Radical search: Found identity", mNrOfFoundIdentities);
         Statistics::addKeyValuePair("Radical search: Found equality", mNrOfFoundEqualities);
         
     }
     
     /**
      *  Count how often the module is called
      */
     void called() { mNrCalls++; }
     /**
      * Count how often we find a constant Gb
      */
     void constantGB() { mNrConstantGBs++; }
     /**
      * Count how often we find infeasibility in the inequalitiestable.
      */
     void infeasibleInequality() { mNrInfeasibleInequalities++;  }
     /**
      *  Count number of times the backend call returns false
      */
     void backendFalse() { mNrBackendReturnsFalse++; }
     /**
      * Count the number of strict inequalities added
      */
     void StrictInequalityAdded() { mNrOfStrictInequalitiesAdded++; }
     /**
      * Count the number of nonstrict inequalities added
      */
     void NonStrictInequalityAdded() { mNrOfNonStrictInequalitiesAdded++; }
     /**
      * Count the kind of constraint which was added 
      */
     void constraintAdded(Constraint_Relation relation) {
         switch(relation) {
         case CR_EQ:
             EqualityAdded();
             break;
         case CR_GEQ:
         case CR_LEQ:
             NonStrictInequalityAdded();
             break;
         case CR_NEQ:
         case CR_GREATER:
         case CR_LESS:
             StrictInequalityAdded();
             break;
         }
     }

       /**
      * Count the kind of constraint which was added 
      */
     void constraintRemoved(Constraint_Relation relation) {
         switch(relation) {
         case CR_EQ:
             EqualityRemoved();
             break;
         case CR_GEQ:
         case CR_LEQ:
             NonStrictInequalityRemoved();
             break;
         case CR_NEQ:
         case CR_GREATER:
         case CR_LESS:
             StrictInequalityRemoved();
             break;
         }
     }       
     /**
      * Count the number of equalities added
      */
     void EqualityAdded() { mNrOfEqualitiesAdded++; }
     /**
      * Count the number of strict inequalities removed
      */
     void StrictInequalityRemoved() { mNrOfStrictInequalitiesRemoved++; }
     /**
      * Count the number of nonstrict inequalities removed
      */
     void NonStrictInequalityRemoved() { mNrOfNonStrictInequalitiesRemoved++; }
     /**
      * Count the number of equalities removed
      */
     void EqualityRemoved() { mNrOfEqualitiesRemoved++; }
     
     /**
      * 
      */
     void DeducedEquality() { mNrDeducedEqualities++; }
     
     /**
      * Record how many deductions for inequalities have been found
      */
     void DeducedInequality() { mNrDeducedInequalities++; }
     /**
      * Record how many conflict sets were returned in each call.
      * @param nrInfeasibles the number of conflict sets before we return.
      */
     void NumberOfConflictSets(unsigned nrInfeasibles) { mNrOfConflictSets.push_back(nrInfeasibles); }
     /**
      * Record how big the conflict sets are w.r.t the whole set.
      * @param ratio The size of the conflict divided through the number of equalities
      */
     void EffectivenessOfConflicts(float ratio) { mEffectivenessOfConflicts.push_back(ratio); }
     
     void FoundEqualities() {
         ++mNrOfFoundEqualities;
     }
     
     void FoundIdentities() {
         ++mNrOfFoundIdentities;
     }
     /**
      * 
      * @param nrOfPops After an equality is removed, how many pop backtracks are coming.
      */
     void PopLevel(unsigned nrOfPops) { mPopLevel.push_back(nrOfPops); }
     
     void print(std::ostream& os = std::cout);
     void exportKeyValue(std::ostream& os = std::cout);
   protected:
    GroebnerModuleStats() : Statistics("GroebnerBasis", this), mNrCalls(0), mNrConstantGBs(0),
            mNrInfeasibleInequalities(0), mNrDeducedInequalities(0), mNrDeducedEqualities(0),mNrBackendReturnsFalse(0), mNrOfStrictInequalitiesAdded(0),
            mNrOfNonStrictInequalitiesAdded(0), mNrOfEqualitiesAdded(0), mNrOfStrictInequalitiesRemoved(0),
            mNrOfNonStrictInequalitiesRemoved(0), mNrOfEqualitiesRemoved(0), mNrOfFoundEqualities(0), mNrOfFoundIdentities(0)
    {}
    unsigned mNrCalls;
    unsigned mNrConstantGBs;
    unsigned mNrInfeasibleInequalities;
    unsigned mNrDeducedInequalities;
    unsigned mNrDeducedEqualities;
    unsigned mNrBackendReturnsFalse;
    unsigned mNrOfStrictInequalitiesAdded;
    unsigned mNrOfNonStrictInequalitiesAdded;
    unsigned mNrOfEqualitiesAdded;
    unsigned mNrOfStrictInequalitiesRemoved;
    unsigned mNrOfNonStrictInequalitiesRemoved;
    unsigned mNrOfEqualitiesRemoved;
    unsigned mNrOfFoundEqualities;
    unsigned mNrOfFoundIdentities;
    
    std::vector<unsigned> mNrOfConflictSets;
    std::vector<float> mEffectivenessOfConflicts;
    std::vector<unsigned> mPopLevel;
    
   private:
     static std::map<unsigned,GroebnerModuleStats*> instances;
};

}

#endif
#endif	/* GBMODULESTATISTICS_H */

