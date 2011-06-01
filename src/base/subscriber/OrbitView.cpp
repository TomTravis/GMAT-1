//$Id$
//------------------------------------------------------------------------------
//                                  OrbitView
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2010/04/19
//
/**
 * Implements OrbitView class.
 */
//------------------------------------------------------------------------------

#include "OrbitView.hpp"
#include "PlotInterface.hpp"       // for UpdateGlPlot()
#include "ColorTypes.hpp"          // for namespace GmatColor::
#include "SubscriberException.hpp" // for SubscriberException()
#include "MessageInterface.hpp"    // for ShowMessage()
#include "TextParser.hpp"          // for SeparateBrackets()
#include "StringUtil.hpp"          // for ToReal()
#include "CoordinateConverter.hpp" // for Convert()
#include <algorithm>               // for find(), distance()

//#define DBGLVL_INIT 1
//#define DBGLVL_DATA 1
//#define DBGLVL_DATA_LABELS 1
//#define DBGLVL_ADD 1
//#define DBGLVL_OBJ 2
//#define DBGLVL_PARAM 2
//#define DBGLVL_PARAM_STRING 2
//#define DBGLVL_PARAM_RVEC3 1
//#define DBGLVL_UPDATE 2
//#define DBGLVL_TAKE_ACTION 1
//#define DBGLVL_REMOVE_SP 1
//#define DBGLVL_RENAME 1
//#define DBGLVL_SOLVER_CURRENT_ITER 2

//---------------------------------
// static data
//---------------------------------
const std::string
OrbitView::PARAMETER_TEXT[OrbitViewParamCount - OrbitPlotParamCount] =
{
   "CoordinateSystem",
   "ViewPointRef",
   "ViewPointReference",
   "ViewPointRefType",
   "ViewPointRefVector",
   "ViewPointVector",
   "ViewPointVectorType",
   "ViewPointVectorVector",
   "ViewDirection",
   "ViewDirectionType",
   "ViewDirectionVector",
   "ViewScaleFactor",
   "FixedFovAngle",
   "ViewUpCoordinateSystem",
   "ViewUpAxis",
   "CelestialPlane",
   "XYPlane",
   "WireFrame",
   "Axes",
   "Grid",
   "EarthSunLines",
   "SunLine",
   "Overlap",
   "UseInitialView",
   "PerspectiveMode",
   "UseFixedFov",
   "StarCount",
   "EnableStars",
   "EnableConstellations",
   "MinFOV",
   "MaxFOV",
   "InitialFOV",
}; 


const Gmat::ParameterType
OrbitView::PARAMETER_TYPE[OrbitViewParamCount - OrbitPlotParamCount] =
{
   Gmat::OBJECT_TYPE,            //"CoordinateSystem"
   Gmat::OBJECT_TYPE,            //"ViewPointRef",
   Gmat::OBJECT_TYPE,            //"ViewPointReference",
   Gmat::STRING_TYPE,            //"ViewPointRefType"
   Gmat::RVECTOR_TYPE,           //"ViewPointRefVector",
   Gmat::OBJECT_TYPE,            //"ViewPointVector",
   Gmat::STRING_TYPE,            //"ViewPointVectorType",
   Gmat::RVECTOR_TYPE,           //"ViewPointVectorVector",
   Gmat::OBJECT_TYPE,            //"ViewDirection",
   Gmat::STRING_TYPE,            //"ViewDirectionType",
   Gmat::RVECTOR_TYPE,           //"ViewDirectionVector",
   Gmat::REAL_TYPE,              //"ViewScaleFactor",
   Gmat::REAL_TYPE,              //"FixedFovAngle",
   Gmat::OBJECT_TYPE,            //"ViewUpCoordinaetSystem"
   Gmat::ENUMERATION_TYPE,       //"ViewUpAxis"
   
   Gmat::ON_OFF_TYPE,            //"CelestialPlane"
   Gmat::ON_OFF_TYPE,            //"XYPlane"
   Gmat::ON_OFF_TYPE,            //"WireFrame"
   Gmat::ON_OFF_TYPE,            //"Axes"
   Gmat::ON_OFF_TYPE,            //"Grid"
   Gmat::ON_OFF_TYPE,            //"EarthSunLines"   
   Gmat::ON_OFF_TYPE,            //"SunLine"   
   Gmat::ON_OFF_TYPE,            //"Overlap"
   Gmat::ON_OFF_TYPE,            //"LockView"
   Gmat::ON_OFF_TYPE,            //"PerspectiveMode"
   Gmat::ON_OFF_TYPE,            //"UseFixedFov"
   
   Gmat::INTEGER_TYPE,           //"StarCount"
   Gmat::ON_OFF_TYPE,            //"EnableStars"
   Gmat::ON_OFF_TYPE,            //"EnableConstellations"
   Gmat::INTEGER_TYPE,           //"MinFOV"
   Gmat::INTEGER_TYPE,           //"MaxFOV"
   Gmat::INTEGER_TYPE,           //"InitialFOV"
};


//------------------------------------------------------------------------------
// OrbitView(const std::string &name)
//------------------------------------------------------------------------------
/**
 * The default constructor
 */
//------------------------------------------------------------------------------
OrbitView::OrbitView(const std::string &name)
   : OrbitPlot("OrbitView", name)
{
   // GmatBase data
   parameterCount = OrbitViewParamCount;
   objectTypes.push_back(Gmat::ORBIT_VIEW);
   objectTypeNames.push_back("OrbitView");
   
   mEclipticPlane = "Off";
   mXYPlane = "On";
   mWireFrame = "Off";
   mAxes = "On";
   mGrid = "Off";
   mSunLine = "Off";
   mOverlapPlot = "Off";
   mUseInitialView = "On";
   mPerspectiveMode = "Off";
   mUseFixedFov = "Off";
   
   // stars
   mEnableStars = "On";
   mEnableConstellations = "On";
   mStarCount = 7000;
   
   // FOV
   mMinFOV = 0;
   mMaxFOV = 90;
   mInitialFOV = 45;
   
   mViewCoordSysName = "EarthMJ2000Eq";
   mViewUpCoordSysName = "EarthMJ2000Eq";
   mViewUpAxisName = "Z";
   
   // viewpoint
   mViewPointRefName = "Earth";
   mViewPointRefType = "Object";
   mViewPointVecName = "[ 0 0 30000 ]";
   mViewPointVecType = "Vector";
   mViewDirectionName = "Earth";
   mViewDirectionType= "Object";
   mViewScaleFactor = 1.0;
   mFixedFovAngle = 45.0;
   mViewPointRefVector.Set(0.0, 0.0, 0.0);
   mViewPointVecVector.Set(0.0, 0.0, 30000.0);
   mViewDirectionVector.Set(0.0, 0.0, -1.0);
   
   mViewCoordSystem = NULL;
   mViewUpCoordSystem = NULL;
   mViewCoordSysOrigin = NULL;
   mViewUpCoordSysOrigin = NULL;
   mViewPointRefObj = NULL;
   mViewPointObj = NULL;
   mViewDirectionObj = NULL;
}


//------------------------------------------------------------------------------
// OrbitView(const OrbitView &ov)
//------------------------------------------------------------------------------
/**
 * The copy consturctor
 */
//------------------------------------------------------------------------------
OrbitView::OrbitView(const OrbitView &ov)
   : OrbitPlot(ov)
{
   mEclipticPlane = ov.mEclipticPlane;
   mXYPlane = ov.mXYPlane;
   mWireFrame = ov.mWireFrame;
   mAxes = ov.mAxes;
   mGrid = ov.mGrid;
   mSunLine = ov.mSunLine;
   mOverlapPlot = ov.mOverlapPlot;
   mUseInitialView = ov.mUseInitialView;
   mPerspectiveMode = ov.mPerspectiveMode;
   mUseFixedFov = ov.mUseFixedFov;

   // stars
   mEnableStars = ov.mEnableStars;
   mEnableConstellations = ov.mEnableConstellations;
   mStarCount = ov.mStarCount;
   
   // FOV
   mMinFOV = ov.mMinFOV;
   mMaxFOV = ov.mMaxFOV;
   mInitialFOV = ov.mInitialFOV;
   
   mViewCoordSysName = ov.mViewCoordSysName;
   
   // viewpoint
   mViewPointRefName = ov.mViewPointRefName;
   mViewPointRefType = ov.mViewPointRefType;
   mViewPointVecName = ov.mViewPointVecName;
   mViewPointVecType = ov.mViewPointVecType;
   mViewDirectionName = ov.mViewDirectionName;
   mViewDirectionType = ov.mViewDirectionType;
   mViewScaleFactor = ov.mViewScaleFactor;
   mFixedFovAngle = ov.mFixedFovAngle;
   mViewPointRefVector = ov.mViewPointRefVector;
   mViewPointVecVector = ov.mViewPointVecVector;
   mViewDirectionVector = ov.mViewDirectionVector;
   mViewUpCoordSysName = ov.mViewUpCoordSysName;
   mViewUpAxisName = ov.mViewUpAxisName;
   
   mViewCoordSystem = ov.mViewCoordSystem;
   mViewUpCoordSystem = ov.mViewCoordSystem;
   mViewCoordSysOrigin = ov.mViewCoordSysOrigin;
   mViewUpCoordSysOrigin = ov.mViewUpCoordSysOrigin;
   mViewPointRefObj = ov.mViewPointRefObj;
   mViewPointObj = ov.mViewPointObj;
   mViewDirectionObj = ov.mViewDirectionObj;
}


//------------------------------------------------------------------------------
// OrbitView& operator=(const OrbitView&)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 */
//------------------------------------------------------------------------------
OrbitView& OrbitView::operator=(const OrbitView& ov)
{
   if (this == &ov)
      return *this;
   
   OrbitPlot::operator=(ov);
   
   mEclipticPlane = ov.mEclipticPlane;
   mXYPlane = ov.mXYPlane;
   mWireFrame = ov.mWireFrame;
   mAxes = ov.mAxes;
   mGrid = ov.mGrid;
   mSunLine = ov.mSunLine;
   mOverlapPlot = ov.mOverlapPlot;
   mUseInitialView = ov.mUseInitialView;
   mPerspectiveMode = ov.mPerspectiveMode;
   mUseFixedFov = ov.mUseFixedFov;
   
   // stars
   mEnableStars = ov.mEnableStars;
   mEnableConstellations = ov.mEnableConstellations;
   mStarCount = ov.mStarCount;
   
   // FOV
   mMinFOV = ov.mMinFOV;
   mMaxFOV = ov.mMaxFOV;
   mInitialFOV = ov.mInitialFOV;

   // View coordinate system name
   mViewCoordSysName = ov.mViewCoordSysName;
   
   // viewpoint
   mViewPointRefName = ov.mViewPointRefName;
   mViewPointRefType = ov.mViewPointRefType;
   mViewPointVecName = ov.mViewPointVecName;
   mViewPointVecType = ov.mViewPointVecType;
   mViewDirectionName = ov.mViewDirectionName;
   mViewDirectionType = ov.mViewDirectionType;
   mViewScaleFactor = ov.mViewScaleFactor;
   mFixedFovAngle = ov.mFixedFovAngle;
   mViewPointRefVector = ov.mViewPointRefVector;
   mViewPointVecVector = ov.mViewPointVecVector;
   mViewDirectionVector = ov.mViewDirectionVector;
   mViewUpCoordSysName = ov.mViewUpCoordSysName;
   mViewUpAxisName = ov.mViewUpAxisName;
   
   // object pointers
   mViewCoordSystem = ov.mViewCoordSystem;
   mViewUpCoordSystem = ov.mViewCoordSystem;
   mViewCoordSysOrigin = ov.mViewCoordSysOrigin;
   mViewUpCoordSysOrigin = ov.mViewUpCoordSysOrigin;
   mViewPointRefObj = ov.mViewPointRefObj;
   mViewPointObj = ov.mViewPointObj;
   mViewDirectionObj = ov.mViewDirectionObj;
   
   return *this;
}


//------------------------------------------------------------------------------
// ~OrbitView()
//------------------------------------------------------------------------------
/**
 * Destructor
 *
 * @note This destructor does not delete 3DView window, but clears data.
 *       3DView window is deleted when it is closed by the user or GMAT
 *       shuts down.
 */
//------------------------------------------------------------------------------
OrbitView::~OrbitView()
{
   PlotInterface::TakeGlAction(instanceName, "ClearObjects");
}


//------------------------------------------------------------------------------
// Rvector3 GetVector(const std::string &which)
//------------------------------------------------------------------------------
Rvector3 OrbitView::GetVector(const std::string &which)
{
   if (which == "ViewPointReference")
      return mViewPointRefVector;
   else if (which == "ViewPointVector")
      return mViewPointVecVector;
   else if (which == "ViewDirection")
      return mViewDirectionVector;
   else
      throw SubscriberException(which + " is unknown OrbitView parameter\n");
}


//------------------------------------------------------------------------------
// void SetVector(const std::string &which, const Rvector3 &value)
//------------------------------------------------------------------------------
void OrbitView::SetVector(const std::string &which, const Rvector3 &value)
{
   #if DBGLVL_SET
   MessageInterface::ShowMessage
      ("OrbitView::SetVector() which=%s, value=%s\n", which.c_str(),
       value.ToString().c_str());
   #endif
   
   if (which == "ViewPointReference")
      mViewPointRefVector = value;
   else if (which == "ViewPointVector")
      mViewPointVecVector = value;
   else if (which == "ViewDirection")
      mViewDirectionVector = value;
   else
      throw SubscriberException(which + " is unknown OrbitView parameter\n");
}


//----------------------------------
// inherited methods from Subscriber
//----------------------------------


//---------------------------------
// inherited methods from GmatBase
//---------------------------------

//------------------------------------------------------------------------------
//  bool Validate()
//------------------------------------------------------------------------------
/**
 * Performs any pre-run validation that the object needs.
 *
 * @return true unless validation fails.
 */
//------------------------------------------------------------------------------
bool OrbitView::Validate()
{
   // Anything to validate here?
   
   return OrbitPlot::Validate();
}


//------------------------------------------------------------------------------
// virtual bool Initialize()
//------------------------------------------------------------------------------
bool OrbitView::Initialize()
{
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   OrbitPlot::Initialize();
   
   // theInternalCoordSys is used only by 3DView so check. (2008.06.16)
   if (theInternalCoordSystem == NULL)
   {
      active = false;
      MessageInterface::PopupMessage
         (Gmat::WARNING_, "*** WARNING *** The 3DView named \"%s\" will be turned off. "
          "It has a NULL internal coordinate system pointer.\n", GetName().c_str());
      return false;
   }
   
   #if DBGLVL_INIT
   MessageInterface::ShowMessage
      ("OrbitView::Initialize() this=<%p>'%s', active=%d, isInitialized=%d, "
       "isEndOfReceive=%d, mAllSpCount=%d\n", this, GetName().c_str(), active,
       isInitialized, isEndOfReceive, mAllSpCount);
   #endif
   
   bool retval = false;
   
   //--------------------------------------------------------
   // start initializing for 3DView
   //--------------------------------------------------------
   if (active && !isInitialized)
   {
      #if DBGLVL_INIT
      MessageInterface::ShowMessage
         ("OrbitView::Initialize() CreateGlPlotWindow() theSolarSystem=%p\n",
          theSolarSystem);
      #endif
      
      // set ViewType
      PlotInterface::SetViewType(GmatPlot::ENHANCED_3D_VIEW);
      
      if (PlotInterface::CreateGlPlotWindow
          (instanceName, mOldName, (mEclipticPlane == "On"), (mXYPlane == "On"),
           (mWireFrame == "On"), (mAxes == "On"), (mGrid == "On"),
           (mSunLine == "On"), (mOverlapPlot == "On"), (mUseInitialView == "On"),
           (mPerspectiveMode == "On"), mNumPointsToRedraw, 
           (mEnableStars == "On"), (mEnableConstellations == "On"), mStarCount))
      {
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            ("   mViewPointRefObj=%p, mViewScaleFactor=%f\n",
             mViewPointRefObj, mViewScaleFactor);
         #endif
         
         //--------------------------------------------------------
         // Set Spacecraft and non-Spacecraft objects.
         // If non-Spacecraft, position has to be computed in the
         // TrajPlotCanvas, so need to pass those object pointers.
         //--------------------------------------------------------
         
         ClearDynamicArrays();
         BuildDynamicArrays();
         
         
         // Should these moved to OrbitPlot? (Yes, for now)
         //mScCount = mScNameArray.size();
         //mObjectCount = mObjectNameArray.size();
         
         // check ViewPoint info to see if any objects need to be
         // included in the non-spacecraft list
         if (mViewCoordSystem == NULL)
            throw SubscriberException
               ("OrbitView::Initialize() CoordinateSystem: " + mViewCoordSysName +
                " not set\n");
         
         if (mViewUpCoordSystem == NULL)
            throw SubscriberException
               ("OrbitView::Initialize() CoordinateSystem: " + mViewUpCoordSysName +
                " not set\n");               
         
         // Get View CoordinateSystem Origin pointer
         mViewCoordSysOrigin = mViewCoordSystem->GetOrigin();
         
         if (mViewCoordSysOrigin != NULL)
            UpdateObjectList(mViewCoordSysOrigin);
         
         // Get View Up CoordinateSystem Origin pointer
         mViewUpCoordSysOrigin = mViewUpCoordSystem->GetOrigin();
         
         if (mViewUpCoordSysOrigin != NULL)
            UpdateObjectList(mViewUpCoordSysOrigin);
         
         // Get ViewPointRef object pointer from the current SolarSystem
         if (mViewPointRefObj != NULL)
            UpdateObjectList(mViewPointRefObj);
         
         // Get ViewPoint object pointer from the current SolarSystem
         if (mViewPointObj != NULL)
            UpdateObjectList(mViewPointObj);
         
         // Get ViewDirection object pointer from the current SolarSystem
         if (mViewDirectionObj != NULL)
            UpdateObjectList(mViewDirectionObj);
         
         // Add Sun to list if it was not added already to enable light source (LOJ: 2010.11.22)
         if (find(mObjectNameArray.begin(), mObjectNameArray.end(), "Sun") ==
             mObjectNameArray.end())
            UpdateObjectList(theSolarSystem->GetBody("Sun"), false);
         
         #if DBGLVL_INIT > 1
         MessageInterface::ShowMessage
            ("   mScNameArray.size=%d, mScOrbitColorArray.size=%d\n",
             mScNameArray.size(), mScOrbitColorArray.size());
         MessageInterface::ShowMessage
            ("   mObjectNameArray.size=%d, mOrbitColorArray.size=%d\n",
             mObjectNameArray.size(), mOrbitColorArray.size());
         
         bool draw, show;
         for (int i=0; i<mObjectCount; i++)
         {
            draw = mDrawOrbitArray[i] ? true : false;
            show = mDrawObjectArray[i] ? true : false;
            MessageInterface::ShowMessage
               ("   mObjectNameArray[%d]=%s, draw=%d, show=%d, color=%d\n",
                i, mObjectNameArray[i].c_str(), draw, show, mOrbitColorArray[i]);
         }
         #endif
         
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            ("   calling PlotInterface::SetGlSolarSystem(%p)\n", theSolarSystem);
         #endif
         
         // set SolarSystem
         PlotInterface::SetGlSolarSystem(instanceName, theSolarSystem);
         
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            ("   calling PlotInterface::SetGlObject()\n");
         for (UnsignedInt i=0; i<mObjectArray.size(); i++)
            MessageInterface::ShowMessage
               ("      mObjectArray[%d]=<%p>'%s'\n", i, mObjectArray[i],
                mObjectArray[i]->GetName().c_str());
         #endif
         
         // set all object array and pointers
         PlotInterface::SetGlObject(instanceName, mObjectNameArray,
                                    mOrbitColorArray, mObjectArray);
         
         //--------------------------------------------------------
         // set CoordinateSystem
         //--------------------------------------------------------
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            ("   theInternalCoordSystem = <%p>, origin = <%p>'%s'\n"
             "   theDataCoordSystem     = <%p>, origin = <%p>'%s'\n"
             "   mViewCoordSystem       = <%p>, origin = <%p>'%s'\n"
             "   mViewUpCoordSystem     = <%p>, origin = <%p>'%s'\n"
             "   mViewPointRefObj       = <%p>'%s'\n"
             "   mViewPointObj          = <%p>'%s'\n"
             "   mViewDirectionObj      = <%p>'%s'\n",
             theInternalCoordSystem, theInternalCoordSystem->GetOrigin(),
             theInternalCoordSystem->GetOriginName().c_str(),
             theDataCoordSystem, theDataCoordSystem->GetOrigin(),
             theDataCoordSystem->GetOriginName().c_str(),
             mViewCoordSystem, mViewCoordSystem->GetOrigin(),
             mViewCoordSystem->GetOriginName().c_str(),
             mViewUpCoordSystem, mViewUpCoordSystem->GetOrigin(),
             mViewUpCoordSystem->GetOriginName().c_str(),
             mViewPointRefObj,
             mViewPointRefObj ? mViewPointRefObj->GetName().c_str() : "NULL",
             mViewPointObj,
             mViewPointObj ? mViewPointObj->GetName().c_str() : "NULL",
             mViewDirectionObj,
             mViewDirectionObj ? mViewDirectionObj->GetName().c_str() : "NULL");
         
         MessageInterface::ShowMessage
            ("   calling PlotInterface::SetGlCoordSystem()\n");
         #endif
         
         PlotInterface::SetGlCoordSystem(instanceName, theInternalCoordSystem,
                                         mViewCoordSystem, mViewUpCoordSystem);
         
         //--------------------------------------------------------
         // set viewpoint info
         //--------------------------------------------------------
         #if DBGLVL_INIT
         MessageInterface::ShowMessage
            ("   calling PlotInterface::SetGlViewOption()\n");
         #endif
         
         PlotInterface::SetGlViewOption
            (instanceName, mViewPointRefObj, mViewPointObj,
             mViewDirectionObj, mViewScaleFactor, mViewPointRefVector,
             mViewPointVecVector, mViewDirectionVector, mViewUpAxisName,
             (mViewPointRefType == "Vector"), (mViewPointVecType == "Vector"),
             (mViewDirectionType == "Vector"), (mUseFixedFov == "On"),
             mFixedFovAngle);
         
         PlotInterface::SetGlUpdateFrequency(instanceName, mUpdatePlotFrequency);
         
         //--------------------------------------------------------
         // set drawing object flag
         //--------------------------------------------------------
         PlotInterface::SetGlDrawOrbitFlag(instanceName, mDrawOrbitArray);
         PlotInterface::SetGlShowObjectFlag(instanceName, mDrawObjectArray);
         
         isInitialized = true;
         retval = true;
      }
      else
      {
         retval = false;
      }
   }
   else
   {
      #if DBGLVL_INIT
      MessageInterface::ShowMessage
         ("OrbitView::Initialize() Plot is active and initialized, "
          "so calling DeleteGlPlot()\n");
      #endif
      
      // Why do we want to delete plot if active and initialized?
      // This causes Global 3DView not to show, so commented out (loj: 2008.10.08)
      //retval =  PlotInterface::DeleteGlPlot(instanceName);
   }
   
   #if DBGLVL_INIT
   MessageInterface::ShowMessage("OrbitView::Initialize() exiting\n");
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
//  GmatBase* Clone() const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the OrbitView.
 *
 * @return clone of the OrbitView.
 *
 */
//------------------------------------------------------------------------------
GmatBase* OrbitView::Clone() const
{
   return (new OrbitView(*this));
}


//---------------------------------------------------------------------------
// void Copy(const GmatBase* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 * 
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void OrbitView::Copy(const GmatBase* orig)
{
   operator=(*((OrbitView *)(orig)));
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const std::string &action,  
//                         const std::string &actionData = "");
//------------------------------------------------------------------------------
/**
 * This method performs action.
 *
 * @param <action> action to perform
 * @param <actionData> action data associated with action
 * @return true if action successfully performed
 *
 */
//------------------------------------------------------------------------------
bool OrbitView::TakeAction(const std::string &action,
                           const std::string &actionData)
{
   #if DBGLVL_TAKE_ACTION
   MessageInterface::ShowMessage
      ("OrbitView::TakeAction() '%s' entered, action='%s', actionData='%s'\n",
       GetName().c_str(), action.c_str(), actionData.c_str());
   #endif
   if (action == "Clear")
   {
      return ClearSpacePointList();
   }
   else if (action == "Remove")
   {
      return RemoveSpacePoint(actionData);
   }
   else if (action == "Finalize")
   {
      PlotInterface::DeleteGlPlot(instanceName);
   }
   
   return false;
}


//---------------------------------------------------------------------------
//  bool RenameRefObject(const Gmat::ObjectType type,
//                       const std::string &oldName, const std::string &newName)
//---------------------------------------------------------------------------
bool OrbitView::RenameRefObject(const Gmat::ObjectType type,
                                const std::string &oldName,
                                const std::string &newName)
{
   #if DBGLVL_RENAME
   MessageInterface::ShowMessage
      ("OrbitView::RenameRefObject() type=%s, oldName=%s, newName=%s\n",
       GetObjectTypeString(type).c_str(), oldName.c_str(), newName.c_str());
   #endif
   
   if (type != Gmat::SPACECRAFT && type != Gmat::COORDINATE_SYSTEM)
      return true;
   
   if (type == Gmat::SPACECRAFT)
   {
      return OrbitPlot::RenameRefObject(type, oldName, newName);
   }
   else if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (mViewCoordSysName == oldName)
         mViewCoordSysName = newName;

      if (mViewUpCoordSysName == oldName)
         mViewUpCoordSysName = newName;      
   }
   
   return true;
}


//------------------------------------------------------------------------------
// std::string GetParameterText(const Integer id) const
//------------------------------------------------------------------------------
std::string OrbitView::GetParameterText(const Integer id) const
{
   if (id >= OrbitPlotParamCount && id < OrbitViewParamCount)
      return PARAMETER_TEXT[id - OrbitPlotParamCount];
   else
      return OrbitPlot::GetParameterText(id);
    
}


//------------------------------------------------------------------------------
// Integer GetParameterID(const std::string &str) const
//------------------------------------------------------------------------------
Integer OrbitView::GetParameterID(const std::string &str) const
{
   for (int i=OrbitPlotParamCount; i<OrbitViewParamCount; i++)
   {
      if (str == PARAMETER_TEXT[i - OrbitPlotParamCount])
         return i;
   }
   
   return OrbitPlot::GetParameterID(str);
}


//------------------------------------------------------------------------------
// Gmat::ParameterType GetParameterType(const Integer id) const
//------------------------------------------------------------------------------
Gmat::ParameterType OrbitView::GetParameterType(const Integer id) const
{
   if (id >= OrbitPlotParamCount && id < OrbitViewParamCount)
      return PARAMETER_TYPE[id - OrbitPlotParamCount];
   else
      return OrbitPlot::GetParameterType(id);
}


//------------------------------------------------------------------------------
// std::string GetParameterTypeString(const Integer id) const
//------------------------------------------------------------------------------
std::string OrbitView::GetParameterTypeString(const Integer id) const
{
   if (id >= OrbitPlotParamCount && id < OrbitViewParamCount)
      return GmatBase::PARAM_TYPE_STRING[GetParameterType(id - OrbitPlotParamCount)];
   else
      return OrbitPlot::GetParameterTypeString(id);
}


//---------------------------------------------------------------------------
//  bool IsParameterReadOnly(const Integer id) const
//---------------------------------------------------------------------------
/**
 * Checks to see if the requested parameter is read only.
 *
 * @param <id> Description for the parameter.
 *
 * @return true if the parameter is read only, false (the default) if not,
 *         throws if the parameter is out of the valid range of values.
 */
//---------------------------------------------------------------------------
bool OrbitView::IsParameterReadOnly(const Integer id) const
{
   //Note: We can remove PERSPECTIVE_MODE, USE_FIXED_FOV, FIXED_FOV_ANGLE
   //      when perspective mode is working.
   
   if (id == OVERLAP_PLOT ||
       id == PERSPECTIVE_MODE || id == USE_FIXED_FOV || id == FIXED_FOV_ANGLE ||
       id == MIN_FOV || id == MAX_FOV || id == INITIAL_FOV ||
       id == EARTH_SUN_LINES || id == VIEWPOINT_REF || id == VIEWPOINT_REF_VECTOR ||
       id == VIEWPOINT_VECTOR_VECTOR || id == VIEW_DIRECTION_VECTOR ||
       id == VIEWPOINT_REF_TYPE || id == VIEWPOINT_VECTOR_TYPE ||
       id == VIEW_DIRECTION_TYPE)
      return true;
   
   return OrbitPlot::IsParameterReadOnly(id);
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const Integer id) const
//------------------------------------------------------------------------------
Integer OrbitView::GetIntegerParameter(const Integer id) const
{
   switch (id)
   {
   case STAR_COUNT:
      return mStarCount;
   case MIN_FOV:
      return mMinFOV;
   case MAX_FOV:
      return mMaxFOV;
   case INITIAL_FOV:
      return mInitialFOV;
   default:
      return OrbitPlot::GetIntegerParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Integer GetIntegerParameter(const std::string &label) const
//------------------------------------------------------------------------------
Integer OrbitView::GetIntegerParameter(const std::string &label) const
{
   return GetIntegerParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const Integer id, const Integer value)
//------------------------------------------------------------------------------
Integer OrbitView::SetIntegerParameter(const Integer id, const Integer value)
{
   switch (id)
   {
   case STAR_COUNT:
      if (value >= 0)
      {
         mStarCount = value;
         return value;
      }
      else
      {
         SubscriberException se;
         se.SetDetails(errorMessageFormat.c_str(),
                       GmatStringUtil::ToString(value, 1).c_str(),
                       "StarCount", "Integer Value >= 0");
      }
   case MIN_FOV:
      mMinFOV = value;
      return value;
   case MAX_FOV:
      mMaxFOV = value;
      return value;
   case INITIAL_FOV:
      mInitialFOV = value;
      return value;
   default:
      return OrbitPlot::SetIntegerParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Integer SetIntegerParameter(const std::string &label,
//                                     const Integer value)
//------------------------------------------------------------------------------
Integer OrbitView::SetIntegerParameter(const std::string &label,
                                       const Integer value)
{
   return SetIntegerParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual Real GetRealParameter(const Integer id) const
//------------------------------------------------------------------------------
Real OrbitView::GetRealParameter(const Integer id) const
{
   switch (id)
   {
   case VIEW_SCALE_FACTOR:
      return mViewScaleFactor;
   case FIXED_FOV_ANGLE:
      return mFixedFovAngle;
   default:
      return OrbitPlot::GetRealParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual Real GetRealParameter(const std::string &label) const
//------------------------------------------------------------------------------
Real OrbitView::GetRealParameter(const std::string &label) const
{
   #if DBGLVL_PARAM
     MessageInterface::ShowMessage
        ("OrbitView::GetRealParameter() label = %s\n", label.c_str());
   #endif
   
   return GetRealParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual Real SetRealParameter(const Integer id, const Real value)
//------------------------------------------------------------------------------
Real OrbitView::SetRealParameter(const Integer id, const Real value)
{
   switch (id)
   {
   case VIEW_SCALE_FACTOR:
      mViewScaleFactor = value;
      return value;
   case FIXED_FOV_ANGLE:
      mFixedFovAngle = value;
      return value;
   default:
      return OrbitPlot::SetRealParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual Real SetRealParameter(const std::string &label, const Real value)
//------------------------------------------------------------------------------
Real OrbitView::SetRealParameter(const std::string &label, const Real value)
{
   #if DBGLVL_PARAM
      MessageInterface::ShowMessage
         ("OrbitView::SetRealParameter() label = %s, value = %f \n",
          label.c_str(), value);
   #endif
   
   return SetRealParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// Real GetRealParameter(const Integer id, const Integer index) const
//------------------------------------------------------------------------------
Real OrbitView::GetRealParameter(const Integer id, const Integer index) const
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      WriteDeprecatedMessage(id);
      return mViewPointRefVector[index];
      
   case VIEWPOINT_VECTOR_VECTOR:
      WriteDeprecatedMessage(id);
      return mViewPointVecVector[index];
      
   case VIEW_DIRECTION_VECTOR:
      WriteDeprecatedMessage(id);
      return mViewDirectionVector[index];
      
   default:
      return OrbitPlot::GetRealParameter(id, index);
   }
}


//------------------------------------------------------------------------------
// Real SetRealParameter(const Integer id, const Real value, const Integer index)
//------------------------------------------------------------------------------
Real OrbitView::SetRealParameter(const Integer id, const Real value,
                                 const Integer index)
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      WriteDeprecatedMessage(id);
      mViewPointRefVector[index] = value;
      return value;
      
   case VIEWPOINT_VECTOR_VECTOR:
      WriteDeprecatedMessage(id);
      mViewPointVecVector[index] = value;
      return value;
      
   case VIEW_DIRECTION_VECTOR:
      WriteDeprecatedMessage(id);
      mViewDirectionVector[index] = value;
      return value;
      
   default:
      return OrbitPlot::SetRealParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& GetRvectorParameter(const Integer id) const
//------------------------------------------------------------------------------
const Rvector& OrbitView::GetRvectorParameter(const Integer id) const
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      //WriteDeprecatedMessage(id);
      return mViewPointRefVector;
   case VIEWPOINT_VECTOR_VECTOR:
      {
         //WriteDeprecatedMessage(id);
         #if DBGLVL_PARAM
         Rvector vec = mViewPointVecVector;
         MessageInterface::ShowMessage
            ("OrbitView::GetRvectorParameter() returning = %s\n",
             vec.ToString().c_str());
         #endif
         return mViewPointVecVector;
      }
   case VIEW_DIRECTION_VECTOR:
      //WriteDeprecatedMessage(id);
      return mViewDirectionVector;
   default:
      return OrbitPlot::GetRvectorParameter(id);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& GetRvectorParameter(const std::string &label) const
//------------------------------------------------------------------------------
const Rvector& OrbitView::GetRvectorParameter(const std::string &label) const
{
   #if DBGLVL_PARAM
   MessageInterface::ShowMessage
      ("OrbitView::GetRvectorParameter() label = %s\n", label.c_str());
   #endif
   
   return GetRvectorParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// virtual const Rvector& SetRvectorParameter(const Integer id,
//                                            const Rvector &value)
//------------------------------------------------------------------------------
const Rvector& OrbitView::SetRvectorParameter(const Integer id,
                                              const Rvector &value)
{
   switch (id)
   {
   case VIEWPOINT_REF_VECTOR:
      WriteDeprecatedMessage(id);
      mViewPointRefVector[0] = value[0];
      mViewPointRefVector[1] = value[1];
      mViewPointRefVector[2] = value[2];
      return value;
      
   case VIEWPOINT_VECTOR_VECTOR:
      mViewPointVecVector[0] = value[0];
      mViewPointVecVector[1] = value[1];
      mViewPointVecVector[2] = value[2];
      return value;
      
   case VIEW_DIRECTION_VECTOR:
      WriteDeprecatedMessage(id);
      mViewDirectionVector[0] = value[0];
      mViewDirectionVector[1] = value[1];
      mViewDirectionVector[2] = value[2];
      return value;
      
   default:
      return OrbitPlot::SetRvectorParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// virtual const Rvector& SetRvectorParameter(const std::string &label,
//                                            const Rvector &value)
//------------------------------------------------------------------------------
const Rvector& OrbitView::SetRvectorParameter(const std::string &label,
                                              const Rvector &value)
{
   #if DBGLVL_PARAM
   Rvector val = value;
   MessageInterface::ShowMessage
      ("OrbitView::SetRvectorParameter() label = %s, "
       "value = %s \n", label.c_str(), val.ToString().c_str());
   #endif
   
   return SetRvectorParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// std::string GetStringParameter(const Integer id) const
//------------------------------------------------------------------------------
std::string OrbitView::GetStringParameter(const Integer id) const
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      ("OrbitView::GetStringParameter()<%s> id=%d<%s>\n",
       instanceName.c_str(), id, GetParameterText(id).c_str());
   #endif
   
   switch (id)
   {
   case COORD_SYSTEM:
      return mViewCoordSysName;
   case VIEWPOINT_REF:      
      WriteDeprecatedMessage(id);
      if (mViewPointRefType == "Vector")
         return ("[ " + mViewPointRefVector.ToString(16) + " ]");
      else
         return mViewPointRefName;
   case VIEWPOINT_REFERENCE:      
      if (mViewPointRefType == "Vector")         
         return ("[ " + mViewPointRefVector.ToString(16) + " ]");
      else
         return mViewPointRefName;
   case VIEWPOINT_REF_TYPE:
      return mViewPointRefType;
   case VIEWPOINT_VECTOR:
      if (mViewPointVecType == "Vector")
         return ("[ " + mViewPointVecVector.ToString(16) + " ]");
      else
         return mViewPointVecName;
   case VIEWPOINT_VECTOR_TYPE:
      return mViewPointVecType;
   case VIEW_DIRECTION:
      if (mViewDirectionType == "Vector")
         return ("[ " + mViewDirectionVector.ToString(16) + " ]");
      else
         return mViewDirectionName;
   case VIEW_DIRECTION_TYPE:
      return mViewDirectionType;
   case VIEW_UP_COORD_SYSTEM:
      return mViewUpCoordSysName;
   case VIEW_UP_AXIS:
      return mViewUpAxisName;
   default:
      return OrbitPlot::GetStringParameter(id);
   }
}


//------------------------------------------------------------------------------
// std::string GetStringParameter(const std::string &label) const
//------------------------------------------------------------------------------
std::string OrbitView::GetStringParameter(const std::string &label) const
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      ("OrbitView::GetStringParameter() label = %s\n", label.c_str());
   #endif
   
   return GetStringParameter(GetParameterID(label));
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const Integer id, const std::string &value)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const Integer id, const std::string &value)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      ("OrbitView::SetStringParameter() this=<%p>'%s', id=%d<%s>, value='%s'\n",
       this, instanceName.c_str(), id, GetParameterText(id).c_str(), value.c_str());
   #endif
   
   switch (id)
   {
   case COORD_SYSTEM:
      mViewCoordSysName = value;
      return true;
   case VIEWPOINT_REF:
      WriteDeprecatedMessage(id);
      mViewPointRefName = value;
      mViewPointRefType = "Object";
      
      // Handle deprecated value "Vector"
      if (value == "Vector" || GmatStringUtil::IsNumber(value))
         mViewPointRefType = "Vector";
      
      if (value[0] == '[')
      {
         PutRvector3Value(mViewPointRefVector, id, value);
         mViewPointRefType = "Vector";
      }
      return true;
   case VIEWPOINT_REFERENCE:
      mViewPointRefName = value;
      mViewPointRefType = "Object";
      
      // Handle deprecated value "Vector"
      if (value == "Vector" || GmatStringUtil::IsNumber(value))
         mViewPointRefType = "Vector";
      
      if (value[0] == '[')
      {
         PutRvector3Value(mViewPointRefVector, id, value);
         mViewPointRefType = "Vector";
      }
   case VIEWPOINT_REF_TYPE:
      mViewPointRefType = value;
      return true;
   case VIEWPOINT_VECTOR:
      mViewPointVecName = value;
      mViewPointVecType = "Object";
      
      // Handle deprecated value "Vector"
      if (value == "Vector" || GmatStringUtil::IsNumber(value))
         mViewPointVecType = "Vector";
      
      if (value[0] == '[')
      {
         PutRvector3Value(mViewPointVecVector, id, value);
         mViewPointVecType = "Vector";
      }
      return true;
   case VIEWPOINT_VECTOR_TYPE:
      mViewPointVecType = value;
      return true;
   case VIEW_DIRECTION:
      mViewDirectionName = value;
      mViewDirectionType = "Object";
      
      // Handle deprecated value "Vector"
      if (value == "Vector" || GmatStringUtil::IsNumber(value))
         mViewDirectionType = "Vector";
      
      if (value[0] == '[')
      {
         PutRvector3Value(mViewDirectionVector, id, value);
         mViewDirectionType = "Vector";
      }
      return true;
   case VIEW_DIRECTION_TYPE:
      mViewDirectionType = value;
      return true;
   case VIEW_UP_COORD_SYSTEM:
      mViewUpCoordSysName = value;
      return true;
   case VIEW_UP_AXIS:
      mViewUpAxisName = value;
      return true;
   default:
      return OrbitPlot::SetStringParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetStringParameter(const std::string &label, const std::string &value)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const std::string &label,
                                   const std::string &value)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      ("OrbitView::SetStringParameter()<%s> label=%s, value=%s \n",
       instanceName.c_str(), label.c_str(), value.c_str());
   #endif
   
   return SetStringParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const Integer id, const std::string &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const Integer id, const std::string &value,
                                   const Integer index)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      ("OrbitView::SetStringParameter()<%s> id=%d<%s>, value=%s, index= %d\n",
       instanceName.c_str(), id, GetParameterText(id).c_str(), value.c_str(), index);
   #endif
   
   switch (id)
   {
   case VIEWPOINT_REF:
      WriteDeprecatedMessage(id);
      mViewPointRefType = "Vector";
      PutRvector3Value(mViewPointRefVector, id, value, index);
      return true;
   case VIEWPOINT_REFERENCE:
      mViewPointRefType = "Vector";
      PutRvector3Value(mViewPointRefVector, id, value, index);
      return true;
   case VIEWPOINT_VECTOR:
      mViewPointVecType = "Vector";
      PutRvector3Value(mViewPointVecVector, id, value, index);
      return true;
   case VIEW_DIRECTION:
      mViewDirectionType = "Vector";
      PutRvector3Value(mViewDirectionVector, id, value, index);
      return true;
   default:
      return OrbitPlot::SetStringParameter(id, value, index);
   }
}


//------------------------------------------------------------------------------
// virtual bool SetStringParameter(const std::string &label,
//                                 const std::string &value,
//                                 const Integer index)
//------------------------------------------------------------------------------
bool OrbitView::SetStringParameter(const std::string &label,
                                   const std::string &value,
                                   const Integer index)
{
   #if DBGLVL_PARAM_STRING
   MessageInterface::ShowMessage
      ("OrbitView::SetStringParameter() label = %s, value = %s, index = %d\n",
       label.c_str(), value.c_str(), index);
   #endif
   
   return SetStringParameter(GetParameterID(label), value, index);
}


//---------------------------------------------------------------------------
//  std::string GetOnOffParameter(const Integer id) const
//---------------------------------------------------------------------------
std::string OrbitView::GetOnOffParameter(const Integer id) const
{
   switch (id)
   {
   case CELESTIAL_PLANE:
      return mEclipticPlane;
   case XY_PLANE:
      return mXYPlane;
   case WIRE_FRAME:
      return mWireFrame;
   case AXES:
      return mAxes;
   case GRID:
      return mGrid;
   case EARTH_SUN_LINES:
      return mSunLine;
   case SUN_LINE:
      return mSunLine;
   case OVERLAP_PLOT:
      return mOverlapPlot;
   case USE_INITIAL_VIEW:
      return mUseInitialView;
   case PERSPECTIVE_MODE:
      return mPerspectiveMode;
   case USE_FIXED_FOV:
      return mUseFixedFov;
   case ENABLE_STARS:
      return mEnableStars;
   case ENABLE_CONSTELLATIONS:
      return mEnableConstellations;
   default:
      return OrbitPlot::GetOnOffParameter(id);
   }
}


//------------------------------------------------------------------------------
// std::string OrbitView::GetOnOffParameter(const std::string &label) const
//------------------------------------------------------------------------------
std::string OrbitView::GetOnOffParameter(const std::string &label) const
{
   return GetOnOffParameter(GetParameterID(label));
}


//---------------------------------------------------------------------------
//  bool SetOnOffParameter(const Integer id, const std::string &value)
//---------------------------------------------------------------------------
bool OrbitView::SetOnOffParameter(const Integer id, const std::string &value)
{
   switch (id)
   {
   case CELESTIAL_PLANE:
      mEclipticPlane = value;
      return true;
   case XY_PLANE:
      mXYPlane = value;
      return true;
   case WIRE_FRAME:
      mWireFrame = value;
      return true;
   case AXES:
      mAxes = value;
      return true;
   case GRID:
      mGrid = value;
      return true;
   case EARTH_SUN_LINES:
      WriteDeprecatedMessage(id);
      mSunLine = value;
      return true;
   case SUN_LINE:
      mSunLine = value;
      return true;
   case OVERLAP_PLOT:
      mOverlapPlot = value;
      return true;
   case USE_INITIAL_VIEW:
      mUseInitialView = value;
      return true;
   case PERSPECTIVE_MODE:
      mPerspectiveMode = value;
      return true;
   case USE_FIXED_FOV:
      mUseFixedFov = value;
      return true;
   case ENABLE_STARS:
      mEnableStars = value;
      return true;
   case ENABLE_CONSTELLATIONS:
      mEnableConstellations = value;
      return true;
   default:
      return OrbitPlot::SetOnOffParameter(id, value);
   }
}


//------------------------------------------------------------------------------
// bool SetOnOffParameter(const std::string &label, const std::string &value)
//------------------------------------------------------------------------------
bool OrbitView::SetOnOffParameter(const std::string &label, 
                                  const std::string &value)
{
   return SetOnOffParameter(GetParameterID(label), value);
}


//------------------------------------------------------------------------------
// virtual std::string GetRefObjectName(const Gmat::ObjectType type) const
//------------------------------------------------------------------------------
std::string OrbitView::GetRefObjectName(const Gmat::ObjectType type) const
{
   #if DBGLVL_OBJ
   MessageInterface::ShowMessage
      ("OrbitView::GetRefObjectName() type: %s\n",
       GmatBase::GetObjectTypeString(type).c_str());
   #endif
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      return mViewCoordSysName; //just return this
   }
   
   #if DBGLVL_OBJ
   std::string msg = "type: " + GmatBase::GetObjectTypeString(type) + " not found";
   MessageInterface::ShowMessage
      ("OrbitView::GetRefObjectName() %s\n", msg.c_str());
   #endif
   
   return OrbitPlot::GetRefObjectName(type);
}


//------------------------------------------------------------------------------
// virtual bool HasRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * @see GmatBase
 */
//------------------------------------------------------------------------------
bool OrbitView::HasRefObjectTypeArray()
{
   return true;
}


//------------------------------------------------------------------------------
// const ObjectTypeArray& GetRefObjectTypeArray()
//------------------------------------------------------------------------------
/**
 * Retrieves the list of ref object types used by this class.
 *
 * @return the list of object types.
 * 
 */
//------------------------------------------------------------------------------
const ObjectTypeArray& OrbitView::GetRefObjectTypeArray()
{
   //@note  Olny leaf class clears refObjectTypes
   refObjectTypes.clear();
   refObjectTypes = OrbitPlot::GetRefObjectTypeArray();
   refObjectTypes.push_back(Gmat::COORDINATE_SYSTEM);
   return refObjectTypes;
}


//------------------------------------------------------------------------------
// virtual const StringArray& GetRefObjectNameArray(const Gmat::ObjectType type)
//------------------------------------------------------------------------------
const StringArray& OrbitView::GetRefObjectNameArray(const Gmat::ObjectType type)
{
   mAllRefObjectNames.clear();
   mAllRefObjectNames = OrbitPlot::GetRefObjectNameArray(type);
   
   // if Draw Earth-Sun lines is on, add Earth and Sun
   if (mSunLine == "On")
   {
      AddSpacePoint("Earth", mAllSpCount, false);
      AddSpacePoint("Sun", mAllSpCount, false);
   }
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      mAllRefObjectNames.push_back(mViewCoordSysName);
      mAllRefObjectNames.push_back(mViewUpCoordSysName);
   }
   else if (type == Gmat::SPACE_POINT)
   {
      mAllRefObjectNames = mAllSpNameArray;
      
      if (mViewPointRefType != "Vector")
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointRefName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointRefName);
      }
      
      if (mViewPointVecType != "Vector")
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointVecName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointVecName);
      }
      
      if (mViewDirectionType != "Vector")
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewDirectionName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewDirectionName);
      }
   }
   else if (type == Gmat::UNKNOWN_OBJECT)
   {
      #ifdef DEBUG_OBJ
      MessageInterface::ShowMessage
         ("mViewPointRefType=%s, mViewPointVecType=%s, mViewDirectionType=%s\n",
          mViewPointRefType.c_str(), mViewPointVecType.c_str(), mViewDirectionType.c_str());
      #endif
      
      mAllRefObjectNames = mAllSpNameArray;
      
      mAllRefObjectNames.push_back(mViewCoordSysName);
      
      if (mViewCoordSysName != mViewUpCoordSysName)
         mAllRefObjectNames.push_back(mViewUpCoordSysName);
      
      if (mViewPointRefType != "Vector")
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointRefName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointRefName);
      }
      
      if (mViewPointVecType != "Vector")
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewPointVecName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewPointVecName);
      }
      
      if (mViewDirectionType != "Vector")
      {
         if (find(mAllRefObjectNames.begin(), mAllRefObjectNames.end(),
                  mViewDirectionName) == mAllRefObjectNames.end())
            mAllRefObjectNames.push_back(mViewDirectionName);
      }
   }
   
   #if DBGLVL_OBJ
   MessageInterface::ShowMessage
      ("OrbitView::GetRefObjectNameArray() returning for type:%d\n", type);
   for (unsigned int i=0; i<mAllRefObjectNames.size(); i++)
      MessageInterface::ShowMessage("   %s\n", mAllRefObjectNames[i].c_str());
   #endif
   
   return mAllRefObjectNames;
}


//------------------------------------------------------------------------------
// virtual GmatBase* GetRefObject(const Gmat::ObjectType type,
//                                const std::string &name)
//------------------------------------------------------------------------------
GmatBase* OrbitView::GetRefObject(const Gmat::ObjectType type,
                                  const std::string &name)
{
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (name == mViewCoordSysName)
         return mViewCoordSystem;
      if (name == mViewUpCoordSysName)
         return mViewUpCoordSystem;
   }
   else if (type == Gmat::SPACE_POINT)
   {
      if (name == mViewPointRefName)
         return mViewPointRefObj;
      else if (name == mViewPointVecName)
         return mViewPointObj;
      else if (name == mViewDirectionName)
         return mViewDirectionObj;
   }
   
   return OrbitPlot::GetRefObject(type, name);
}


//------------------------------------------------------------------------------
// virtual bool SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
//                           const std::string &name = "")
//------------------------------------------------------------------------------
/**
 * Set reference object pointer.
 *
 * @param <obj>  Reference object pointer to set to given object type and name
 * @param <type> Reference object type
 * @param <name> Reference object name
 */
//------------------------------------------------------------------------------
bool OrbitView::SetRefObject(GmatBase *obj, const Gmat::ObjectType type,
                             const std::string &name)
{
   #if DBGLVL_OBJ
   MessageInterface::ShowMessage
      ("OrbitView::SetRefObject() this=<%p>'%s', obj=<%p>'%s', type=%d[%s], name='%s'\n",
       this, GetName().c_str(), obj, obj->GetName().c_str(), type,
       obj->GetTypeName().c_str(), name.c_str());
   #endif
   
   std::string realName = name;
   if (name == "")
      realName = obj->GetName();
   
   if (type == Gmat::COORDINATE_SYSTEM)
   {
      if (realName == mViewCoordSysName)
         mViewCoordSystem = (CoordinateSystem*)obj;
      if (realName == mViewUpCoordSysName)
         mViewUpCoordSystem = (CoordinateSystem*)obj;
      return true;
   }
   
   
   if (obj->IsOfType(Gmat::SPACE_POINT))
   {
      #if DBGLVL_OBJ
      MessageInterface::ShowMessage("   Setting View information...\n");
      #endif
      
      // ViewPoint info
      if (realName == mViewPointRefName)
         mViewPointRefObj = (SpacePoint*)obj;
      
      if (realName == mViewPointVecName)
         mViewPointObj = (SpacePoint*)obj;
      
      if (realName == mViewDirectionName)
         mViewDirectionObj = (SpacePoint*)obj;
      
      #if DBGLVL_OBJ
      MessageInterface::ShowMessage
         ("OrbitView::SetRefObject() mViewPointRefObj=<%p>, mViewPointObj=<%p>, "
          "mViewDirectionObj=<%p>\n", mViewPointRefObj, mViewPointObj,
          mViewDirectionObj);
      #endif
   }
   
   return OrbitPlot::SetRefObject(obj, type, realName);
}


//---------------------------------
// protected methods
//---------------------------------

//------------------------------------------------------------------------------
// void PutRvector3Value(Rvector3 &rvec3, Integer id,
//                       const std::string &sval, Integer index = -1);
//------------------------------------------------------------------------------
/*
 * Converts input string to Real and store as Rvector3 element at idnex.
 *
 * @param rvec3  input Rvector3 where value to be stored
 * @param id     input Parameter ID used for formating error message
 * @param sval   input string value
 * @param index  input index to be used for storing a Rvector3 element
 *               if index is -1, whole 3 elements are converted and stored
 *               from a string format of "[element1 element2 element3]"
 */
//------------------------------------------------------------------------------
void OrbitView::PutRvector3Value(Rvector3 &rvec3, Integer id,
                                  const std::string &sval, Integer index)
{
   #if DBGLVL_PARAM_RVEC3
   MessageInterface::ShowMessage
      ("OrbitView::PutRvector3Value() id=%d, sval=%s, index=%d\n",
       id, sval.c_str(), index);
   #endif
   
   std::string badVal;
   bool isValid = true;
   std::string field = GetParameterText(id);
   
   // Check index, throw exception if out of bound
   if (index < -1 || index > 2)
   {
      badVal = sval;
      isValid = false;
   }
   
   if (isValid)
   {
      // Convert string value to Real
      if (index != -1)
      {
         Real rval;
         if (GmatStringUtil::ToReal(sval, rval))
            rvec3[index] = rval;
         else
         {
            isValid = false;
            badVal = sval;
         }
      }
      else if (index == -1)
      {
         StringArray valArray;
         std::string svalue = sval;
         svalue = GmatStringUtil::Trim(svalue);
         std::string::size_type index1 = svalue.find_first_of("[");
         if (index1 != svalue.npos)
         {
            std::string::size_type index2 = svalue.find_last_of("]");
            if (index2 != svalue.npos)
            {
               svalue = svalue.substr(index1+1, index2-index1-1);
            }
            else
            {
               isValid = false;
               badVal = sval;
            }
         }
         
         valArray = GmatStringUtil::SeparateBy(svalue, " ,");         
         int arraySize = valArray.size();
         
         if (arraySize == 3)
         {
            Real rvals[3];
            bool rvalsOk[3];
            rvalsOk[0] = GmatStringUtil::ToReal(valArray[0], rvals[0]);
            rvalsOk[1] = GmatStringUtil::ToReal(valArray[1], rvals[1]);
            rvalsOk[2] = GmatStringUtil::ToReal(valArray[2], rvals[2]);
            
            // Detects first invalid input and throw exception
            if (!rvalsOk[0])
               badVal = valArray[0];
            else if (!rvalsOk[1])
               badVal = valArray[1];
            else if (!rvalsOk[2])
               badVal = valArray[2];
            
            if (rvalsOk[0] && rvalsOk[1] && rvalsOk[2])
               rvec3.Set(rvals[0], rvals[1], rvals[2]);
            else
               isValid = false;
         }
         else
         {
            isValid = false;
            badVal = sval;
         }
      }
   }
   
   if (!isValid)
   {
      SubscriberException se;
      se.SetDetails(errorMessageFormat.c_str(), badVal.c_str(), field.c_str(),
                    "SpacecraftName, CelestialBodyName, LibrationPointName, "
                    "BarycenterName, or a 3-vector of numerical values");
      throw se;
   }
}


//------------------------------------------------------------------------------
// void WriteDeprecatedMessage(Integer id) const
//------------------------------------------------------------------------------
void OrbitView::WriteDeprecatedMessage(Integer id) const
{
   // Write only one message per session
   static bool writeEarthSunLines = true;
   static bool writeViewpointRef = true;
   static bool writeViewpointRefVector = true;
   static bool writeViewpointVectorVector = true;
   static bool writeViewDirectionVector = true;
   
   switch (id)
   {
   case EARTH_SUN_LINES:
      if (writeEarthSunLines)
      {
         MessageInterface::ShowMessage
            ("*** WARNING *** \"EarthSunLines\" is deprecated and will be "
             "removed from a future build; please use \"SunLine\" "
             "instead.\n");
         writeEarthSunLines = false;
      }
      break;
   case VIEWPOINT_REF:
      if (writeViewpointRef)
      {
         MessageInterface::ShowMessage
            ("*** WARNING *** \"ViewPointRef\" is deprecated and will be "
             "removed from a future build; please use \"ViewPointReference\" "
             "instead.\n");
         writeViewpointRef = false;
      }
      break;
   case VIEWPOINT_REF_VECTOR:
      if (writeViewpointRefVector)
      {
         MessageInterface::ShowMessage
            ("*** WARNING *** \"ViewPointRefVector\" is deprecated and will be "
             "removed from a future build.\n");
         writeViewpointRefVector = false;
      }
      break;
   case VIEWPOINT_VECTOR_VECTOR:
      if (writeViewpointVectorVector)
      {
         MessageInterface::ShowMessage
            ("*** WARNING *** \"ViewPointVectorVector\" is deprecated and will be "
             "removed from a future build.\n");
         writeViewpointVectorVector = false;
      }
      break;
   case VIEW_DIRECTION_VECTOR:
      if (writeViewDirectionVector)
      {
         MessageInterface::ShowMessage
            ("*** WARNING *** \"ViewDirectionVector\" is deprecated and will be "
             "removed from a future build.\n");
         writeViewDirectionVector = false;
      }
      break;
   default:
      break;
   }
}


//------------------------------------------------------------------------------
// bool UpdateSolverData()
//------------------------------------------------------------------------------
bool OrbitView::UpdateSolverData()
{
   int size = mCurrEpochArray.size();
   int last = size - 1;
   
   #if DBGLVL_SOLVER_CURRENT_ITER
   MessageInterface::ShowMessage("===> num buffered data = %d\n", size);
   MessageInterface::ShowMessage("==========> now update solver plot\n");
   #endif
   
   if (size == 0)
      return true;
   
   UnsignedIntArray colorArray = mScOrbitColorArray;
   if (runstate == Gmat::SOLVING)
      colorArray = mScTargetColorArray;
   else
      colorArray = mScOrbitColorArray;
   
   // Update plot with last iteration data
   for (int i=0; i<size-1; i++)
   {
      #if DBGLVL_SOLVER_CURRENT_ITER > 1
      for (int sc=0; sc<mScCount; sc++)
         MessageInterface::ShowMessage
            ("   i=%d, sc=%d, solver epoch = %f, X,Y,Z = %f, %f, %f\n", i, sc,
             mCurrEpochArray[i], mCurrXArray[i][sc], mCurrYArray[i][sc],
             mCurrZArray[i][sc]);
      #endif
      
      // Just buffer data up to last point - 1
      PlotInterface::
         UpdateGlPlot(instanceName, mOldName, mCurrScArray[i],
                      mCurrEpochArray[i], mCurrXArray[i], mCurrYArray[i],
                      mCurrZArray[i], mCurrVxArray[i], mCurrVyArray[i],
                      mCurrVzArray[i], colorArray, true, mSolverIterOption, false);
   }
   
   // Buffer last point and Update the plot
   PlotInterface::
      UpdateGlPlot(instanceName, mOldName, mCurrScArray[last],
                   mCurrEpochArray[last], mCurrXArray[last], mCurrYArray[last],
                   mCurrZArray[last], mCurrVxArray[last], mCurrVyArray[last],
                   mCurrVzArray[last], colorArray, true, mSolverIterOption, true);
   
   // clear arrays
   mCurrScArray.clear();
   mCurrEpochArray.clear();
   mCurrXArray.clear();
   mCurrYArray.clear();
   mCurrZArray.clear();
   mCurrVxArray.clear();
   mCurrVyArray.clear();
   mCurrVzArray.clear();
   
   if (runstate == Gmat::SOLVING)
      PlotInterface::TakeGlAction(instanceName, "ClearSolverData");
   
   return true;
}


//--------------------------------------
// methods inherited from Subscriber
//--------------------------------------

//------------------------------------------------------------------------------
// bool Distribute(int len)
//------------------------------------------------------------------------------
bool OrbitView::Distribute(int len)
{
   //loj: How do I convert data to Real data?
   return false;
}


//------------------------------------------------------------------------------
// bool Distribute(const Real *dat, Integer len)
//------------------------------------------------------------------------------
bool OrbitView::Distribute(const Real *dat, Integer len)
{
   #if DBGLVL_UPDATE
   MessageInterface::ShowMessage
      ("===========================================================================\n"
       "OrbitView::Distribute() instanceName=%s, active=%d, isEndOfRun=%d, "
       "isEndOfReceive=%d\n   mAllSpCount=%d, mScCount=%d, len=%d, runstate=%d\n",
       instanceName.c_str(), active, isEndOfRun, isEndOfReceive, mAllSpCount,
       mScCount, len, runstate);
   #endif
   
   if (GmatGlobal::Instance()->GetRunMode() == GmatGlobal::TESTING_NO_PLOTS)
      return true;
   
   if (!active || mScCount <= 0)
      return true;
   
   // test isEndOfRun first
   if (isEndOfRun)
      return PlotInterface::SetGlEndOfRun(instanceName);
   
   if (isEndOfReceive)
   {
      if ((mSolverIterOption == SI_CURRENT) &&
          (runstate == Gmat::SOLVING || runstate == Gmat::SOLVEDPASS))
      {
         UpdateSolverData();
      }
      else
      {
         return PlotInterface::RefreshGlPlot(instanceName);
      }
   }
   
   
   if (len <= 0)
      return true;
   
   
   #if DBGLVL_DATA
   MessageInterface::ShowMessage("%s, len=%d\n", GetName().c_str(), len);
   for (int i=0; i<len; i++)
      MessageInterface::ShowMessage("%.11f  ", dat[i]);
   MessageInterface::ShowMessage("\n");
   #endif
   
   //------------------------------------------------------------
   // if targeting and draw target is None, just return
   //------------------------------------------------------------
   if ((mSolverIterOption == SI_NONE) && (runstate == Gmat::SOLVING))
   {
      #if DBGLVL_UPDATE > 1
      MessageInterface::ShowMessage
         ("   Just returning: SolverIterations is %d and runstate is %d\n",
          mSolverIterOption, runstate);
      #endif
      
      return true;
   }
   
   //------------------------------------------------------------
   // update plot data
   //------------------------------------------------------------
   
   CoordinateConverter coordConverter;
   mNumData++;
   
   #if DBGLVL_UPDATE > 1
   MessageInterface::ShowMessage
      ("   mNumData=%d, mDataCollectFrequency=%d, currentProvider=<%p>\n",
       mNumData, mDataCollectFrequency, currentProvider);
   #endif
   
   if ((mNumData % mDataCollectFrequency) == 0)
   {
      mNumData = 0;
      mNumCollected++;
      bool update = (mNumCollected % mUpdatePlotFrequency) == 0;
      
      #if DBGLVL_UPDATE > 1
      MessageInterface::ShowMessage
         ("   currentProvider=%d, theDataLabels.size()=%d\n",
          currentProvider, theDataLabels.size());
      #endif
      
      #if DBGLVL_UPDATE > 2
      MessageInterface::ShowMessage
         ("OrbitView::Distribute() Using new Publisher code\n");
      #endif
      
      // @note
      // New Publisher code doesn't assign currentProvider anymore,
      // it just copies current labels. There was an issue with
      // provider id keep incrementing if data is regisgered and
      // published inside a GmatFunction
      StringArray dataLabels = theDataLabels[0];
      
      #if DBGLVL_DATA_LABELS
      MessageInterface::ShowMessage("   Data labels for %s =\n   ", GetName().c_str());
      for (int j=0; j<(int)dataLabels.size(); j++)
         MessageInterface::ShowMessage("%s ", dataLabels[j].c_str());
      MessageInterface::ShowMessage("\n");
      #endif
      
      Integer idX, idY, idZ;
      Integer idVx, idVy, idVz;
      Integer scIndex = -1;
      
      for (int i=0; i<mScCount; i++)
      {
         idX = FindIndexOfElement(dataLabels, mScNameArray[i]+".X");
         idY = FindIndexOfElement(dataLabels, mScNameArray[i]+".Y");
         idZ = FindIndexOfElement(dataLabels, mScNameArray[i]+".Z");
         
         idVx = FindIndexOfElement(dataLabels, mScNameArray[i]+".Vx");
         idVy = FindIndexOfElement(dataLabels, mScNameArray[i]+".Vy");
         idVz = FindIndexOfElement(dataLabels, mScNameArray[i]+".Vz");
         
         #if DBGLVL_DATA_LABELS
         MessageInterface::ShowMessage
            ("   mScNameArray[%d]=%s, idX=%d, idY=%d, idZ=%d, idVx=%d, idVy=%d, idVz=%d\n",
             i, mScNameArray[i].c_str(), idX, idY, idZ, idVx, idVy, idVz);
         #endif
         
         // if any of index not found, continue with next spacecraft name
         if (idX  == -1 || idY  == -1 || idZ  == -1 ||
             idVx == -1 || idVy == -1 || idVz == -1)
            continue;
         
         scIndex++;
         
         // buffer data
         for (int sc=0; sc<mScCount; sc++)
         {
            // If distributed data coordinate system is different from view
            // coordinate system, convert data here.
            // if we convert after current epoch, it will not give correct
            // results, if origin is spacecraft,
            // ie, sat->GetMJ2000State(epoch) will not give correct results.
            
            #if DBGLVL_DATA
            MessageInterface::ShowMessage
               ("   %s, %.11f, X,Y,Z = %f, %f, %f\n", GetName().c_str(), dat[0],
                dat[idX], dat[idY], dat[idZ]);
            #endif
            
            if ((theDataCoordSystem != NULL && mViewCoordSystem != NULL) &&
                (mViewCoordSystem != theDataCoordSystem))
            {
               Rvector6 inState, outState;
               
               // convert position and velocity
               inState.Set(dat[idX], dat[idY], dat[idZ],
                           dat[idVx], dat[idVy], dat[idVz]);
               
               coordConverter.Convert(dat[0], inState, theDataCoordSystem,
                                      outState, mViewCoordSystem);
               
               mScXArray[scIndex] = outState[0];
               mScYArray[scIndex] = outState[1];
               mScZArray[scIndex] = outState[2];
               mScVxArray[scIndex] = outState[3];
               mScVyArray[scIndex] = outState[4];
               mScVzArray[scIndex] = outState[5];
            }
            else
            {
               mScXArray[scIndex] = dat[idX];
               mScYArray[scIndex] = dat[idY];
               mScZArray[scIndex] = dat[idZ];
               mScVxArray[scIndex] = dat[idVx];
               mScVyArray[scIndex] = dat[idVy];
               mScVzArray[scIndex] = dat[idVz];
            }
            
            #if DBGLVL_DATA
            MessageInterface::ShowMessage
               ("   after buffering, scNo=%d, scIndex=%d, X,Y,Z = %f, %f, %f\n",
                i, scIndex, mScXArray[scIndex], mScYArray[scIndex], mScZArray[scIndex]);
            #endif
            
            #if DBGLVL_DATA > 1
            MessageInterface::ShowMessage
               ("   Vx,Vy,Vz = %f, %f, %f\n",
                mScVxArray[scIndex], mScVyArray[scIndex], mScVzArray[scIndex]);
            #endif
         }
      }
      
      // if only showing current iteration, buffer data and return
      if (mSolverIterOption == SI_CURRENT)
      {
         // save data when targeting or last iteration
         if (runstate == Gmat::SOLVING || runstate == Gmat::SOLVEDPASS)
         {
            mCurrScArray.push_back(mScNameArray);
            mCurrEpochArray.push_back(dat[0]);
            mCurrXArray.push_back(mScXArray);
            mCurrYArray.push_back(mScYArray);
            mCurrZArray.push_back(mScZArray);
            mCurrVxArray.push_back(mScVxArray);
            mCurrVyArray.push_back(mScVyArray);
            mCurrVzArray.push_back(mScVzArray);
         }
         
         if (runstate == Gmat::SOLVING)
         {
            //MessageInterface::ShowMessage
            //   ("=====> num buffered = %d\n", mCurrEpochArray.size());
            return true;
         }
      }
      
      
      #if DBGLVL_UPDATE > 0
      MessageInterface::ShowMessage("==========> now update 3D View\n");
      #endif
      
      bool solving = false;
      UnsignedIntArray colorArray = mScOrbitColorArray;
      if (runstate == Gmat::SOLVING)
      {
         solving = true;
         colorArray = mScTargetColorArray;
      }

      bool inFunction = false;
      if (currentProvider && currentProvider->TakeAction("IsInFunction"))
         inFunction = true;
      
      PlotInterface::
         UpdateGlPlot(instanceName, mOldName, mScNameArray,
                      dat[0], mScXArray, mScYArray, mScZArray,
                      mScVxArray, mScVyArray, mScVzArray,
                      colorArray, solving, mSolverIterOption, update, inFunction);
      
      if (update)
         mNumCollected = 0;
   }
   
   //loj: always return true otherwise next subscriber will not call ReceiveData()
   //     in Publisher::Publish()
   return true;
}
