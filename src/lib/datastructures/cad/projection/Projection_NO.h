#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "../Common.h"
#include "BaseProjection.h"

namespace smtrat {
namespace cad {
	
	/**
	 * This class implements a projection that supports no incrementality and expects backtracking to be in order.
	 *
	 * It is based on the following data structures:
	 * - mPolynomialIDs: maps polynomials to a (per level) unique id
	 * - mPolynomials: stores polynomials as a list (per level) with their origin
	 *
	 * The origin of a polynomial in level zero is the id of the corresponding constraint.
	 * For all other levels, it is the id of some polynomial from level zero such that the polynomial must be removed if the origin is removed.
	 * For a single projection operation, the resulting origin is the largest of the participating polynomials.
	 * If a polynomial is derived from multiple projection operations, the origin is the earliest and thus smallest, at least for this non-incremental setting.
	 */
	template<typename Settings>
	class Projection<Incrementality::NONE, Backtracking::ORDERED, Settings>: public BaseProjection {
	private:
		/// Maps polynomials to a (per level) unique ID.
		std::vector<std::map<UPoly,std::size_t>> mPolynomialIDs;
		/// Stores polynomials with their origin constraint ids.
		std::vector<std::vector<std::pair<UPoly,std::size_t>>> mPolynomials;
		
		/// Inserts a polynomial with the given origin into the given level.
		void insertPolynomial(std::size_t level, const UPoly& p, std::size_t origin) {
			std::size_t id = mPolynomials[level].size();
			mPolynomials[level].emplace_back(p, origin);
			mPolynomialIDs[level].emplace(p, id);
			mLiftingQueues[level].insert(id);
		}
		/// Removed the last polynomial from the given level.
		void removePolynomial(std::size_t level) {
			auto it = mPolynomialIDs[level].find(mPolynomials[level].back().first);
			assert(it != mPolynomialIDs[level].end());
			mLiftingQueues[level].erase(it->second);
			mPolynomialIDs[level].erase(it);
			mPolynomials[level].pop_back();
		}
		
		/// Adds a new polynomial to the given level and perform the projection recursively.
		void addToProjection(std::size_t level, const UPoly& p, std::size_t origin) {
			if (canBePurged(p)) return;
			if ((level > 0) && (level < dim() - 1) && canBeForwarded(level, p)) {
				addToProjection(level+1, p.switchVariable(var(level+1)), origin);
				return;
			}
			SMTRAT_LOG_DEBUG("smtrat.cad.projection", "Adding " << p << " to projection level " << level);
			assert(level < dim());
			assert(p.mainVar() == var(level));
			auto it = mPolynomialIDs[level].find(p);
			if (it != mPolynomialIDs[level].end()) {
				// We already have this polynomial.
				if (level > 0) {
					assert(mPolynomials[level][it->second].second <= origin);
				}
				return;
			}
			if (level == 0) {
				// Change origin to the id of this polynomial
				origin = mPolynomials[level].size();
			}
			if (level < dim() - 1) {
				mOperator(Settings::projectionOperator, p, var(level + 1), 
					[&](const UPoly& np){ addToProjection(level + 1, np, origin); }
				);
				for (const auto& it: mPolynomials[level]) {
					std::size_t newOrigin = std::max(origin, it.second);
					mOperator(Settings::projectionOperator, p, it.first, var(level + 1),
						[&](const UPoly& np){ addToProjection(level + 1, np, newOrigin); }
					);
				}
			}
			// Actually insert afterwards to avoid pairwise projection with itself.
			insertPolynomial(level, p, origin);
		}
	public:
		/**
		 * Resets all datastructures, use the given variables from now on.
		 */
		void reset(const std::vector<carl::Variable>& vars) {
			BaseProjection::reset(vars);
			mPolynomials.clear();
			mPolynomials.resize(dim());
			mPolynomialIDs.clear();
			mPolynomialIDs.resize(dim());
		}
		/**
		 * Adds the given polynomial to the projection with the given constraint id as origin.
		 * Asserts that the main variable of the polynomial is the first variable.
		 */
		void addPolynomial(const UPoly& p, std::size_t cid) {
			assert(p.mainVar() == var(0));
			addToProjection(0, p, cid);
		}
		/**
		 * Removed the given polynomial from the projection.
		 * Asserts that this polynomial was the one added last and has the given constraint id as origin.
		 * Calls the callback function for every level with a mask designating the polynomials removed from this level.
		 */
		void removePolynomial(const UPoly& p, std::size_t cid) {
			assert(mPolynomials[0].back().first == p);
			assert(mPolynomials[0].back().second == cid);
			removePolynomial(0);
			std::size_t origin = mPolynomials[0].size();
			mRemoveCallback(0, SampleLiftedWith().set(origin));
			// Remove all polynomials from all levels that have the removed polynomial as origin.
			for (std::size_t level = 1; level < dim(); level++) {
				Bitset removed;
				if (mPolynomials[level].empty()) continue;
				while (mPolynomials[level].back().second == origin) {
					removePolynomial(level);
					removed.set(mPolynomials[level].size());
				}
				assert(mPolynomials[level].back().second < origin);
				mRemoveCallback(level, removed);
			}
		}
		
		/// Returns the number of polynomials in this level.
		std::size_t size(std::size_t level) const {
			return mPolynomials[level].size();
		}
		/// Returns whether the number of polynomials in this level is zero.
		bool empty(std::size_t level) const {
			return mPolynomials[level].empty();
		}
		
		/// Returns false, as the projection is not incremental.
		bool projectNewPolynomial(std::size_t level, const ConstraintSelection& ps = Bitset(true)) {
			return false;
		}
		/// Get a polynomial from this level suited for lifting.
		OptionalPoly getPolyForLifting(std::size_t level, SampleLiftedWith& slw) {
			for (const auto& pid: mLiftingQueues[level]) {
				SMTRAT_LOG_DEBUG("smtrat.cad.projection", "Checking " << mPolynomials[level][pid].first);
				if (!slw.test(pid)) {
					SMTRAT_LOG_DEBUG("smtrat.cad.projection", mPolynomials[level][pid].first << " can be used.");
					slw.set(pid);
					return OptionalPoly(mPolynomials[level][pid].first);
				} else {
					SMTRAT_LOG_DEBUG("smtrat.cad.projection", mPolynomials[level][pid].first << " was already used.");
				}
			}
			return OptionalPoly();
		}
		/// Get a polynomial from this level suited for lifting.
		OptionalPoly getPolyForLifting(std::size_t level, SampleLiftedWith& slw, const ConstraintSelection& cs) {
			for (const auto& pid: mLiftingQueues[level]) {
				if (!slw.test(pid)) {
					if ((mPolynomials[level][pid].second & cs).any()) {
						slw.set(pid);
						return OptionalPoly(mPolynomials[level][pid].first);
					}
				}
			}
			return OptionalPoly();
		}
		/// Get the polynomial from this level with the given id.
		const UPoly& getPolynomialById(std::size_t level, std::size_t id) const {
			assert(level < mPolynomials.size());
			assert(id < mPolynomials[level].size());
			return mPolynomials[level][id].first;
		}
		
		template<typename S>
		friend std::ostream& operator<<(std::ostream& os, const Projection<Incrementality::NONE, Backtracking::ORDERED, S>& p) {
			for (std::size_t level = 0; level < p.dim(); level++) {
				os << level << " " << p.var(level) << ":" << std::endl;
				for (const auto& it: p.mPolynomials[level]) {
					os << "\t" << it.first << " [" << it.second << "]" << std::endl;
				}
			}
			return os;
		}
	};
}
}