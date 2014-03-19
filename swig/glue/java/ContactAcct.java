/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.opentransactions.otapi;

public class ContactAcct extends Displayable {
  private long swigCPtr;

  protected ContactAcct(long cPtr, boolean cMemoryOwn) {
    super(otapiJNI.ContactAcct_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(ContactAcct obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        otapiJNI.delete_ContactAcct(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }
/*@SWIG:otapi/OTAPI.i,158,OT_CAN_BE_CONTAINED_BY@*/
	// Ensure that the GC doesn't collect any OT_CONTAINER instance set from Java
	private Contact containerRefContact;
	// ----------------	
	protected void addReference(Contact theContainer) {  // This is Java code
		containerRefContact = theContainer;
	}
	// ----------------
/*@SWIG@*/
	// ------------------------
  public void setGui_label(String value) {
    otapiJNI.ContactAcct_gui_label_set(swigCPtr, this, value);
  }

  public String getGui_label() {
    return otapiJNI.ContactAcct_gui_label_get(swigCPtr, this);
  }

  public void setServer_type(String value) {
    otapiJNI.ContactAcct_server_type_set(swigCPtr, this, value);
  }

  public String getServer_type() {
    return otapiJNI.ContactAcct_server_type_get(swigCPtr, this);
  }

  public void setServer_id(String value) {
    otapiJNI.ContactAcct_server_id_set(swigCPtr, this, value);
  }

  public String getServer_id() {
    return otapiJNI.ContactAcct_server_id_get(swigCPtr, this);
  }

  public void setAsset_type_id(String value) {
    otapiJNI.ContactAcct_asset_type_id_set(swigCPtr, this, value);
  }

  public String getAsset_type_id() {
    return otapiJNI.ContactAcct_asset_type_id_get(swigCPtr, this);
  }

  public void setAcct_id(String value) {
    otapiJNI.ContactAcct_acct_id_set(swigCPtr, this, value);
  }

  public String getAcct_id() {
    return otapiJNI.ContactAcct_acct_id_get(swigCPtr, this);
  }

  public void setNym_id(String value) {
    otapiJNI.ContactAcct_nym_id_set(swigCPtr, this, value);
  }

  public String getNym_id() {
    return otapiJNI.ContactAcct_nym_id_get(swigCPtr, this);
  }

  public void setMemo(String value) {
    otapiJNI.ContactAcct_memo_set(swigCPtr, this, value);
  }

  public String getMemo() {
    return otapiJNI.ContactAcct_memo_get(swigCPtr, this);
  }

  public void setPublic_key(String value) {
    otapiJNI.ContactAcct_public_key_set(swigCPtr, this, value);
  }

  public String getPublic_key() {
    return otapiJNI.ContactAcct_public_key_get(swigCPtr, this);
  }

  public static ContactAcct ot_dynamic_cast(Storable pObject) {
    long cPtr = otapiJNI.ContactAcct_ot_dynamic_cast(Storable.getCPtr(pObject), pObject);
    return (cPtr == 0) ? null : new ContactAcct(cPtr, false);
  }

}
