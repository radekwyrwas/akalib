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

public class Date : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Date(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(Date obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~Date() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          AkaApiPINVOKE.delete_Date(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public Date(int arg0, int arg1, int arg2, Date.ENTRY arg3) : this(AkaApiPINVOKE.new_Date__SWIG_0(arg0, arg1, arg2, (int)arg3), true) {
  }

  public Date(int arg0, int arg1, int arg2) : this(AkaApiPINVOKE.new_Date__SWIG_1(arg0, arg1, arg2), true) {
  }

  public Date(string arg0, Date.ENTRY arg1, char sep) : this(AkaApiPINVOKE.new_Date__SWIG_2(arg0, (int)arg1, sep), true) {
  }

  public Date(string arg0, Date.ENTRY arg1) : this(AkaApiPINVOKE.new_Date__SWIG_3(arg0, (int)arg1), true) {
  }

  public Date(int arg0) : this(AkaApiPINVOKE.new_Date__SWIG_4(arg0), true) {
  }

  public Date() : this(AkaApiPINVOKE.new_Date__SWIG_5(), true) {
  }

  public Date(Date arg0, double yrs, Bond.DAYCOUNT arg2) : this(AkaApiPINVOKE.new_Date__SWIG_6(Date.getCPtr(arg0), yrs, (int)arg2), true) {
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool Good() {
    bool ret = AkaApiPINVOKE.Date_Good(swigCPtr);
    return ret;
  }

  public int Libdate() {
    int ret = AkaApiPINVOKE.Date_Libdate(swigCPtr);
    return ret;
  }

  public int DayOf() {
    int ret = AkaApiPINVOKE.Date_DayOf(swigCPtr);
    return ret;
  }

  public int MonthOf() {
    int ret = AkaApiPINVOKE.Date_MonthOf(swigCPtr);
    return ret;
  }

  public int YearOf() {
    int ret = AkaApiPINVOKE.Date_YearOf(swigCPtr);
    return ret;
  }

  public double YearsTo(Date other, Bond.DAYCOUNT arg1) {
    double ret = AkaApiPINVOKE.Date_YearsTo(swigCPtr, Date.getCPtr(other), (int)arg1);
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int DaysTo(Date other) {
    int ret = AkaApiPINVOKE.Date_DaysTo(swigCPtr, Date.getCPtr(other));
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Date PlusEqual(int arg0) {
    Date ret = new Date(AkaApiPINVOKE.Date_PlusEqual(swigCPtr, arg0), false);
    return ret;
  }

  public Date MinusEqual(int days) {
    Date ret = new Date(AkaApiPINVOKE.Date_MinusEqual(swigCPtr, days), false);
    return ret;
  }

  public bool IsLT(Date other) {
    bool ret = AkaApiPINVOKE.Date_IsLT(swigCPtr, Date.getCPtr(other));
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool IsLE(Date other) {
    bool ret = AkaApiPINVOKE.Date_IsLE(swigCPtr, Date.getCPtr(other));
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool IsEq(Date other) {
    bool ret = AkaApiPINVOKE.Date_IsEq(swigCPtr, Date.getCPtr(other));
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool IsGE(Date other) {
    bool ret = AkaApiPINVOKE.Date_IsGE(swigCPtr, Date.getCPtr(other));
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool IsGT(Date other) {
    bool ret = AkaApiPINVOKE.Date_IsGT(swigCPtr, Date.getCPtr(other));
    if (AkaApiPINVOKE.SWIGPendingException.Pending) throw AkaApiPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public enum ENTRY {
    YMD,
    MDY,
    DMY
  }

}

}