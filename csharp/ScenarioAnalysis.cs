/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.9
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace AndrewKalotayAssociates {

using System;
using System.Runtime.InteropServices;

public class ScenarioAnalysis : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal ScenarioAnalysis(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(ScenarioAnalysis obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~ScenarioAnalysis() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          AkaApiPINVOKE.delete_ScenarioAnalysis(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public ScenarioAnalysis.REDEEMED redeemed {
    set {
      AkaApiPINVOKE.ScenarioAnalysis_redeemed_set(swigCPtr, (int)value);
    } 
    get {
      ScenarioAnalysis.REDEEMED ret = (ScenarioAnalysis.REDEEMED)AkaApiPINVOKE.ScenarioAnalysis_redeemed_get(swigCPtr);
      return ret;
    } 
  }

  public int flowwhen {
    set {
      AkaApiPINVOKE.ScenarioAnalysis_flowwhen_set(swigCPtr, value);
    } 
    get {
      int ret = AkaApiPINVOKE.ScenarioAnalysis_flowwhen_get(swigCPtr);
      return ret;
    } 
  }

  public double efficiency {
    set {
      AkaApiPINVOKE.ScenarioAnalysis_efficiency_set(swigCPtr, value);
    } 
    get {
      double ret = AkaApiPINVOKE.ScenarioAnalysis_efficiency_get(swigCPtr);
      return ret;
    } 
  }

  public double reinvestment {
    set {
      AkaApiPINVOKE.ScenarioAnalysis_reinvestment_set(swigCPtr, value);
    } 
    get {
      double ret = AkaApiPINVOKE.ScenarioAnalysis_reinvestment_get(swigCPtr);
      return ret;
    } 
  }

  public static string redeemstring(ScenarioAnalysis.REDEEMED arg0) {
    string ret = AkaApiPINVOKE.ScenarioAnalysis_redeemstring__SWIG_0((int)arg0);
    return ret;
  }

  public string redeemstring() {
    string ret = AkaApiPINVOKE.ScenarioAnalysis_redeemstring__SWIG_1(swigCPtr);
    return ret;
  }

  public ScenarioAnalysis() : this(AkaApiPINVOKE.new_ScenarioAnalysis(), true) {
  }

  public enum REDEEMED {
    NOTREDEEMED,
    CALL,
    PUT,
    SINK,
    MATURITY
  }

}

}
