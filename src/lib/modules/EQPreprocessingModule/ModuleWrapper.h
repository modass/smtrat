#ifndef SRC_LIB_MODULES_EQPREPROCESSINGMODULE_MODULE_WRAPPER_H_
#define SRC_LIB_MODULES_EQPREPROCESSINGMODULE_MODULE_WRAPPER_H_

#include "../../Common.h"
#include "../../solver/Module.h"
#include "../EQModule/EQModule.h"

namespace smtrat {
	template<typename M> class ModuleWrapper {
		public:
			ModuleWrapper(ModuleType type) :
				input(),
				module(type, &input, nullptr, conditionals, nullptr)
			{}

			M& get() noexcept { return module; }

			bool assertSubformula(const FormulaT& formula) {
				ModuleInput::iterator iter;
				bool added;

				std::tie(iter, added) = input.add(formula);
				if(added) {
					module.inform(formula);
				}

				asserted.insert(formula);
				return module.assertSubformula(iter);
			}

			void removeSubformula(const FormulaT& formula) {
				ModuleInput::iterator iter = input.find(formula);
				if(iter != input.end()) {
					asserted.erase(formula);
					module.removeSubformula(iter);
				}
			}

			bool isAsserted(const FormulaT& formula) {
				return asserted.count(formula);
			}

			bool isConsistent() {
				return module.isConsistent() != False;
			}

			const std::vector<FormulasT>& infeasibleSubsets() {
				return module.infeasibleSubsets();
			}

		private:
			FormulasT asserted;
			Conditionals conditionals;
			ModuleInput input;
			M module;
	};
}

#endif
