/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.9
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.kalotay.akalib;

public class Quote {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected Quote(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Quote obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        AkaApiJNI.delete_Quote(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void setQuote(double value) {
    AkaApiJNI.Quote_quote_set(swigCPtr, this, value);
  }

  public double getQuote() {
    return AkaApiJNI.Quote_quote_get(swigCPtr, this);
  }

  public void setQuotetype(Quote.QuoteType value) {
    AkaApiJNI.Quote_quotetype_set(swigCPtr, this, value.swigValue());
  }

  public Quote.QuoteType getQuotetype() {
    return Quote.QuoteType.swigToEnum(AkaApiJNI.Quote_quotetype_get(swigCPtr, this));
  }

  public Quote(double q, Quote.QuoteType qt) {
    this(AkaApiJNI.new_Quote(q, qt.swigValue()), true);
  }

  public final static class QuoteType {
    public final static Quote.QuoteType OAS = new Quote.QuoteType("OAS");
    public final static Quote.QuoteType PRICE = new Quote.QuoteType("PRICE");
    public final static Quote.QuoteType YTM = new Quote.QuoteType("YTM");
    public final static Quote.QuoteType YTC = new Quote.QuoteType("YTC");
    public final static Quote.QuoteType YTP = new Quote.QuoteType("YTP");

    public final int swigValue() {
      return swigValue;
    }

    public String toString() {
      return swigName;
    }

    public static QuoteType swigToEnum(int swigValue) {
      if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
        return swigValues[swigValue];
      for (int i = 0; i < swigValues.length; i++)
        if (swigValues[i].swigValue == swigValue)
          return swigValues[i];
      throw new IllegalArgumentException("No enum " + QuoteType.class + " with value " + swigValue);
    }

    private QuoteType(String swigName) {
      this.swigName = swigName;
      this.swigValue = swigNext++;
    }

    private QuoteType(String swigName, int swigValue) {
      this.swigName = swigName;
      this.swigValue = swigValue;
      swigNext = swigValue+1;
    }

    private QuoteType(String swigName, QuoteType swigEnum) {
      this.swigName = swigName;
      this.swigValue = swigEnum.swigValue;
      swigNext = this.swigValue+1;
    }

    private static QuoteType[] swigValues = { OAS, PRICE, YTM, YTC, YTP };
    private static int swigNext = 0;
    private final int swigValue;
    private final String swigName;
  }

}
