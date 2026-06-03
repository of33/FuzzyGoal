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
    // Equality constraint example
    // ---------------------------------------------------------
    // Equality condition:
    //
    //     g(x) = x
    //     g*   = 0.5
    //
    // internally evaluated as
    //
    //     c = |x - 0.5|
    //
    // The tolerance defines the reference value of this
    // deviation criterion.
    // =========================================================

    const double target    = 0.5;
    const double tolerance = 0.05;
    const double gamma     = 10.0;

    // ---------------------------------------------------------
    // Equality only
    // ---------------------------------------------------------
    FuzzyGoal equalityOnly;

    auto eq = equalityOnly.addEqualityConstraint(
        "x_equals_0.5",
        target,
        tolerance,
        FuzzyGoal::GaussianGlobalGaussianTol,
        0.15,
        gamma
    );

    if (eq.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create equality constraint: "
                  << FuzzyGoal::statusToString(eq.status) << "\n";
        return 1;
    }

    // ---------------------------------------------------------
    // Equality plus inequality-like criterion:
    //
    //     x - x* <= 0, with x* = 0.5
    //
    // This is modeled as a fuzzy criterion where smaller values
    // are better and the reference value is x*.
    // ---------------------------------------------------------
    FuzzyGoal equalityPlusInequality;

    auto eq2 = equalityPlusInequality.addEqualityConstraint(
        "x_equals_0.5",
        target,
        tolerance,
        FuzzyGoal::GaussianGlobalGaussianTol,
        0.15,
        gamma
    );

    auto ineq = equalityPlusInequality.addCriterion(
        "x_less_equal_0.5",
        0.0,
        1.0,
        target,
        FuzzyGoal::SmallerIsBetter,
        FuzzyGoal::LinearGlobalParabolicTol,
        FuzzyGoal::LinearGuidance
    );

    if (eq2.status != FuzzyGoal::CriterionOk ||
        ineq.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create constraints.\n";
        return 1;
    }

    std::ofstream out("equality_constraint_example.dat");

    out << "# x f_equality f_equality_plus_inequality\n";
    out << std::fixed << std::setprecision(10);

    const int n = 400;

    for (int i = 0; i <= n; ++i)
    {
        const double x = static_cast<double>(i) / static_cast<double>(n);

        double fEquality = 0.0;
        double fCombined = 0.0;

        auto eqStatus = equalityOnly.evaluate({{eq.id, x}}, fEquality);

        if (eqStatus != FuzzyGoal::EvaluationOk)
        {
            std::cerr << "Equality-only evaluation failed: "
            << FuzzyGoal::statusToString(eqStatus) << "\n";
            return 1;
        }

        auto combinedStatus = equalityPlusInequality.evaluate(
            {{eq2.id, x}, {ineq.id, x}},
            fCombined
        );

        if (combinedStatus != FuzzyGoal::EvaluationOk)
        {
            std::cerr << "Combined evaluation failed: "
            << FuzzyGoal::statusToString(combinedStatus) << "\n";
            return 1;
        }

        out << x << " "
            << fEquality << " "
            << fCombined << "\n";
    }

    std::cout << "Wrote equality_constraint_example.dat\n";
    return 0;
}
