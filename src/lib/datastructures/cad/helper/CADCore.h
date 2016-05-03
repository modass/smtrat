#pragma once

#include "../Common.h"

namespace smtrat {
namespace cad {

template<CoreHeuristic CH>
struct CADCore {};

template<>
struct CADCore<CoreHeuristic::BySample> {
	template<typename CAD>
	Answer operator()(Assignment& assignment, CAD& cad) {
		cad.mLifting.resetFullSamples();
		cad.mLifting.restoreRemovedSamples();
		while (true) {
			SMTRAT_LOG_DEBUG("smtrat.cad", "Current sample tree:" << std::endl << cad.mLifting.getTree());
			SMTRAT_LOG_DEBUG("smtrat.cad", "Current sample queue:" << std::endl << cad.mLifting.getLiftingQueue());
			Answer res = cad.checkFullSamples(assignment);
			if (res == Answer::SAT) return Answer::SAT;
			
			if (!cad.mLifting.hasNextSample()) {
				SMTRAT_LOG_DEBUG("smtrat.cad", "There is no sample to be lifted.");
				break;
			}
			auto it = cad.mLifting.getNextSample();
			Sample& s = *it;
			SMTRAT_LOG_DEBUG("smtrat.cad", "Sample " << s << " at depth " << it.depth());
			SMTRAT_LOG_DEBUG("smtrat.cad", "Current sample: " << cad.mLifting.printSample(it));
			assert(0 <= it.depth() && it.depth() < cad.dim());
			if (s.hasConflictWithConstraint()) {
				SMTRAT_LOG_DEBUG("smtrat.cad", "Sample " << s << " already has a conflict.");
				cad.mLifting.removeNextSample();
				continue;
			}
			auto polyID = cad.mProjection.getPolyForLifting(cad.idLP(it.depth() + 1), s.liftedWith());
			if (polyID) {
				const auto& poly = cad.mProjection.getPolynomialById(cad.idLP(it.depth() + 1), *polyID);
				SMTRAT_LOG_DEBUG("smtrat.cad", "Lifting " << s << " with " << poly);
				cad.mLifting.liftSample(it, poly, *polyID);
			} else {
				cad.mLifting.removeNextSample();
				if (!cad.mLifting.hasNextSample()) {
					SMTRAT_LOG_DEBUG("smtrat.cad", "Got nothing to lift anymore, projecting into level " << idLP(it.depth() + 1) << " ...");
					Bitset gotNewPolys = cad.mProjection.projectNewPolynomial();
					if (gotNewPolys.any()) {
						SMTRAT_LOG_DEBUG("smtrat.cad", "Current projection:" << std::endl << cad.mProjection);
						cad.mLifting.restoreRemovedSamples();
					}
				}
			}
		}
		return Answer::UNSAT;
	}
};

template<>
struct CADCore<CoreHeuristic::PreferProjection> {
	template<typename CAD>
	Answer operator()(Assignment& assignment, CAD& cad) {
		cad.mLifting.resetFullSamples();
		cad.mLifting.restoreRemovedSamples();
		while (true) {
			Answer res = cad.checkFullSamples(assignment);
			if (res == Answer::SAT) return Answer::SAT;
			
			if (!cad.mLifting.hasNextSample()) {
				SMTRAT_LOG_DEBUG("smtrat.cad", "There is no sample to be lifted.");
				return Answer::UNSAT;
			}
			
			auto it = cad.mLifting.getNextSample();
			Sample& s = *it;
			assert(0 <= it.depth() && it.depth() < cad.dim());
			if (s.hasConflictWithConstraint()) {
				cad.mLifting.removeNextSample();
				continue;
			}
			
			auto polyID = cad.mProjection.getPolyForLifting(cad.idLP(it.depth() + 1), s.liftedWith());
			if (polyID) {
				const auto& poly = cad.mProjection.getPolynomialById(cad.idLP(it.depth() + 1), *polyID);
				SMTRAT_LOG_DEBUG("smtrat.cad", "Lifting " << s << " with " << poly);
				cad.mLifting.liftSample(it, poly, *polyID);
			} else {
				SMTRAT_LOG_DEBUG("smtrat.cad", "Got no polynomial for " << s << ", projecting into level " << cad.idLP(it.depth() + 1) << " ...");
				SMTRAT_LOG_DEBUG("smtrat.cad", "Current projection:" << std::endl << cad.mProjection);
				auto gotNewPolys = cad.mProjection.projectNewPolynomial();
				SMTRAT_LOG_DEBUG("smtrat.cad", "Tried to project polynomials into level " << cad.idLP(it.depth() + 1) << ", result = " << gotNewPolys);
				if (gotNewPolys.any()) {
					cad.mLifting.restoreRemovedSamples();
				} else if (cad.mProjection.empty(cad.idLP(it.depth() + 1))) {
					if (!cad.mLifting.addTrivialSample(it)) {
						cad.mLifting.removeNextSample();
					}
				} else {
					cad.mLifting.removeNextSample();
				}
			}
		}
	}
};

template<>
struct CADCore<CoreHeuristic::PreferSampling> {
	template<typename CAD>
	Answer operator()(Assignment& assignment, CAD& cad) {
		cad.mLifting.resetFullSamples();
		while (true) {
			cad.mLifting.restoreRemovedSamples();
			while (cad.mLifting.hasNextSample() || cad.mLifting.hasFullSamples()) {
				Answer res = cad.checkFullSamples(assignment);
				if (res == Answer::SAT) return Answer::SAT;
				
				auto it = cad.mLifting.getNextSample();
				Sample& s = *it;
				assert(0 <= it.depth() && it.depth() < cad.dim());
				if (s.hasConflictWithConstraint()) {
					cad.mLifting.removeNextSample();
					continue;
				}
				auto polyID = cad.mProjection.getPolyForLifting(cad.idLP(it.depth() + 1), s.liftedWith());
				if (polyID) {
					const auto& poly = cad.mProjection.getPolynomialById(cad.idLP(it.depth() + 1), *polyID);
					SMTRAT_LOG_DEBUG("smtrat.cad", "Lifting " << s << " with " << poly);
					cad.mLifting.liftSample(it, poly, *polyID);
				} else {
					if (!cad.mLifting.addTrivialSample(it)) {
						cad.mLifting.removeNextSample();
					}
				}
			}
			
			auto r = cad.mProjection.projectNewPolynomial();
			if (r.none()) return Answer::UNSAT;
		}
	}
};

}
}
