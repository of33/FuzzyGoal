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

#include <iostream>
#include <fstream>
#include <iomanip>

#include "fuzzygoal.h"

int main()
{
    // =========================================================
    // Minimal fuzzy-logic example
    // ---------------------------------------------------------
    // Two conflicting criteria:
    //
    //     c1 = x
    //     c2 = 1 - x
    //
    // Both criteria are to be minimized.
    // Therefore:
    //
    //     c1 is best near x = 0
    //     c2 is best near x = 1
    //
    // This creates a simple one-dimensional objective conflict.
    // =========================================================

    const double c0   = 0.0;
    const double c1   = 1.0;
    const double cref = 0.5;

    // ---------------------------------------------------------
    // Objective 1: pure classification, no explicit rules
    // ---------------------------------------------------------
    FuzzyGoal fuzzyBase;

    auto baseC1 = fuzzyBase.addCriterion(
        "c1",
        c0, c1, cref,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    auto baseC2 = fuzzyBase.addCriterion(
        "c2",
        c0, c1, cref,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    if (baseC1.status != FuzzyGoal::CriterionOk ||
        baseC2.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create criteria for base objective:\n"
                  << "  c1: " << FuzzyGoal::statusToString(baseC1.status) << "\n"
                  << "  c2: " << FuzzyGoal::statusToString(baseC2.status) << "\n";
        return 1;
    }

    // ---------------------------------------------------------
    // Objective 2: same criteria plus one explicit rule
    //
    // Rule:
    //   If c1 is undesirable OR c2 is undesirable,
    //   then the result is undesirable.
    //
    // This penalizes the extreme solutions:
    //
    //   x = 0 -> c2 = 1 -> undesirable
    //   x = 1 -> c1 = 1 -> undesirable
    //
    // Hence, the rule favors a compromise near the center.
    // ---------------------------------------------------------
    FuzzyGoal fuzzyRule;

    auto ruleC1 = fuzzyRule.addCriterion(
        "c1",
        c0, c1, cref,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    auto ruleC2 = fuzzyRule.addCriterion(
        "c2",
        c0, c1, cref,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearRefLinearTol
    );

    if (ruleC1.status != FuzzyGoal::CriterionOk ||
        ruleC2.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create criteria for rule objective:\n"
                  << "  c1: " << FuzzyGoal::statusToString(ruleC1.status) << "\n"
                  << "  c2: " << FuzzyGoal::statusToString(ruleC2.status) << "\n";
        return 1;
    }

    auto rule = fuzzyRule.addRule(
        ruleC1.id, FuzzyGoal::Undesirable,
        ruleC2.id, FuzzyGoal::Undesirable,
        FuzzyGoal::Or,
        FuzzyGoal::Undesirable
    );

    if (rule.status != FuzzyGoal::RuleOk)
    {
        std::cerr << "Failed to add fuzzy rule: "
        << FuzzyGoal::statusToString(rule.status) << "\n";
        return 1;
    }

    // ---------------------------------------------------------
    // Output
    // ---------------------------------------------------------
    std::ofstream out("basic_fuzzy_example.dat");

    out << "# x c1 c2 "
        << "c1_des c1_tol c1_undes "
        << "c2_des c2_tol c2_undes "
        << "f_base f_rule\n";

    out << std::fixed << std::setprecision(8);

    const int n = 100;

    for (int i = 0; i <= n; ++i)
    {
        const double x = static_cast<double>(i) / static_cast<double>(n);

        const double valueC1 = x;
        const double valueC2 = 1.0 - x;

        FuzzyGoal::CriterionMembership m1;
        FuzzyGoal::CriterionMembership m2;

        auto mStatus1 =
        fuzzyBase.evaluateCriterionMembership(baseC1.id, valueC1, m1);

        auto mStatus2 =
        fuzzyBase.evaluateCriterionMembership(baseC2.id, valueC2, m2);

        if (mStatus1 != FuzzyGoal::EvaluationOk ||
            mStatus2 != FuzzyGoal::EvaluationOk)
        {
            std::cerr << "Membership evaluation failed:\n"
            << "  c1: " << FuzzyGoal::statusToString(mStatus1) << "\n"
            << "  c2: " << FuzzyGoal::statusToString(mStatus2) << "\n";
            return 1;
        }

        double fBase = 0.0;

        auto baseStatus = fuzzyBase.evaluate(
            {{baseC1.id, valueC1}, {baseC2.id, valueC2}},
            fBase
        );

        if (baseStatus != FuzzyGoal::EvaluationOk)
        {
            std::cerr << "Base objective evaluation failed: "
            << FuzzyGoal::statusToString(baseStatus) << "\n";
            return 1;
        }

        double fRule = 0.0;

        auto ruleStatus = fuzzyRule.evaluate(
            {{ruleC1.id, valueC1}, {ruleC2.id, valueC2}},
            fRule
        );

        if (ruleStatus != FuzzyGoal::EvaluationOk)
        {
            std::cerr << "Rule objective evaluation failed: "
            << FuzzyGoal::statusToString(ruleStatus) << "\n";
            return 1;
        }

        out << x << " "
            << valueC1 << " "
            << valueC2 << " "
            << m1.desirable << " "
            << m1.tolerable << " "
            << m1.undesirable << " "
            << m2.desirable << " "
            << m2.tolerable << " "
            << m2.undesirable << " "
            << fBase << " "
            << fRule << "\n";
    }

    std::cout << "Wrote basic_fuzzy_example.dat\n";
    return 0;
}
