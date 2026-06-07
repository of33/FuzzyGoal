#include "fuzzygoal.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    int failures = 0;

    void check(bool condition, const std::string& message)
    {
        if (!condition)
        {
            std::cerr << "[FAILED] " << message << "\n";
            ++failures;
        }
        else
        {
            std::cout << "[OK]     " << message << "\n";
        }
    }

    bool approx(double a, double b, double tol = 1e-12)
    {
        return std::abs(a - b) <= tol;
    }
}

// ============================================================
// Anchor values tests
// ============================================================

void testDefuzzificationAnchorValues()
{
    std::cout << "\n=== Defuzzification anchor values ===\n";

    FuzzyGoal goal;

    auto c = goal.addCriterion(
        "c",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    check(c.status == FuzzyGoal::CriterionOk,
          "criterion for defuzzification anchor test created");

    double objective = -1.0;

    auto status0 = goal.evaluate({{c.id, 0.0}}, objective);

    check(status0 == FuzzyGoal::EvaluationOk,
          "evaluation at desirable anchor returns EvaluationOk");

    check(approx(objective, 0.0),
          "pure desirable result maps to objective value 0");

    auto statusRef = goal.evaluate({{c.id, 0.5}}, objective);

    check(statusRef == FuzzyGoal::EvaluationOk,
          "evaluation at tolerable anchor returns EvaluationOk");

    check(approx(objective, 0.5),
          "pure tolerable result maps to objective value 0.5");

    auto status1 = goal.evaluate({{c.id, 1.0}}, objective);

    check(status1 == FuzzyGoal::EvaluationOk,
          "evaluation at undesirable anchor returns EvaluationOk");

    check(approx(objective, 1.0),
          "pure undesirable result maps to objective value 1");
}

// ============================================================
// Criterion creation tests
// ============================================================

void testCriterionCreation()
{
    std::cout << "\n=== Criterion creation ===\n";

    FuzzyGoal goal;

    auto c = goal.addCriterion(
        "valid",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    check(c.status == FuzzyGoal::CriterionOk,
          "valid criterion returns CriterionOk");

    check(c.id >= 0,
          "valid criterion returns non-negative id");

    auto invalidInterval = goal.addCriterion(
        "invalid_interval",
        1.0,
        0.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    check(invalidInterval.status == FuzzyGoal::InvalidInterval,
          "invalid interval returns InvalidInterval");

    auto refBelow = goal.addCriterion(
        "ref_below",
        0.0,
        1.0,
        -0.1,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    check(refBelow.status == FuzzyGoal::RefBelowInterval,
          "reference below interval returns RefBelowInterval");

    auto refAbove = goal.addCriterion(
        "ref_above",
        0.0,
        1.0,
        1.1,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    check(refAbove.status == FuzzyGoal::RefAboveInterval,
          "reference above interval returns RefAboveInterval");

    auto invalidSigma = goal.addCriterion(
        "invalid_sigma",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::GaussianGlobalGaussianTol,
        FuzzyGoal::HardSaturation,
        -0.1
    );

    check(invalidSigma.status == FuzzyGoal::InvalidSigma,
          "negative sigmaRel for Gaussian membership returns InvalidSigma");
}

// ============================================================
// Membership tests
// ============================================================

void testAllMembershipFunctionsAreUsable()
{
    std::cout << "\n=== All membership functions ===\n";

    const FuzzyGoal::MembershipFunction functions[] =
    {
        FuzzyGoal::LinearRefLinearTol,
        FuzzyGoal::LinearRefParabolicTol,
        FuzzyGoal::LinearGlobalLinearTol,
        FuzzyGoal::LinearGlobalParabolicTol,
        FuzzyGoal::ParabolicRefParabolicTol,
        FuzzyGoal::ParabolicGlobalParabolicTol,
        FuzzyGoal::LinearGlobalGaussianTol,
        FuzzyGoal::GaussianRefGaussianTol,
        FuzzyGoal::GaussianGlobalGaussianTol
    };

    for (FuzzyGoal::MembershipFunction mf : functions)
    {
        FuzzyGoal goal;

        auto c = goal.addCriterion(
            "criterion",
            0.0,
            1.0,
            0.5,
            FuzzyGoal::SmallerIsBetter,
            mf
        );

        check(c.status == FuzzyGoal::CriterionOk,
              "membership function can create a criterion");

        if (c.status != FuzzyGoal::CriterionOk)
            continue;

        FuzzyGoal::CriterionMembership m;

        auto status = goal.evaluateCriterionMembership(
            c.id,
            0.5,
            m
        );

        check(status == FuzzyGoal::EvaluationOk,
              "membership function can be evaluated");

        check(m.desirable >= 0.0 && m.desirable <= 1.0,
              "desirable membership lies in [0, 1]");

        check(m.tolerable >= 0.0 && m.tolerable <= 1.0,
              "tolerable membership lies in [0, 1]");

        check(m.undesirable >= 0.0 && m.undesirable <= 1.0,
              "undesirable membership lies in [0, 1]");

        double objective = 0.0;

        auto evalStatus = goal.evaluate(
            {
                {c.id, 0.5}
            },
            objective
        );

        check(evalStatus == FuzzyGoal::EvaluationOk,
              "membership function can be used in objective evaluation");

        check(objective >= 0.0 && objective <= 1.0,
              "objective value lies in [0, 1]");
    }
}

// ============================================================
// Specific GaussianRefGaussianTol tests
// ============================================================

void testGaussianRefGaussianTol()
{
    std::cout << "\n=== GaussianRefGaussianTol ===\n";

    FuzzyGoal goal;

    auto c = goal.addCriterion(
        "gaussian_ref",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::GaussianRefGaussianTol
    );

    check(c.status == FuzzyGoal::CriterionOk,
          "GaussianRefGaussianTol criterion is created with default sigmaRel");

    if (c.status != FuzzyGoal::CriterionOk)
        return;

    FuzzyGoal::CriterionMembership m;

    auto status = goal.evaluateCriterionMembership(c.id, 0.5, m);

    check(status == FuzzyGoal::EvaluationOk,
          "GaussianRefGaussianTol evaluates at reference value");

    check(approx(m.tolerable, 1.0),
          "GaussianRefGaussianTol has tolerable = 1 at reference value");

    check(approx(m.desirable, 0.0),
          "GaussianRefGaussianTol has desirable = 0 at reference value");

    check(approx(m.undesirable, 0.0),
          "GaussianRefGaussianTol has undesirable = 0 at reference value");

    status = goal.evaluateCriterionMembership(c.id, 0.0, m);

    check(status == FuzzyGoal::EvaluationOk,
          "GaussianRefGaussianTol evaluates at lower bound");

    check(approx(m.desirable, 1.0),
          "GaussianRefGaussianTol has desirable = 1 at lower bound for SmallerIsBetter");
}

// ============================================================
// Rule creation tests
// ============================================================

void testRuleCreation()
{
    std::cout << "\n=== Rule creation ===\n";

    FuzzyGoal goal;

    auto c1 = goal.addCriterion(
        "c1",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    auto c2 = goal.addCriterion(
        "c2",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    check(c1.status == FuzzyGoal::CriterionOk,
          "first criterion for rule test created");

    check(c2.status == FuzzyGoal::CriterionOk,
          "second criterion for rule test created");

    auto rule = goal.addRule(
        c1.id, FuzzyGoal::Desirable,
        c2.id, FuzzyGoal::Desirable,
        FuzzyGoal::And,
        FuzzyGoal::Desirable
    );

    check(rule.status == FuzzyGoal::RuleOk,
          "valid rule returns RuleOk");

    check(rule.id >= 0,
          "valid rule returns non-negative id");

    auto unknownA = goal.addRule(
        9999, FuzzyGoal::Desirable,
        c2.id, FuzzyGoal::Desirable,
        FuzzyGoal::And,
        FuzzyGoal::Desirable
    );

    check(unknownA.status == FuzzyGoal::RuleUnknownCriterionA,
          "unknown first criterion returns RuleUnknownCriterionA");

    auto unknownB = goal.addRule(
        c1.id, FuzzyGoal::Desirable,
        9999, FuzzyGoal::Desirable,
        FuzzyGoal::And,
        FuzzyGoal::Desirable
    );

    check(unknownB.status == FuzzyGoal::RuleUnknownCriterionB,
          "unknown second criterion returns RuleUnknownCriterionB");

    auto invalidPNorm = goal.addRule(
        c1.id, FuzzyGoal::Desirable,
        c2.id, FuzzyGoal::Desirable,
        FuzzyGoal::And,
        FuzzyGoal::Desirable,
        FuzzyGoal::Aggregation::PNormAgg(2.0)
    );

    check(invalidPNorm.status == FuzzyGoal::RuleInvalidAggregation,
          "invalid PNorm parameter returns RuleInvalidAggregation");

    auto invalidSoftMinMax = goal.addRule(
        c1.id, FuzzyGoal::Desirable,
        c2.id, FuzzyGoal::Desirable,
        FuzzyGoal::And,
        FuzzyGoal::Desirable,
        FuzzyGoal::Aggregation::SoftMinMaxAgg(0.0)
    );

    check(invalidSoftMinMax.status == FuzzyGoal::RuleInvalidAggregation,
          "invalid SoftMinMax parameter returns RuleInvalidAggregation");
}

// ============================================================
// Evaluation input validation tests
// ============================================================

void testEvaluationStatus()
{
    std::cout << "\n=== Evaluation status ===\n";

    FuzzyGoal goal;

    auto c1 = goal.addCriterion(
        "c1",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    auto c2 = goal.addCriterion(
        "c2",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalLinearTol
    );

    double objective = 0.0;

    auto ok = goal.evaluate(
        {
            {c1.id, 0.25},
            {c2.id, 0.75}
        },
        objective
    );

    check(ok == FuzzyGoal::EvaluationOk,
          "valid evaluation returns EvaluationOk");

    auto missing = goal.evaluate(
        {
            {c1.id, 0.25}
        },
        objective
    );

    check(missing == FuzzyGoal::EvaluationMissingCriterion,
          "missing criterion returns EvaluationMissingCriterion");

    auto duplicate = goal.evaluate(
        {
            {c1.id, 0.25},
            {c1.id, 0.75}
        },
        objective
    );

    check(duplicate == FuzzyGoal::EvaluationDuplicateCriterion,
          "duplicate criterion returns EvaluationDuplicateCriterion");

    auto unknown = goal.evaluate(
        {
            {c1.id, 0.25},
            {9999, 0.75}
        },
        objective
    );

    check(unknown == FuzzyGoal::EvaluationUnknownCriterion,
          "unknown criterion returns EvaluationUnknownCriterion");

    FuzzyGoal emptyGoal;

    auto defuzzFail = emptyGoal.evaluate({}, objective);

    check(defuzzFail == FuzzyGoal::EvaluationDefuzzificationFailed,
          "empty objective returns EvaluationDefuzzificationFailed");
}

// ============================================================
// Single-criterion membership evaluation tests
// ============================================================

void testCriterionMembershipEvaluation()
{
    std::cout << "\n=== Criterion membership evaluation ===\n";

    FuzzyGoal goal;

    auto c = goal.addCriterion(
        "c",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    FuzzyGoal::CriterionMembership m;

    auto status = goal.evaluateCriterionMembership(c.id, 0.0, m);

    check(status == FuzzyGoal::EvaluationOk,
          "membership evaluation for valid criterion returns EvaluationOk");

    check(approx(m.desirable, 1.0),
          "membership at favorable boundary has desirable = 1");

    check(approx(m.tolerable, 0.0),
          "membership at favorable boundary has tolerable = 0");

    check(approx(m.undesirable, 0.0),
          "membership at favorable boundary has undesirable = 0");

    auto unknown = goal.evaluateCriterionMembership(9999, 0.0, m);

    check(unknown == FuzzyGoal::EvaluationUnknownCriterion,
          "membership evaluation for unknown criterion returns EvaluationUnknownCriterion");
}

// ============================================================
// Regression test for basic two-criteria example
// ============================================================

void testBasicTwoCriteriaRegression()
{
    std::cout << "\n=== Basic two-criteria regression ===\n";

    FuzzyGoal goal;

    auto c1 = goal.addCriterion(
        "c1",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    auto c2 = goal.addCriterion(
        "c2",
        0.0,
        1.0,
        0.5,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    check(c1.status == FuzzyGoal::CriterionOk,
          "regression criterion c1 created");

    check(c2.status == FuzzyGoal::CriterionOk,
          "regression criterion c2 created");

    const double xs[] = {0.0, 0.25, 0.5, 0.75, 1.0};

    for (double x : xs)
    {
        const double valueC1 = x;
        const double valueC2 = 1.0 - x;

        double objective = -1.0;

        auto status = goal.evaluate(
            {
                {c1.id, valueC1},
                {c2.id, valueC2}
            },
            objective
        );

        check(status == FuzzyGoal::EvaluationOk,
              "regression evaluation returns EvaluationOk");

        check(approx(objective, 0.5),
              "base two-criteria objective remains at neutral value 0.5");
    }
}

// ============================================================
// Equality constraint tests
// ============================================================

void testEqualityConstraint()
{
    std::cout << "\n=== Equality constraint ===\n";

    FuzzyGoal goal;

    auto eq = goal.addEqualityConstraint(
        "x_equals_0.5",
        0.5,
        0.05,
        FuzzyGoal::GaussianGlobalGaussianTol,
        0.15,
        10.0
    );

    check(eq.status == FuzzyGoal::CriterionOk,
          "valid equality constraint returns CriterionOk");

    double objective = 0.0;

    auto status = goal.evaluate(
        {
            {eq.id, 0.5}
        },
        objective
    );

    check(status == FuzzyGoal::EvaluationOk,
          "equality constraint evaluation returns EvaluationOk");

    check(objective >= 0.0 && objective <= 1.0,
          "equality constraint objective lies in [0, 1]");

    auto invalid = goal.addEqualityConstraint(
        "invalid_equality",
        0.5,
        -0.05,
        FuzzyGoal::GaussianGlobalGaussianTol,
        0.15,
        10.0
    );

    check(invalid.status == FuzzyGoal::InvalidInterval,
          "negative equality tolerance returns InvalidInterval");
}

// ============================================================
// Status string tests
// ============================================================

void testStatusStrings()
{
    std::cout << "\n=== Status strings ===\n";

    check(std::string(FuzzyGoal::statusToString(FuzzyGoal::CriterionOk)) ==
    "CriterionOk",
    "criterion status string is available");

    check(std::string(FuzzyGoal::statusToString(FuzzyGoal::RuleOk)) ==
    "RuleOk",
    "rule status string is available");

    check(std::string(FuzzyGoal::statusToString(FuzzyGoal::EvaluationOk)) ==
    "EvaluationOk",
    "evaluation status string is available");
}

// ============================================================
// Main
// ============================================================

int main()
{
    testDefuzzificationAnchorValues();
    testCriterionCreation();
    testAllMembershipFunctionsAreUsable();
    testGaussianRefGaussianTol();
    testRuleCreation();
    testEvaluationStatus();
    testCriterionMembershipEvaluation();
    testBasicTwoCriteriaRegression();
    testEqualityConstraint();
    testStatusStrings();

    std::cout << "\n========================================\n";

    if (failures == 0)
    {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
