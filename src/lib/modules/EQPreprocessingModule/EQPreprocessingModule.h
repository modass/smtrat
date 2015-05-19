/*
 * SMT-RAT - Satisfiability-Modulo-Theories Real Algebra Toolbox
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
 * @file EQPreprocessingModule.h
 * @author YOUR NAME <YOUR EMAIL ADDRESS>
 *
 * @version 2014-12-05
 * Created on 2014-12-05.
 */

#pragma once

#include <unordered_map>
#include <iostream>

#include "../../Common.h"
#include "../../solver/Module.h"
#include "ModuleWrapper.h"
#include "../EQModule/VariantHash.h"
#include "../EQModule/PairHash.h"
#include "../../datastructures/eq/union_find.h"
#include "../EQModule/pmatrix.hpp"
#include "EQPreprocessingStatistics.h"
#include "EQPreprocessingSettings.h"
#include "FormulaVisitor.hpp"
#include "JunctorMerger.h"
#include "EQPreprocessingUFRewriter.h"
#include "NNFRewriter.h"
#include "BoolUEQRewriter.h"
#include "OrPathShortener.h"
#include "ReplaceVariablesRewriter.h"

namespace smtrat
{
	template<typename Settings>
		class EQPreprocessingModule : public Module
	{
		private:
			typedef carl::UVariable UVariable;
		    typedef carl::UFInstance UFInstance;
		    typedef carl::UninterpretedFunction UFunction;
		    typedef carl::UEquality UEquality;
		    typedef UEquality::Arg UArg;

		    // Members.
		    // helper EQ module
		    ModuleWrapper<EQModule<EQSettingsForPreprocessing>> mEQHelper;

		    // map facts we have found to their truth value and origin
		    std::unordered_map<FormulaT, bool> mFacts;
		    std::unordered_map<FormulaT, FormulaT> mFactOrigins;

		    // mapping of input (sub)formulas to new formulas
		    std::unordered_multimap<FormulaT, FormulaT> mOldToNew;

		    // rewriter for function instances; only used if that option is set
		    formula_rewriter<UFRewriter> mRewriter;

		    // rewriter for bool domain constraints to UEQ
		    formula_rewriter<BoolUEQRewriter>* mBoolRewriter;

#ifdef SMTRAT_DEVOPTION_Statistics
		    // statistics collection
		    EQPreprocessingStatistics* mStatistics;
#endif

		    // helper types
		    typedef typename decltype(mOldToNew)::iterator old_to_new_iter;

			// helper methods
		    template<typename Rewriter, typename... Args> bool apply_rewriting(Args&&... args);
		    template<typename Rewriter, typename... Args> inline bool apply_to_formula(old_to_new_iter formula, Args&&... args);

			void P_do_preprocessing();

			// Transform the formula insert NNF (negation normal form)
			void P_NNF_transform();

			// On all formulas, rewrite functional congruences and function instances if that setting is enabled
			void P_rewrite_congruences();

			// on all formulas, rewrite boolean domain constraints if that setting is enabled
			void P_rewrite_bool_domain();

			// the actual implementation of updateModel
			void P_update_model();

			// collect all facts (literals that always have a specific value); result.first is false iff unsat, result.second is false iff stable
			std::pair<bool,bool> P_collect_facts();
			inline std::pair<bool,bool> P_collect_fact(const FormulaT& origin, const FormulaT& fact, bool negated);

		public:
			EQPreprocessingModule(ModuleType _type, const ModuleInput* _formula, RuntimeSettings* _settings, Conditionals& _conditionals, Manager* _manager = NULL);
			~EQPreprocessingModule();

			void updateModel() const;

			// Main interfaces. This module does not need any of the other methods, we do all the work in isConsistent.

			/**
			 * Checks the received formula for consistency.
			 * @return True,	if the received formula is satisfiable;
			 *		 False,   if the received formula is not satisfiable;
			 *		 Unknown, otherwise.
			 */
			Answer isConsistent();
	};
	
	extern template class EQPreprocessingModule<EQPreprocessingSettings1>;
	extern template class EQPreprocessingModule<EQPreprocessingSettings2>;
}

#include "EQPreprocessingModule.tpp"
