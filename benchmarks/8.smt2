(set-logic QF_NRA)
(declare-fun a () Real)
(declare-fun b () Real)
(declare-fun c () Real)
(declare-fun x () Real)
(declare-fun y () Real)
(declare-fun z () Real)
(assert (implies (and (> a 0) (not (= (+ (+ (* a (* z z)) (* b z)) c) 0))) (< y (+ (+ (* a (* x x)) (* b x)) c))))
(eliminate-quantifiers (forall x a b c) (exists z))
(exit)
