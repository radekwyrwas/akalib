/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.9
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.kalotay.akalib;

public class Duration {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected Duration(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Duration obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        AkaApiJNI.delete_Duration(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public Duration(double d, double c) {
    this(AkaApiJNI.new_Duration(d, c), true);
  }

  public void setDuration(double value) {
    AkaApiJNI.Duration_duration_set(swigCPtr, this, value);
  }

  public double getDuration() {
    return AkaApiJNI.Duration_duration_get(swigCPtr, this);
  }

  public void setConvexity(double value) {
    AkaApiJNI.Duration_convexity_set(swigCPtr, this, value);
  }

  public double getConvexity() {
    return AkaApiJNI.Duration_convexity_get(swigCPtr, this);
  }

}
