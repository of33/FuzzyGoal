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
#include <cmath>
#include <iomanip>

#include "fuzzygoal.h"

int main()
{
    const double Lambda0 = 4.0;
    const double Lambda1 = 16.0;

    const double t0 = 0.1;
    const double t1 = 0.2;

    const int N_L = 40;
    const int N_t = 40;

    const double c1_ref = 0.5;
    const double c2_ref = 7.0 / 27.0;

    const auto membership = FuzzyGoal::ParabolicGlobalParabolicTol;
    const auto boundary   = FuzzyGoal::LinearGuidance;
    const auto agg        = FuzzyGoal::Aggregation::PNormAgg(-30.0);

    // =========================================================
    // Objective 1
    //
    // Rule 1:
    //   If c1 is desirable AND c2 is desirable,
    //   then the result is desirable.
    // =========================================================
    FuzzyGoal fg1;

    auto fg1_c1 = fg1.addCriterion(
        "c1", 0.0, 1.0, c1_ref,
        FuzzyGoal::SmallerIsBetter,
        membership,
        boundary
    );

    auto fg1_c2 = fg1.addCriterion(
        "c2", 0.0, 1.0, c2_ref,
        FuzzyGoal::SmallerIsBetter,
        membership,
        boundary
    );

    if (fg1_c1.status != FuzzyGoal::CriterionOk ||
        fg1_c2.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create criteria for fg1.\n";
        return 1;
    }

    auto fg1_rule = fg1.addRule(
        fg1_c1.id, FuzzyGoal::Desirable,
        fg1_c2.id, FuzzyGoal::Desirable,
        FuzzyGoal::And,
        FuzzyGoal::Desirable,
        agg
    );

    if (fg1_rule.status != FuzzyGoal::RuleOk)
    {
        std::cerr << "Failed to add rule for fg1: "
        << FuzzyGoal::statusToString(fg1_rule.status) << "\n";
        return 1;
    }

    // =========================================================
    // Objective 2
    //
    // Rule 2:
    //   If c1 is desirable AND c2 is tolerable,
    //   then the result is tolerable.
    // =========================================================
    FuzzyGoal fg2;

    auto fg2_c1 = fg2.addCriterion(
        "c1", 0.0, 1.0, c1_ref,
        FuzzyGoal::SmallerIsBetter,
        membership,
        boundary
    );

    auto fg2_c2 = fg2.addCriterion(
        "c2", 0.0, 1.0, c2_ref,
        FuzzyGoal::SmallerIsBetter,
        membership,
        boundary
    );

    if (fg2_c1.status != FuzzyGoal::CriterionOk ||
        fg2_c2.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create criteria for fg2.\n";
        return 1;
    }

    auto fg2_rule = fg2.addRule(
        fg2_c1.id, FuzzyGoal::Desirable,
        fg2_c2.id, FuzzyGoal::Tolerable,
        FuzzyGoal::And,
        FuzzyGoal::Tolerable,
        agg
    );

    if (fg2_rule.status != FuzzyGoal::RuleOk)
    {
        std::cerr << "Failed to add rule for fg2: "
        << FuzzyGoal::statusToString(fg2_rule.status) << "\n";
        return 1;
    }

    // =========================================================
    // Objective 3
    //
    // Rule 3:
    //   If c1 is desirable OR c2 is undesirable,
    //   then the result is undesirable.
    //
    // This is intentionally a problematic preference formulation.
    // =========================================================
    FuzzyGoal fg3;

    auto fg3_c1 = fg3.addCriterion(
        "c1", 0.0, 1.0, c1_ref,
        FuzzyGoal::SmallerIsBetter,
        membership,
        boundary
    );

    auto fg3_c2 = fg3.addCriterion(
        "c2", 0.0, 1.0, c2_ref,
        FuzzyGoal::SmallerIsBetter,
        membership,
        boundary
    );

    if (fg3_c1.status != FuzzyGoal::CriterionOk ||
        fg3_c2.status != FuzzyGoal::CriterionOk)
    {
        std::cerr << "Failed to create criteria for fg3.\n";
        return 1;
    }

    auto fg3_rule = fg3.addRule(
        fg3_c1.id, FuzzyGoal::Desirable,
        fg3_c2.id, FuzzyGoal::Undesirable,
        FuzzyGoal::Or,
        FuzzyGoal::Undesirable,
        agg
    );

    if (fg3_rule.status != FuzzyGoal::RuleOk)
    {
        std::cerr << "Failed to add rule for fg3: "
        << FuzzyGoal::statusToString(fg3_rule.status) << "\n";
        return 1;
    }

    std::ofstream out("c1_c2_topography_fuzzy.dat");

    out << "# Lambda tbar c1 c2 f_rule1 f_rule2 f_rule3\n";
    out << std::fixed << std::setprecision(10);

    for (int i = 0; i <= N_L; ++i)
    {
        const double Lambda =
            Lambda0 + (Lambda1 - Lambda0) * i / static_cast<double>(N_L);

        for (int j = 0; j <= N_t; ++j)
        {
            const double tbar =
                t0 + (t1 - t0) * j / static_cast<double>(N_t);

            // -----------------------------------------------------
            // Extended Test case 1 criteria
            //
            // c1 ~ Lambda / tbar
            //
            // c2 = 0.5 * (parasite drag part + induced drag part)
            //
            // parasite drag part ~ sqrt(1 + tbar^2)
            // induced drag part  ~ 1 / Lambda
            // -----------------------------------------------------
            const double c1 =
                (Lambda / tbar - Lambda0 / t1)
              / (Lambda1 / t0 - Lambda0 / t1);

            const double term_t =
                (std::sqrt(1.0 + tbar * tbar) - std::sqrt(1.0 + t0 * t0))
              / (std::sqrt(1.0 + t1   * t1)   - std::sqrt(1.0 + t0 * t0));

            const double term_L =
                (1.0 / Lambda - 1.0 / Lambda1)
              / (1.0 / Lambda0 - 1.0 / Lambda1);

            const double c2 = 0.5 * (term_t + term_L);

            double f1 = 0.0;
            double f2 = 0.0;
            double f3 = 0.0;

            auto eval1 = fg1.evaluate({{fg1_c1.id, c1}, {fg1_c2.id, c2}}, f1);
            auto eval2 = fg2.evaluate({{fg2_c1.id, c1}, {fg2_c2.id, c2}}, f2);
            auto eval3 = fg3.evaluate({{fg3_c1.id, c1}, {fg3_c2.id, c2}}, f3);

            if (eval1 != FuzzyGoal::EvaluationOk ||
                eval2 != FuzzyGoal::EvaluationOk ||
                eval3 != FuzzyGoal::EvaluationOk)
            {
                std::cerr << "Evaluation failed at Lambda = "
                << Lambda << ", tbar = " << tbar << "\n"
                << "  fg1: " << FuzzyGoal::statusToString(eval1) << "\n"
                << "  fg2: " << FuzzyGoal::statusToString(eval2) << "\n"
                << "  fg3: " << FuzzyGoal::statusToString(eval3) << "\n";
                return 1;
            }

            out << Lambda << " "
                << tbar << " "
                << c1 << " "
                << c2 << " "
                << f1 << " "
                << f2 << " "
                << f3 << "\n";
        }

        out << "\n";
    }

    std::cout << "Wrote c1_c2_topography_fuzzy.dat\n";
    return 0;
}
