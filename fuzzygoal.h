/*
 * Copyright (C) 2026 Olaf Frommann
 * * This file is part of the FuzzyGoal toolkit.
 *
 * FuzzyGoal is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * FuzzyGoal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FUZZYGOAL_H
#define FUZZYGOAL_H

#include <vector>
#include <string>
#include <utility>

// =============================================================
// FuzzyGoal
// -------------------------------------------------------------
// Public interface for constructing and evaluating fuzzy
// multi-criteria objective functions with inequality criteria,
// equality constraints, rule-based aggregation, and
// defuzzification.
// =============================================================
class FuzzyGoal
{
public:
    // =========================================================
    // Membership function variants
    // =========================================================
    enum MembershipFunction
    {
        LinearRefLinearTol,
        LinearRefParabolicTol,

        LinearGlobalLinearTol,
        LinearGlobalParabolicTol,

        ParabolicRefParabolicTol,
        ParabolicGlobalParabolicTol,

        LinearGlobalGaussianTol,
        GaussianRefGaussianTol,
        GaussianGlobalGaussianTol
    };

    // =========================================================
    // Aggregation operators for fuzzy rules
    // =========================================================
    enum AggregationOperator
    {
        MinMax,        // min / max
        Product,       // product / probabilistic sum
        SoftMinMax,    // smooth min / smooth max
        PNorm          // p-norm approximation
    };

    // =========================================================
    // Boundary behavior outside the modeled interval
    // =========================================================
    enum BoundaryBehavior
    {
        // Saturate memberships outside the interval.
        HardSaturation,

        // On the favorable side outside the interval, the desirable
        // membership is reduced linearly to avoid a flat saturated
        // region and to provide directional information.
        //
        // On the unfavorable side, the value is saturated as undesirable.
        LinearGuidance
    };

    // =========================================================
    // Aggregation descriptor
    //
    // parameter:
    //   - epsilon for SoftMinMax
    //   - p < 0   for PNorm
    // =========================================================
    struct Aggregation
    {
        AggregationOperator type;
        double parameter;

    private:
        Aggregation(AggregationOperator t, double p)
        : type(t), parameter(p) {}

    public:
        static Aggregation MinMaxAgg()
        {
            return Aggregation(MinMax, 0.0);
        }

        static Aggregation ProductAgg()
        {
            return Aggregation(Product, 0.0);
        }

        static Aggregation SoftMinMaxAgg(double eps)
        {
            return Aggregation(SoftMinMax, eps);
        }

        static Aggregation PNormAgg(double p)
        {
            return Aggregation(PNorm, p);
        }
    };

    // =========================================================
    // Criterion direction
    // =========================================================
    enum Direction
    {
        SmallerIsBetter,
        LargerIsBetter
    };

    // =========================================================
    // Status codes for criterion creation
    // =========================================================
    enum AddCriterionStatus
    {
        CriterionOk,
        InvalidInterval,
        RefBelowInterval,
        RefAboveInterval,
        RefTooCloseToLowerBound,
        RefTooCloseToUpperBound,
        InvalidSigma
    };

    // =========================================================
    // Status codes for rule creation
    // =========================================================
    enum AddRuleStatus
    {
        RuleOk,
        RuleUnknownCriterionA,
        RuleUnknownCriterionB,
        RuleInvalidAggregation
    };

    // =========================================================
    // Status codes for evaluation
    // =========================================================
    enum EvaluationStatus
    {
        EvaluationOk,
        EvaluationUnknownCriterion,
        EvaluationMissingCriterion,
        EvaluationDuplicateCriterion,
        EvaluationDefuzzificationFailed
    };

    static const char* statusToString(AddCriterionStatus status);
    static const char* statusToString(AddRuleStatus status);
    static const char* statusToString(EvaluationStatus status);

    // =========================================================
    // Result handle for newly created criteria
    // =========================================================
    struct NewCriterion
    {
        AddCriterionStatus status;
        int id;   // valid only if status == CriterionOk
    };

    // =========================================================
    // Result handle for newly created rules
    // =========================================================
    struct NewRule
    {
        AddRuleStatus status;
        int id;   // valid only if status == RuleOk
    };

    // =========================================================
    // Result of evaluating a single criterion
    // =========================================================
    struct CriterionMembership
    {
        double desirable;
        double tolerable;
        double undesirable;
    };

    // =========================================================
    // Construction
    // =========================================================
    FuzzyGoal();

    // =========================================================
    // Criterion handling
    // =========================================================
    NewCriterion addCriterion(
        const std::string& name,
        double c0,
        double c1,
        double cRef,
        Direction direction,
        MembershipFunction mf
    );

    NewCriterion addCriterion(
        const std::string& name,
        double c0,
        double c1,
        double cRef,
        Direction direction,
        MembershipFunction mf,
        double sigmaRel
    );

    NewCriterion addCriterion(
        const std::string& name,
        double c0,
        double c1,
        double cRef,
        Direction direction,
        MembershipFunction mf,
        BoundaryBehavior boundaryBehavior
    );

    NewCriterion addCriterion(
        const std::string& name,
        double c0,
        double c1,
        double cRef,
        Direction direction,
        MembershipFunction mf,
        BoundaryBehavior boundaryBehavior,
        double sigmaRel
    );

    EvaluationStatus evaluateCriterionMembership(
        int criterionId,
        double value,
        CriterionMembership& membership
    ) const;

    // =========================================================
    // Equality constraints
    // ---------------------------------------------------------
    // Equality constraints are evaluated internally via
    //
    //     c = |g(x) - g*|
    //
    // The user passes g(x) as value during evaluation.
    //
    // targetValue corresponds to g*.
    // tolerance is used as the reference value cRef.
    // gamma defines the upper interval boundary:
    //
    //     c0   = 0
    //     cRef = tolerance
    //     c1   = gamma * tolerance
    // =========================================================
    NewCriterion addEqualityConstraint(
        const std::string& name,
        double targetValue,
        double tolerance,
        MembershipFunction mf = GaussianGlobalGaussianTol,
        double sigmaRel = 0.15
    );

    NewCriterion addEqualityConstraint(
        const std::string& name,
        double targetValue,
        double tolerance,
        MembershipFunction mf,
        double sigmaRel,
        double gamma
    );

    // Convenience overload for targetValue = 0.
    NewCriterion addEqualityConstraint(
        const std::string& name,
        double tolerance,
        MembershipFunction mf = GaussianGlobalGaussianTol,
        double sigmaRel = 0.15
    );

    NewCriterion addEqualityConstraint(
        const std::string& name,
        double tolerance,
        MembershipFunction mf,
        double sigmaRel,
        double gamma
    );

    void removeCriterion(int criterionId);

    // =========================================================
    // Rule handling
    // =========================================================
    enum RuleMembership
    {
        Desirable,
        Tolerable,
        Undesirable
    };

    enum RuleOperator
    {
        And,
        Or
    };

    NewRule addRule(
        int criterionA, RuleMembership memA,
        int criterionB, RuleMembership memB,
        RuleOperator op,
        RuleMembership outcome
    );

    NewRule addRule(
        int criterionA, RuleMembership memA,
        int criterionB, RuleMembership memB,
        RuleOperator op,
        RuleMembership outcome,
        Aggregation aggregation
    );

    void removeRule(int ruleId);

    // =========================================================
    // Evaluation
    //
    // Strict evaluation:
    //   - each registered criterion must be provided exactly once
    //   - unknown criterion ids cause evaluation to fail
    //   - missing criterion values cause evaluation to fail
    // =========================================================
    EvaluationStatus evaluate(
        const std::vector<std::pair<int,double>>& values,
        double& objectiveValue
    ) const;

private:
    class SingleCriterion
    {
    public:
        SingleCriterion(
            double c0,
            double c1,
            double cRef,
            Direction direction,
            MembershipFunction mf,
            BoundaryBehavior bb
        );

        SingleCriterion(
            double c0,
            double c1,
            double cRef,
            Direction direction,
            MembershipFunction mf,
            BoundaryBehavior bb,
            double sigmaRel
        );

        void evaluateMemberships(
            double c,
            double& md,
            double& mt,
            double& mu
        ) const;

    private:
        double c0;
        double c1;
        double cRef;

        Direction direction;
        MembershipFunction mf;
        BoundaryBehavior boundaryBehavior;

        double sigmaRel;
        double sigmaTolLeft;
        double sigmaTolRight;
        double sigmaGlob;

        double linearDesirableRef(double c) const;
        double linearDesirableGlobal(double c) const;
        double linearTolerableLeft(double c) const;
        double linearTolerableRight(double c) const;
        double linearUndesirableRef(double c) const;
        double linearUndesirableGlobal(double c) const;

        double parabolicDesirableRef(double c) const;
        double parabolicDesirableGlobal(double c) const;
        double parabolicTolerableLeft(double c) const;
        double parabolicTolerableRight(double c) const;
        double parabolicUndesirableRef(double c) const;
        double parabolicUndesirableGlobal(double c) const;

        double gaussianDesirableGlobal(double c) const;
        double gaussianTolerableLeft(double c) const;
        double gaussianTolerableRight(double c) const;
        double gaussianUndesirableGlobal(double c) const;

        void computeLinearDesUndesRef_LinearTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeLinearDesUndesRef_ParabolicTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeLinearDesUndesGlobal_LinearTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeLinearDesUndesGlobal_ParabolicTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeParabolicDesUndesRef_ParabolicTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeParabolicDesUndesGlobal_ParabolicTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeLinearDesUndesGlobal_GaussianTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeGaussianDesUndesRef_GaussianTolRef(
            double c, double& md, double& mt, double& mu) const;

        void computeGaussianDesUndesGlobal_GaussianTolRef(
            double c, double& md, double& mt, double& mu) const;
    };

    class Rule
    {
    public:
        Rule(
            int a, RuleMembership ma,
            int b, RuleMembership mb,
            RuleOperator op,
            RuleMembership out,
            Aggregation agg
        );

        double evaluate(double muA, double muB) const;

        int critA;
        int critB;

        RuleMembership memA;
        RuleMembership memB;

        RuleOperator op;
        RuleMembership outcome;

        Aggregation aggregation;
    };

    struct FuzzyInferenceResult
    {
        double desirable   = 0.0;
        double tolerable   = 0.0;
        double undesirable = 0.0;
    };

    class Defuzzifier
    {
    public:
        Defuzzifier();

        bool defuzzify(
            const FuzzyInferenceResult& inf,
            double& value
        ) const;

    private:
        double sd;
        double st;
        double su;
    };

    struct CriterionEntry
    {
        CriterionEntry(
            int id_,
            const std::string& name_,
            const SingleCriterion& criterion_,
            bool isEquality_,
            double targetValue_
        )
        : id(id_),
          name(name_),
          criterion(criterion_),
          isEquality(isEquality_),
          targetValue(targetValue_)
        {}

        int id;
        std::string name;
        SingleCriterion criterion;

        bool isEquality;
        double targetValue;
    };

    struct RuleEntry
    {
        RuleEntry(int id_, const Rule& rule_)
        : id(id_), rule(rule_) {}

        int id;
        Rule rule;
    };

    std::vector<CriterionEntry> criteria;
    std::vector<RuleEntry> rules;

    int nextCriterionId;
    int nextRuleId;

    const CriterionEntry* findCriterion(int id) const;

    double transformCriterionValue(
        const CriterionEntry& criterion,
        double rawValue
    ) const;

    EvaluationStatus validateEvaluationInput(
        const std::vector<std::pair<int,double>>& values
    ) const;

    FuzzyInferenceResult inference(
        const std::vector<std::pair<int,double>>& values
    ) const;
};

#endif // FUZZYGOAL_H
