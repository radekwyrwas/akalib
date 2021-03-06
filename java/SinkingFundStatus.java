/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.9
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.kalotay.akalib;

public class SinkingFundStatus {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected SinkingFundStatus(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(SinkingFundStatus obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        AkaApiJNI.delete_SinkingFundStatus(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setOutstanding(double value) {
    AkaApiJNI.SinkingFundStatus_outstanding_set(swigCPtr, this, value);
  }

  public double getOutstanding() {
    return AkaApiJNI.SinkingFundStatus_outstanding_get(swigCPtr, this);
  }

  public void setAccumulation(double value) {
    AkaApiJNI.SinkingFundStatus_accumulation_set(swigCPtr, this, value);
  }

  public double getAccumulation() {
    return AkaApiJNI.SinkingFundStatus_accumulation_get(swigCPtr, this);
  }

  public SinkingFundStatus(long os, long ac) {
    this(AkaApiJNI.new_SinkingFundStatus__SWIG_0(os, ac), true);
  }

  public SinkingFundStatus(double os, double ac) {
    this(AkaApiJNI.new_SinkingFundStatus__SWIG_1(os, ac), true);
  }

  public SinkingFundStatus() {
    this(AkaApiJNI.new_SinkingFundStatus__SWIG_2(), true);
  }

}
