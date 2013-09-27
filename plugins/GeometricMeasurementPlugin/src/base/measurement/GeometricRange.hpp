//$Id: GeometricRange.hpp 1398 2011-04-21 20:39:37Z ljun@NDC $
//------------------------------------------------------------------------------
//                         GeometricRange
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG06CA54C
//
// Author: Darrel J. Conway, Thinking Systems, Inc.
// Created: 2009/06/29
//
/**
 * Definition of the geometric range measurement.
 */
//------------------------------------------------------------------------------


#ifndef GeometricRange_hpp
#define GeometricRange_hpp

#include "geometricmeasurement_defs.hpp"
#include "CoreMeasurement.hpp"

/**
 * A basic, simple range measurement
 *
 * The GeometricRange Measurement evaluates the instantaneous displacement
 * between two participants, taking into account a potentially obscuring body
 * between them.  This measurement does not evaluate light transit time between
 * the participants, nor does it include any corrections to the transit path.
 */
class GEOMETRICMEAS_API GeometricRange: public CoreMeasurement
{
public:
   GeometricRange(const std::string &name = "");
   virtual ~GeometricRange();
   GeometricRange(const GeometricRange &rm);
   GeometricRange& operator=(const GeometricRange &rm);

   virtual GmatBase*       Clone() const;
   virtual bool            Initialize();
   virtual const std::vector<RealArray>&
                           CalculateMeasurementDerivatives(GmatBase *obj,
                                 Integer id);

protected:
   bool                    Evaluate(bool withEvents = false);
};

#endif /* GeometricRange_hpp */