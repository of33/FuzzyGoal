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

#include "fuzzygoal.h"

#include <cmath>
#include <algorithm>

namespace
{
    double clamp01(double x)
    {
        if (x < 0.0) return 0.0;
        if (x > 1.0) return 1.0;
        return x;
    }

    bool usesGaussian(FuzzyGoal::MembershipFunction mf)
    {
        return mf == FuzzyGoal::LinearGlobalGaussianTol ||
        mf == FuzzyGoal::GaussianRefGaussianTol ||
        mf == FuzzyGoal::GaussianGlobalGaussianTol;
    }

    FuzzyGoal::AddCriterionStatus validateCriterionInput(
        double c0,
        double c1,
        double cRef,
        bool checkSigma,
        double sigmaRel
    )
    {
        if (!(c1 > c0))
            return FuzzyGoal::InvalidInterval;

        if (cRef < c0)
            return FuzzyGoal::RefBelowInterval;

        if (cRef > c1)
            return FuzzyGoal::RefAboveInterval;

        const double length = c1 - c0;
        const double eps = 1e-12 * std::max(1.0, std::abs(length));

        if ((cRef - c0) <= eps)
            return FuzzyGoal::RefTooCloseToLowerBound;

        if ((c1 - cRef) <= eps)
            return FuzzyGoal::RefTooCloseToUpperBound;

        if (checkSigma && sigmaRel <= 0.0)
            return FuzzyGoal::InvalidSigma;

        return FuzzyGoal::CriterionOk;
    }

    bool validateAggregation(const FuzzyGoal::Aggregation& agg)
    {
        if (agg.type == FuzzyGoal::SoftMinMax && agg.parameter <= 0.0)
            return false;

        if (agg.type == FuzzyGoal::PNorm && agg.parameter >= 0.0)
            return false;

        return true;
    }

    double aggregateMemberships(
        double a,
        double b,
        FuzzyGoal::RuleOperator logicOp,
        const FuzzyGoal::Aggregation& agg
    )
    {
        a = clamp01(a);
        b = clamp01(b);

        switch (agg.type)
        {
            case FuzzyGoal::MinMax:
                return (logicOp == FuzzyGoal::And)
                    ? std::min(a, b)
                    : std::max(a, b);

            case FuzzyGoal::Product:
                return (logicOp == FuzzyGoal::And)
                    ? (a * b)
                    : (a + b - a * b);

            case FuzzyGoal::SoftMinMax:
            {
                const double eps = agg.parameter;
                double val = 0.0;

                if (logicOp == FuzzyGoal::And)
                    val = 0.5 * (a + b - std::sqrt((a - b) * (a - b) + eps));
                else
                    val = 0.5 * (a + b + std::sqrt((a - b) * (a - b) + eps));

                return clamp01(val);
            }

            case FuzzyGoal::PNorm:
            {
                const double p = agg.parameter;
                const double eps = 1e-12;

                const double aa = (a < eps) ? eps : a;
                const double bb = (b < eps) ? eps : b;

                if (logicOp == FuzzyGoal::And)
                {
                    const double val =
                        std::pow(std::pow(aa, p) + std::pow(bb, p), 1.0 / p);

                    return clamp01(val);
                }
                else
                {
                    const double q = -p;

                    const double val =
                        std::pow(std::pow(aa, q) + std::pow(bb, q), 1.0 / q);

                    return clamp01(val);
                }
            }
        }

        return 0.0;
    }
}

// =============================================================
// Status strings
// =============================================================

const char* FuzzyGoal::statusToString(AddCriterionStatus status)
{
    switch (status)
    {
        case CriterionOk:
            return "CriterionOk";
        case InvalidInterval:
            return "InvalidInterval";
        case RefBelowInterval:
            return "RefBelowInterval";
        case RefAboveInterval:
            return "RefAboveInterval";
        case RefTooCloseToLowerBound:
            return "RefTooCloseToLowerBound";
        case RefTooCloseToUpperBound:
            return "RefTooCloseToUpperBound";
        case InvalidSigma:
            return "InvalidSigma";
    }

    return "UnknownStatus";
}

const char* FuzzyGoal::statusToString(AddRuleStatus status)
{
    switch (status)
    {
        case RuleOk:
            return "RuleOk";
        case RuleUnknownCriterionA:
            return "RuleUnknownCriterionA";
        case RuleUnknownCriterionB:
            return "RuleUnknownCriterionB";
        case RuleInvalidAggregation:
            return "RuleInvalidAggregation";
    }

    return "UnknownRuleStatus";
}

const char* FuzzyGoal::statusToString(EvaluationStatus status)
{
    switch (status)
    {
        case EvaluationOk:
            return "EvaluationOk";
        case EvaluationUnknownCriterion:
            return "EvaluationUnknownCriterion";
        case EvaluationMissingCriterion:
            return "EvaluationMissingCriterion";
        case EvaluationDuplicateCriterion:
            return "EvaluationDuplicateCriterion";
        case EvaluationDefuzzificationFailed:
            return "EvaluationDefuzzificationFailed";
    }

    return "UnknownEvaluationStatus";
}

// =============================================================
// FuzzyGoal
// =============================================================

FuzzyGoal::FuzzyGoal()
: nextCriterionId(0),
  nextRuleId(0)
{}

// =============================================================
// SingleCriterion construction
// =============================================================

FuzzyGoal::SingleCriterion::SingleCriterion(
    double c0_,
    double c1_,
    double cRef_,
    Direction direction_,
    MembershipFunction mf_,
    BoundaryBehavior bb_
)
: c0(c0_),
  c1(c1_),
  cRef(cRef_),
  direction(direction_),
  mf(mf_),
  boundaryBehavior(bb_),
  sigmaRel(0.0),
  sigmaTolLeft(0.0),
  sigmaTolRight(0.0),
  sigmaGlob(0.0)
{}

FuzzyGoal::SingleCriterion::SingleCriterion(
    double c0_,
    double c1_,
    double cRef_,
    Direction direction_,
    MembershipFunction mf_,
    BoundaryBehavior bb_,
    double sigmaRel_
)
: c0(c0_),
  c1(c1_),
  cRef(cRef_),
  direction(direction_),
  mf(mf_),
  boundaryBehavior(bb_),
  sigmaRel(sigmaRel_)
{
    const double alpha = 4.0;

    sigmaTolLeft  = alpha * sigmaRel * (cRef - c0);
    sigmaTolRight = alpha * sigmaRel * (c1 - cRef);
    sigmaGlob     = alpha * sigmaRel * (c1 - c0);
}

// =============================================================
// Membership primitives
// =============================================================

double FuzzyGoal::SingleCriterion::linearDesirableRef(double c) const
{
    if (direction == SmallerIsBetter)
        return (-1.0 / (cRef - c0)) * (c - cRef);

    return (1.0 / (c1 - cRef)) * (c - cRef);
}

double FuzzyGoal::SingleCriterion::linearUndesirableRef(double c) const
{
    if (direction == SmallerIsBetter)
        return (1.0 / (c1 - cRef)) * (c - cRef);

    return (-1.0 / (cRef - c0)) * (c - cRef);
}

double FuzzyGoal::SingleCriterion::linearDesirableGlobal(double c) const
{
    if (direction == SmallerIsBetter)
        return (c1 - c) / (c1 - c0);

    return (c - c0) / (c1 - c0);
}

double FuzzyGoal::SingleCriterion::linearUndesirableGlobal(double c) const
{
    if (direction == SmallerIsBetter)
        return (c - c0) / (c1 - c0);

    return (c1 - c) / (c1 - c0);
}

double FuzzyGoal::SingleCriterion::linearTolerableLeft(double c) const
{
    return (c - c0) / (cRef - c0);
}

double FuzzyGoal::SingleCriterion::linearTolerableRight(double c) const
{
    return -(c - c1) / (c1 - cRef);
}

double FuzzyGoal::SingleCriterion::parabolicDesirableRef(double c) const
{
    if (direction == SmallerIsBetter)
    {
        if (c > cRef) return 0.0;

        const double x = (c - cRef) / (c0 - cRef);
        return x * x;
    }
    else
    {
        if (c < cRef) return 0.0;

        const double x = (c - cRef) / (c1 - cRef);
        return x * x;
    }
}

double FuzzyGoal::SingleCriterion::parabolicUndesirableRef(double c) const
{
    if (direction == SmallerIsBetter)
    {
        if (c < cRef) return 0.0;

        const double x = (c - cRef) / (c1 - cRef);
        return x * x;
    }
    else
    {
        if (c > cRef) return 0.0;

        const double x = (c - cRef) / (c0 - cRef);
        return x * x;
    }
}

double FuzzyGoal::SingleCriterion::parabolicDesirableGlobal(double c) const
{
    double x = 0.0;

    if (direction == SmallerIsBetter)
        x = (c - c1) / (c0 - c1);
    else
        x = (c - c0) / (c1 - c0);

    return x * x;
}

double FuzzyGoal::SingleCriterion::parabolicUndesirableGlobal(double c) const
{
    double x = 0.0;

    if (direction == SmallerIsBetter)
        x = (c - c0) / (c1 - c0);
    else
        x = (c - c1) / (c0 - c1);

    return x * x;
}

double FuzzyGoal::SingleCriterion::parabolicTolerableLeft(double c) const
{
    const double x = (cRef - c) / (cRef - c0);
    return 1.0 - x * x;
}

double FuzzyGoal::SingleCriterion::parabolicTolerableRight(double c) const
{
    const double x = (cRef - c) / (cRef - c1);
    return 1.0 - x * x;
}

double FuzzyGoal::SingleCriterion::gaussianDesirableGlobal(double c) const
{
    const double d  = (direction == SmallerIsBetter) ? (c - c0) : (c1 - c);
    const double d1 = c1 - c0;

    const double g  = std::exp(-(d  * d ) / (2.0 * sigmaGlob * sigmaGlob));
    const double g1 = std::exp(-(d1 * d1) / (2.0 * sigmaGlob * sigmaGlob));

    return (g - g1) / (1.0 - g1);
}

double FuzzyGoal::SingleCriterion::gaussianUndesirableGlobal(double c) const
{
    const double d  = (direction == SmallerIsBetter) ? (c1 - c) : (c - c0);
    const double d0 = c1 - c0;

    const double g  = std::exp(-(d  * d ) / (2.0 * sigmaGlob * sigmaGlob));
    const double g0 = std::exp(-(d0 * d0) / (2.0 * sigmaGlob * sigmaGlob));

    return (g - g0) / (1.0 - g0);
}

double FuzzyGoal::SingleCriterion::gaussianTolerableLeft(double c) const
{
    const double g =
        std::exp(-(c - cRef) * (c - cRef) /
                 (2.0 * sigmaTolLeft * sigmaTolLeft));

    const double g0 =
        std::exp(-(c0 - cRef) * (c0 - cRef) /
                 (2.0 * sigmaTolLeft * sigmaTolLeft));

    return (g - g0) / (1.0 - g0);
}

double FuzzyGoal::SingleCriterion::gaussianTolerableRight(double c) const
{
    const double g =
        std::exp(-(c - cRef) * (c - cRef) /
                 (2.0 * sigmaTolRight * sigmaTolRight));

    const double g1 =
        std::exp(-(c1 - cRef) * (c1 - cRef) /
                 (2.0 * sigmaTolRight * sigmaTolRight));

    return (g - g1) / (1.0 - g1);
}

// =============================================================
// Composite membership variants
// =============================================================

void FuzzyGoal::SingleCriterion::computeLinearDesUndesRef_LinearTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = linearDesirableRef(c);
    mu = linearUndesirableRef(c);
    mt = (c <= cRef) ? linearTolerableLeft(c) : linearTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeLinearDesUndesRef_ParabolicTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = linearDesirableRef(c);
    mu = linearUndesirableRef(c);
    mt = (c <= cRef) ? parabolicTolerableLeft(c) : parabolicTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeLinearDesUndesGlobal_LinearTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = linearDesirableGlobal(c);
    mu = linearUndesirableGlobal(c);
    mt = (c <= cRef) ? linearTolerableLeft(c) : linearTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeLinearDesUndesGlobal_ParabolicTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = linearDesirableGlobal(c);
    mu = linearUndesirableGlobal(c);
    mt = (c <= cRef) ? parabolicTolerableLeft(c) : parabolicTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeParabolicDesUndesRef_ParabolicTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = parabolicDesirableRef(c);
    mu = parabolicUndesirableRef(c);
    mt = (c <= cRef) ? parabolicTolerableLeft(c) : parabolicTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeParabolicDesUndesGlobal_ParabolicTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = parabolicDesirableGlobal(c);
    mu = parabolicUndesirableGlobal(c);
    mt = (c <= cRef) ? parabolicTolerableLeft(c) : parabolicTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeLinearDesUndesGlobal_GaussianTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = linearDesirableGlobal(c);
    mu = linearUndesirableGlobal(c);
    mt = (c <= cRef) ? gaussianTolerableLeft(c) : gaussianTolerableRight(c);
}

void FuzzyGoal::SingleCriterion::computeGaussianDesUndesRef_GaussianTolRef(
    double c,
    double& md,
    double& mt,
    double& mu
) const
{
    if (c <= cRef)
    {
        mt = gaussianTolerableLeft(c);

        if (direction == SmallerIsBetter)
        {
            md = 1.0 - mt;
            mu = 0.0;
        }
        else
        {
            md = 0.0;
            mu = 1.0 - mt;
        }
    }
    else
    {
        mt = gaussianTolerableRight(c);

        if (direction == SmallerIsBetter)
        {
            md = 0.0;
            mu = 1.0 - mt;
        }
        else
        {
            md = 1.0 - mt;
            mu = 0.0;
        }
    }
}

void FuzzyGoal::SingleCriterion::computeGaussianDesUndesGlobal_GaussianTolRef(
    double c, double& md, double& mt, double& mu) const
{
    md = gaussianDesirableGlobal(c);
    mu = gaussianUndesirableGlobal(c);
    mt = (c <= cRef) ? gaussianTolerableLeft(c) : gaussianTolerableRight(c);
}

// =============================================================
// Membership evaluation
// =============================================================

void FuzzyGoal::SingleCriterion::evaluateMemberships(
    double c,
    double& md,
    double& mt,
    double& mu
) const
{
    if (direction == SmallerIsBetter)
    {
        if (c < c0)
        {
            if (boundaryBehavior == HardSaturation)
            {
                md = 1.0;
            }
            else
            {
                const double beta = 1.0 / (3.0 * (c1 - c0));
                md = 1.0 - beta * (c0 - c);
                if (md < 0.0) md = 0.0;
            }

            mt = 0.0;
            mu = 0.0;
            return;
        }

        if (c > c1)
        {
            md = 0.0;
            mt = 0.0;
            mu = 1.0;
            return;
        }
    }
    else
    {
        if (c > c1)
        {
            if (boundaryBehavior == HardSaturation)
            {
                md = 1.0;
            }
            else
            {
                const double beta = 1.0 / (3.0 * (c1 - c0));
                md = 1.0 - beta * (c - c1);
                if (md < 0.0) md = 0.0;
            }

            mt = 0.0;
            mu = 0.0;
            return;
        }

        if (c < c0)
        {
            md = 0.0;
            mt = 0.0;
            mu = 1.0;
            return;
        }
    }

    switch (mf)
    {
        case LinearRefLinearTol:
            computeLinearDesUndesRef_LinearTolRef(c, md, mt, mu);
            break;

        case LinearRefParabolicTol:
            computeLinearDesUndesRef_ParabolicTolRef(c, md, mt, mu);
            break;

        case LinearGlobalLinearTol:
            computeLinearDesUndesGlobal_LinearTolRef(c, md, mt, mu);
            break;

        case LinearGlobalParabolicTol:
            computeLinearDesUndesGlobal_ParabolicTolRef(c, md, mt, mu);
            break;

        case ParabolicRefParabolicTol:
            computeParabolicDesUndesRef_ParabolicTolRef(c, md, mt, mu);
            break;

        case ParabolicGlobalParabolicTol:
            computeParabolicDesUndesGlobal_ParabolicTolRef(c, md, mt, mu);
            break;

        case LinearGlobalGaussianTol:
            computeLinearDesUndesGlobal_GaussianTolRef(c, md, mt, mu);
            break;

        case GaussianRefGaussianTol:
            computeGaussianDesUndesRef_GaussianTolRef(c, md, mt, mu);
            break;

        case GaussianGlobalGaussianTol:
            computeGaussianDesUndesGlobal_GaussianTolRef(c, md, mt, mu);
            break;

        default:
            md = mt = mu = 0.0;
            break;
    }

    md = clamp01(md);
    mt = clamp01(mt);
    mu = clamp01(mu);
}

// =============================================================
// Rule
// =============================================================

FuzzyGoal::Rule::Rule(
    int a,
    RuleMembership ma,
    int b,
    RuleMembership mb,
    RuleOperator op_,
    RuleMembership out,
    Aggregation agg
)
: critA(a),
  critB(b),
  memA(ma),
  memB(mb),
  op(op_),
  outcome(out),
  aggregation(agg)
{}

double FuzzyGoal::Rule::evaluate(double muA, double muB) const
{
    return aggregateMemberships(muA, muB, op, aggregation);
}

// =============================================================
// Defuzzifier
// =============================================================

FuzzyGoal::Defuzzifier::Defuzzifier()
: sd(1.0 / 6.0),
  st(1.0 / 2.0),
  su(5.0 / 6.0)
{}

bool FuzzyGoal::Defuzzifier::defuzzify(
    const FuzzyInferenceResult& inf,
    double& value
) const
{
    const double sum = inf.desirable + inf.tolerable + inf.undesirable;

    if (sum <= 0.0)
        return false;

    const double weighted =
        sd * inf.desirable +
        st * inf.tolerable +
        su * inf.undesirable;

    value = 1.5 * (weighted / sum) - 0.25;
    value = clamp01(value);

    return true;
}

// =============================================================
// Criterion creation
// =============================================================

FuzzyGoal::NewCriterion FuzzyGoal::addCriterion(
    const std::string& name,
    double c0,
    double c1,
    double cRef,
    Direction direction,
    MembershipFunction mf
)
{
    if (usesGaussian(mf))
    {
        return addCriterion(
            name,
            c0,
            c1,
            cRef,
            direction,
            mf,
            HardSaturation,
            0.15
        );
    }

    return addCriterion(
        name,
        c0,
        c1,
        cRef,
        direction,
        mf,
        HardSaturation
    );
}

FuzzyGoal::NewCriterion FuzzyGoal::addCriterion(
    const std::string& name,
    double c0,
    double c1,
    double cRef,
    Direction direction,
    MembershipFunction mf,
    double sigmaRel
)
{
    return addCriterion(
        name,
        c0,
        c1,
        cRef,
        direction,
        mf,
        HardSaturation,
        sigmaRel
    );
}

FuzzyGoal::NewCriterion FuzzyGoal::addCriterion(
    const std::string& name,
    double c0,
    double c1,
    double cRef,
    Direction direction,
    MembershipFunction mf,
    BoundaryBehavior bb
)
{
    const AddCriterionStatus status =
        validateCriterionInput(c0, c1, cRef, false, 0.0);

    if (status != CriterionOk)
        return {status, -1};

    if (usesGaussian(mf))
    {
        return addCriterion(
            name,
            c0,
            c1,
            cRef,
            direction,
            mf,
            bb,
            0.15
        );
    }

    const int id = nextCriterionId++;

    criteria.emplace_back(
        id,
        name,
        SingleCriterion(c0, c1, cRef, direction, mf, bb),
        false,
        0.0
    );

    return {CriterionOk, id};
}

FuzzyGoal::NewCriterion FuzzyGoal::addCriterion(
    const std::string& name,
    double c0,
    double c1,
    double cRef,
    Direction direction,
    MembershipFunction mf,
    BoundaryBehavior bb,
    double sigmaRel
)
{
    const AddCriterionStatus status =
        validateCriterionInput(c0, c1, cRef, usesGaussian(mf), sigmaRel);

    if (status != CriterionOk)
        return {status, -1};

    const int id = nextCriterionId++;

    criteria.emplace_back(
        id,
        name,
        SingleCriterion(c0, c1, cRef, direction, mf, bb, sigmaRel),
        false,
        0.0
    );

    return {CriterionOk, id};
}

// =============================================================
// Equality constraints
// =============================================================

FuzzyGoal::NewCriterion FuzzyGoal::addEqualityConstraint(
    const std::string& name,
    double targetValue,
    double tolerance,
    MembershipFunction mf,
    double sigmaRel
)
{
    return addEqualityConstraint(
        name,
        targetValue,
        tolerance,
        mf,
        sigmaRel,
        10.0
    );
}

FuzzyGoal::NewCriterion FuzzyGoal::addEqualityConstraint(
    const std::string& name,
    double targetValue,
    double tolerance,
    MembershipFunction mf,
    double sigmaRel,
    double gamma
)
{
    if (tolerance <= 0.0 || gamma <= 1.0)
        return {InvalidInterval, -1};

    const double c0   = 0.0;
    const double cRef = tolerance;
    const double c1   = gamma * tolerance;

    const AddCriterionStatus status =
        validateCriterionInput(c0, c1, cRef, usesGaussian(mf), sigmaRel);

    if (status != CriterionOk)
        return {status, -1};

    const int id = nextCriterionId++;

    criteria.emplace_back(
        id,
        name,
        SingleCriterion(
            c0,
            c1,
            cRef,
            SmallerIsBetter,
            mf,
            HardSaturation,
            sigmaRel
        ),
        true,
        targetValue
    );

    return {CriterionOk, id};
}

FuzzyGoal::NewCriterion FuzzyGoal::addEqualityConstraint(
    const std::string& name,
    double tolerance,
    MembershipFunction mf,
    double sigmaRel
)
{
    return addEqualityConstraint(
        name,
        0.0,
        tolerance,
        mf,
        sigmaRel,
        10.0
    );
}

FuzzyGoal::NewCriterion FuzzyGoal::addEqualityConstraint(
    const std::string& name,
    double tolerance,
    MembershipFunction mf,
    double sigmaRel,
    double gamma
)
{
    return addEqualityConstraint(
        name,
        0.0,
        tolerance,
        mf,
        sigmaRel,
        gamma
    );
}

void FuzzyGoal::removeCriterion(int criterionId)
{
    for (auto it = criteria.begin(); it != criteria.end(); )
    {
        if (it->id == criterionId)
            it = criteria.erase(it);
        else
            ++it;
    }

    for (auto it = rules.begin(); it != rules.end(); )
    {
        if (it->rule.critA == criterionId ||
            it->rule.critB == criterionId)
            it = rules.erase(it);
        else
            ++it;
    }
}

// =============================================================
// Rule handling
// =============================================================

FuzzyGoal::NewRule FuzzyGoal::addRule(
    int criterionA,
    RuleMembership memA,
    int criterionB,
    RuleMembership memB,
    RuleOperator op,
    RuleMembership outcome
)
{
    return addRule(
        criterionA,
        memA,
        criterionB,
        memB,
        op,
        outcome,
        Aggregation::MinMaxAgg()
    );
}

FuzzyGoal::NewRule FuzzyGoal::addRule(
    int criterionA,
    RuleMembership memA,
    int criterionB,
    RuleMembership memB,
    RuleOperator op,
    RuleMembership outcome,
    Aggregation aggregation
)
{
    if (!findCriterion(criterionA))
        return {RuleUnknownCriterionA, -1};

    if (!findCriterion(criterionB))
        return {RuleUnknownCriterionB, -1};

    if (!validateAggregation(aggregation))
        return {RuleInvalidAggregation, -1};

    const int id = nextRuleId++;

    Rule rule(
        criterionA,
        memA,
        criterionB,
        memB,
        op,
        outcome,
        aggregation
    );

    rules.emplace_back(id, rule);

    return {RuleOk, id};
}

void FuzzyGoal::removeRule(int ruleId)
{
    for (auto it = rules.begin(); it != rules.end(); )
    {
        if (it->id == ruleId)
            it = rules.erase(it);
        else
            ++it;
    }
}

// =============================================================
// Internal helpers
// =============================================================

const FuzzyGoal::CriterionEntry*
FuzzyGoal::findCriterion(int id) const
{
    for (const CriterionEntry& e : criteria)
    {
        if (e.id == id)
            return &e;
    }

    return nullptr;
}

double FuzzyGoal::transformCriterionValue(
    const CriterionEntry& criterion,
    double rawValue
) const
{
    if (criterion.isEquality)
        return std::abs(rawValue - criterion.targetValue);

    return rawValue;
}

FuzzyGoal::EvaluationStatus FuzzyGoal::validateEvaluationInput(
    const std::vector<std::pair<int,double>>& values
) const
{
    // Unknown criterion IDs.
    for (const auto& v : values)
    {
        if (!findCriterion(v.first))
            return EvaluationUnknownCriterion;
    }

    // Missing or duplicated registered criteria.
    for (const CriterionEntry& c : criteria)
    {
        int count = 0;

        for (const auto& v : values)
        {
            if (v.first == c.id)
                ++count;
        }

        if (count == 0)
            return EvaluationMissingCriterion;

        if (count > 1)
            return EvaluationDuplicateCriterion;
    }

    return EvaluationOk;
}

// =============================================================
// Inference
// =============================================================

FuzzyGoal::FuzzyInferenceResult
FuzzyGoal::inference(
    const std::vector<std::pair<int,double>>& values
) const
{
    FuzzyInferenceResult result;

    // ---------------------------------------------------------
    // Implicit single-criterion rules
    // ---------------------------------------------------------
    for (const auto& v : values)
    {
        const CriterionEntry* ce = findCriterion(v.first);

        if (!ce)
            continue;

        const double value = transformCriterionValue(*ce, v.second);

        double md = 0.0;
        double mt = 0.0;
        double mu = 0.0;

        ce->criterion.evaluateMemberships(value, md, mt, mu);

        result.desirable   += md;
        result.tolerable   += mt;
        result.undesirable += mu;
    }

    // ---------------------------------------------------------
    // Explicit rules
    // ---------------------------------------------------------
    for (const RuleEntry& r : rules)
    {
        double muA = 0.0;
        double muB = 0.0;

        bool foundA = false;
        bool foundB = false;

        for (const auto& v : values)
        {
            if (v.first == r.rule.critA)
            {
                const CriterionEntry* ce = findCriterion(v.first);

                if (!ce)
                    continue;

                const double value = transformCriterionValue(*ce, v.second);

                double md = 0.0;
                double mt = 0.0;
                double mu = 0.0;

                ce->criterion.evaluateMemberships(value, md, mt, mu);

                muA =
                    (r.rule.memA == Desirable)   ? md :
                    (r.rule.memA == Tolerable)   ? mt :
                                                    mu;

                foundA = true;
            }

            if (v.first == r.rule.critB)
            {
                const CriterionEntry* ce = findCriterion(v.first);

                if (!ce)
                    continue;

                const double value = transformCriterionValue(*ce, v.second);

                double md = 0.0;
                double mt = 0.0;
                double mu = 0.0;

                ce->criterion.evaluateMemberships(value, md, mt, mu);

                muB =
                    (r.rule.memB == Desirable)   ? md :
                    (r.rule.memB == Tolerable)   ? mt :
                                                    mu;

                foundB = true;
            }
        }

        if (!foundA || !foundB)
            continue;

        const double muRule = r.rule.evaluate(muA, muB);

        if (r.rule.outcome == Desirable)
            result.desirable += muRule;
        else if (r.rule.outcome == Tolerable)
            result.tolerable += muRule;
        else
            result.undesirable += muRule;
    }

    return result;
}

// =============================================================
// Evaluation
// =============================================================

FuzzyGoal::EvaluationStatus FuzzyGoal::evaluateCriterionMembership(
    int criterionId,
    double value,
    CriterionMembership& membership
) const
{
    const CriterionEntry* ce = findCriterion(criterionId);

    if (!ce)
        return EvaluationUnknownCriterion;

    const double transformedValue = transformCriterionValue(*ce, value);

    ce->criterion.evaluateMemberships(
        transformedValue,
        membership.desirable,
        membership.tolerable,
        membership.undesirable
    );

    return EvaluationOk;
}

FuzzyGoal::EvaluationStatus FuzzyGoal::evaluate(
    const std::vector<std::pair<int,double>>& values,
    double& objectiveValue
) const
{
    const EvaluationStatus inputStatus = validateEvaluationInput(values);

    if (inputStatus != EvaluationOk)
        return inputStatus;

    const FuzzyInferenceResult inf = inference(values);

    Defuzzifier def;

    if (!def.defuzzify(inf, objectiveValue))
        return EvaluationDefuzzificationFailed;

    return EvaluationOk;
}
