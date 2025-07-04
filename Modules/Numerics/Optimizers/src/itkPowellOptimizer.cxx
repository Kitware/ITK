/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkPowellOptimizer.h"
#include "itkMath.h"

namespace itk
{
PowellOptimizer::PowellOptimizer()
  : m_MaximumIteration(100)
  , m_MaximumLineIteration(100)
  , m_StepLength(1.0)
  , m_StepTolerance(0.00001)
  , m_ValueTolerance(0.00001)

{
  m_StopConditionDescription << this->GetNameOfClass() << ": ";
}

PowellOptimizer::~PowellOptimizer() = default;

void
PowellOptimizer::SetLine(const PowellOptimizer::ParametersType & origin, const vnl_vector<double> & direction)
{
  const Optimizer::ScalesType & inv_scales = this->GetInverseScales();
  for (unsigned int i = 0; i < m_SpaceDimension; ++i)
  {
    m_LineOrigin[i] = origin[i];
    m_LineDirection[i] = direction[i] * inv_scales[i];
  }
}

double
PowellOptimizer::GetLineValue(double x) const
{
  PowellOptimizer::ParametersType tempCoord(m_SpaceDimension);

  return this->GetLineValue(x, tempCoord);
}

double
PowellOptimizer::GetLineValue(double x, ParametersType & tempCoord) const
{
  for (unsigned int i = 0; i < m_SpaceDimension; ++i)
  {
    tempCoord[i] = this->m_LineOrigin[i] + x * this->m_LineDirection[i];
  }
  itkDebugMacro("x = " << x);
  double val = NAN;
  try
  {
    val = (this->m_CostFunction->GetValue(tempCoord));
  }
  catch (...)
  {
    if (m_CatchGetValueException)
    {
      val = m_MetricWorstPossibleValue;
    }
    else
    {
      throw;
    }
  }
  if (m_Maximize)
  {
    val *= -1;
  }
  itkDebugMacro("val = " << val);
  return val;
}

void
PowellOptimizer::SetCurrentLinePoint(double x, double fx)
{
  for (unsigned int i = 0; i < m_SpaceDimension; ++i)
  {
    this->m_CurrentPosition[i] = this->m_LineOrigin[i] + x * this->m_LineDirection[i];
  }
  if (m_Maximize)
  {
    this->SetCurrentCost(-fx);
  }
  else
  {
    this->SetCurrentCost(fx);
  }
  this->Modified();
}

void
PowellOptimizer::Swap(double * a, double * b) const
{
  const double tf = *a;
  *a = *b;
  *b = tf;
}

void
PowellOptimizer::Shift(double * a, double * b, double * c, double d) const
{
  *a = *b;
  *b = *c;
  *c = d;
}

//
// This code was implemented from the description of
// the Golden section search available in the Wikipedia
//
// https://en.wikipedia.org/wiki/Golden_section_search
//
//
// The inputs to this function are
//
// x1 and its function value f1
// x2
//
// (f2 is not yet evaluated, it will be computed inside)
// (x2 and its function value f3 are also computed inside)
//
// The outputs are the values of x2 and f2 at
// the end of the iterations.
//
void
PowellOptimizer::LineBracket(double * x1, double * x2, double * x3, double * f1, double * f2, double * f3)
{
  PowellOptimizer::ParametersType tempCoord(m_SpaceDimension);

  this->LineBracket(x1, x2, x3, f1, f2, f3, tempCoord);
}

void
PowellOptimizer::LineBracket(double *         x1,
                             double *         x2,
                             double *         x3,
                             double *         f1,
                             double *         f2,
                             double *         f3,
                             ParametersType & tempCoord)
{
  //
  // Compute the golden ratio as a constant to be
  // used when extrapolating the bracket
  //
  const double goldenRatio = (1.0 + std::sqrt(5.0)) / 2.0;

  //
  // Get the value of the function for point x2
  //
  *f2 = this->GetLineValue(*x2, tempCoord);

  //
  // Compute extrapolated point using the golden ratio
  //
  if (*f2 >= *f1)
  {
    this->Swap(x1, x2);
    this->Swap(f1, f2);
  }

  // compute x3 on the side of x2
  *x3 = *x1 + goldenRatio * (*x2 - *x1);
  *f3 = this->GetLineValue(*x3, tempCoord);

  // If the new point is a minimum
  // then continue extrapolating in
  // that direction until we find a
  // value of f3 that makes f2 to be
  // a minimum.
  while (*f3 < *f2)
  {
    *x2 = *x3;
    *f2 = *f3;
    *x3 = *x1 + goldenRatio * (*x2 - *x1);
    *f3 = this->GetLineValue(*x3, tempCoord);
  }

  itkDebugMacro("Initial: " << *x1 << ", " << *x2 << ", " << *x3);
  //
  // Report the central point as the minimum
  //
  this->SetCurrentLinePoint(*x2, *f2);
}

void
PowellOptimizer::BracketedLineOptimize(double   ax,
                                       double   bx,
                                       double   cx,
                                       double   fa,
                                       double   functionValueOfb,
                                       double   fc,
                                       double * extX,
                                       double * extVal)
{
  PowellOptimizer::ParametersType tempCoord(m_SpaceDimension);

  this->BracketedLineOptimize(ax, bx, cx, fa, functionValueOfb, fc, extX, extVal, tempCoord);
}

void
PowellOptimizer::BracketedLineOptimize(double           ax,
                                       double           bx,
                                       double           cx,
                                       double           itkNotUsed(fa),
                                       double           functionValueOfb,
                                       double           itkNotUsed(fc),
                                       double *         extX,
                                       double *         extVal,
                                       ParametersType & tempCoord)
{
  double v = 0.0;
  /* Abscissae, descr. see above  */

  double a = (ax < cx ? ax : cx);
  double b = (ax > cx ? ax : cx);

  double x = bx;
  double w = bx;

  const double goldenSectionRatio = (3.0 - std::sqrt(5.0)) / 2; /* Gold
                                                                 section
                                                                 ratio    */
  constexpr double POWELL_TINY = 1.0e-20;

  /* f(x)        */
  double functionValueOfV = functionValueOfb;
  /* f(v)        */
  double functionValueOfX = functionValueOfV;
  /* f(w)        */
  double functionValueOfW = functionValueOfV;

  for (m_CurrentLineIteration = 0; m_CurrentLineIteration < m_MaximumLineIteration; ++m_CurrentLineIteration)
  {
    const double middle_range = (a + b) / 2;

    /* Step at this iteration       */
    const double tolerance1 = m_StepTolerance * itk::Math::abs(x) + POWELL_TINY;
    const double tolerance2 = 2.0 * tolerance1;

    if (itk::Math::abs(x - middle_range) <= (tolerance2 - 0.5 * (b - a)) || 0.5 * (b - a) < m_StepTolerance)
    {
      *extX = x;
      *extVal = functionValueOfX;
      this->SetCurrentLinePoint(x, functionValueOfX);
      itkDebugMacro("x = " << *extX);
      itkDebugMacro("val = " << *extVal);
      itkDebugMacro("return 1");
      return; /* Acceptable approx. is found  */
    }

    /* Obtain the gold section step  */
    double new_step = goldenSectionRatio * (x < middle_range ? b - x : a - x);

    /* Decide if the interpolation can be tried  */
    if (itk::Math::abs(x - w) >= tolerance1) /* If x and w are distinct      */
    {
      const double t = (x - w) * (functionValueOfX - functionValueOfV);

      /* ted as p/q; division operation*/
      double q = (x - v) * (functionValueOfX - functionValueOfW);

      /* Interpolation step is calcula-*/
      double p = (x - v) * q - (x - w) * t;

      q = 2 * (q - t);

      if (q > 0.0) /* q was calculated with the op-*/
      {
        p *= -1; /* posite sign; make q positive  */
      }
      else /* and assign possible minus to  */
      {
        q *= -1; /* p        */
      }

      /* Check if x+p/q falls in [a,b] and  not too close to a and b
           and isn't too large */
      if (itk::Math::abs(p) < itk::Math::abs(new_step * q) && p > q * (a - x + 2 * tolerance1) &&
          p < q * (b - x - 2 * tolerance1))
      {
        new_step = p / q; /* it is accepted         */
      }

      /* If p/q is too large then the  gold section procedure can
         reduce [a,b] range to more  extent      */
    }

    /* Adjust the step to be not less than tolerance*/
    if (itk::Math::abs(new_step) < tolerance1)
    {
      if (new_step > 0.0)
      {
        new_step = tolerance1;
      }
      else
      {
        new_step = -tolerance1;
      }
    }

    /* Obtain the next approximation to min  */
    /* and reduce the enveloping range  */
    const double t = x + new_step; /* Tentative point for the min  */

    const double functionValueOft = this->GetLineValue(t, tempCoord);

    if (functionValueOft <= functionValueOfX)
    {
      if (t < x) /* Reduce the range so that  */
      {
        b = x; /* t would fall within it  */
      }
      else
      {
        a = x;
      }

      /* assign the best approximation to x */
      v = w;
      w = x;
      x = t;

      functionValueOfV = functionValueOfW;
      functionValueOfW = functionValueOfX;
      functionValueOfX = functionValueOft;
    }
    else /* x remains the better approx  */
    {
      if (t < x) /* Reduce the range enclosing x  */
      {
        a = t;
      }
      else
      {
        b = t;
      }

      if (functionValueOft <= functionValueOfW || Math::ExactlyEquals(w, x))
      {
        v = w;
        w = t;
        functionValueOfV = functionValueOfW;
        functionValueOfW = functionValueOft;
      }
      else if (functionValueOft <= functionValueOfV || Math::AlmostEquals(v, x) || itk::Math::AlmostEquals(v, w))
      {
        v = t;
        functionValueOfV = functionValueOft;
      }
    }
  }

  *extX = x;
  *extVal = functionValueOfX;
  itkDebugMacro("x = " << *extX);
  itkDebugMacro("val = " << *extVal);
  itkDebugMacro("return 2");

  this->SetCurrentLinePoint(x, functionValueOfX);
}

void
PowellOptimizer::StartOptimization()
{
  if (m_CostFunction.IsNull())
  {
    return;
  }

  m_StopConditionDescription.str("");
  m_StopConditionDescription << this->GetNameOfClass() << ": ";

  this->InvokeEvent(StartEvent());
  m_Stop = false;

  m_SpaceDimension = m_CostFunction->GetNumberOfParameters();
  m_LineOrigin.set_size(m_SpaceDimension);
  m_LineDirection.set_size(m_SpaceDimension);
  this->m_CurrentPosition.set_size(m_SpaceDimension);

  vnl_matrix<double> xi(m_SpaceDimension, m_SpaceDimension);
  vnl_vector<double> xit(m_SpaceDimension);
  xi.set_identity();
  xit.fill(0);
  xit[0] = 1;

  PowellOptimizer::ParametersType tempCoord(m_SpaceDimension);

  PowellOptimizer::ParametersType p(m_SpaceDimension);
  PowellOptimizer::ParametersType pt(m_SpaceDimension);
  PowellOptimizer::ParametersType ptt(m_SpaceDimension);
  p = this->GetInitialPosition();
  pt = p;

  double xx = 0;
  this->SetLine(p, xit);
  double fx = this->GetLineValue(0, tempCoord);

  for (m_CurrentIteration = 0; m_CurrentIteration <= m_MaximumIteration; ++m_CurrentIteration)
  {
    const double fp = fx;
    unsigned int ibig = 0;
    double       del = 0.0;

    for (unsigned int i = 0; i < m_SpaceDimension; ++i)
    {
      for (unsigned int j = 0; j < m_SpaceDimension; ++j)
      {
        xit[j] = xi[j][i];
      }
      const double fptt = fx;

      this->SetLine(p, xit);

      double ax = 0.0;
      double fa = fx;
      xx = m_StepLength;
      double bx = NAN;
      double fb = NAN;
      this->LineBracket(&ax, &xx, &bx, &fa, &fx, &fb, tempCoord);
      this->BracketedLineOptimize(ax, xx, bx, fa, fx, fb, &xx, &fx, tempCoord);
      this->SetCurrentLinePoint(xx, fx);
      p = this->GetCurrentPosition();

      if (itk::Math::abs(fptt - fx) > del)
      {
        del = itk::Math::abs(fptt - fx);
        ibig = i;
      }
    }

    if (2.0 * itk::Math::abs(fp - fx) <= m_ValueTolerance * (itk::Math::abs(fp) + itk::Math::abs(fx)))
    {
      m_StopConditionDescription << "Cost function values at the current parameter (" << fx
                                 << ") and at the local extrema (" << fp << ") are within Value Tolerance ("
                                 << m_ValueTolerance << ')';
      this->InvokeEvent(EndEvent());
      return;
    }

    const Optimizer::ScalesType & scales = this->GetScales();
    for (unsigned int j = 0; j < m_SpaceDimension; ++j)
    {
      ptt[j] = 2.0 * p[j] - pt[j];
      xit[j] = (p[j] - pt[j]) * scales[j];
      pt[j] = p[j];
    }

    this->SetLine(ptt, xit);
    const double fptt = this->GetLineValue(0, tempCoord);
    if (fptt < fp)
    {
      const double t = 2.0 * (fp - 2.0 * fx + fptt) * itk::Math::sqr(fp - fx - del) - del * itk::Math::sqr(fp - fptt);
      if (t < 0.0)
      {
        this->SetLine(p, xit);

        double ax = 0.0;
        double fa = fx;
        xx = 1;
        double bx = NAN;
        double fb = NAN;
        this->LineBracket(&ax, &xx, &bx, &fa, &fx, &fb, tempCoord);
        this->BracketedLineOptimize(ax, xx, bx, fa, fx, fb, &xx, &fx, tempCoord);
        this->SetCurrentLinePoint(xx, fx);
        p = this->GetCurrentPosition();

        for (unsigned int j = 0; j < m_SpaceDimension; ++j)
        {
          xi[j][ibig] = xx * xit[j];
        }
      }
    }

    this->InvokeEvent(IterationEvent());
  }

  m_StopConditionDescription << "Maximum number of iterations exceeded. "
                             << "Number of iterations is " << m_MaximumIteration;
  this->InvokeEvent(EndEvent());
}

/**
 *
 */
std::string
PowellOptimizer::GetStopConditionDescription() const
{
  return m_StopConditionDescription.str();
}

/**
 *
 */
void
PowellOptimizer::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Metric Worst Possible Value " << m_MetricWorstPossibleValue << std::endl;
  os << indent << "Catch GetValue Exception " << m_CatchGetValueException << std::endl;
  os << indent << "Space Dimension   " << m_SpaceDimension << std::endl;
  os << indent << "Maximum Iteration " << m_MaximumIteration << std::endl;
  os << indent << "Current Iteration " << m_CurrentIteration << std::endl;
  os << indent << "Maximize On/Off   " << m_Maximize << std::endl;
  os << indent << "StepLength        " << m_StepLength << std::endl;
  os << indent << "StepTolerance     " << m_StepTolerance << std::endl;
  os << indent << "ValueTolerance    " << m_ValueTolerance << std::endl;
  os << indent << "LineOrigin        " << m_LineOrigin << std::endl;
  os << indent << "LineDirection     " << m_LineDirection << std::endl;
  os << indent << "Current Cost      " << m_CurrentCost << std::endl;
  os << indent << "Maximum Line Iteration " << m_MaximumLineIteration << std::endl;
  os << indent << "Current Line Iteration " << m_CurrentLineIteration << std::endl;
  os << indent << "Stop              " << m_Stop << std::endl;
}
} // end of namespace itk
