

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Wed Jan 21 10:49:08 2026
 */
/* Compiler settings for iHR320.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __iHR320_h_h__
#define __iHR320_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IiHR320_FWD_DEFINED__
#define __IiHR320_FWD_DEFINED__
typedef interface IiHR320 IiHR320;

#endif 	/* __IiHR320_FWD_DEFINED__ */


#ifndef __iHR320_FWD_DEFINED__
#define __iHR320_FWD_DEFINED__

#ifdef __cplusplus
typedef class iHR320 iHR320;
#else
typedef struct iHR320 iHR320;
#endif /* __cplusplus */

#endif 	/* __iHR320_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __iHR320_LIBRARY_DEFINED__
#define __iHR320_LIBRARY_DEFINED__

/* library iHR320 */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_iHR320;

#ifndef __IiHR320_DISPINTERFACE_DEFINED__
#define __IiHR320_DISPINTERFACE_DEFINED__

/* dispinterface IiHR320 */
/* [uuid] */ 


EXTERN_C const IID DIID_IiHR320;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("52AA4D6E-4383-4CFF-90FB-8886361BD9C2")
    IiHR320 : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IiHR320Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IiHR320 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IiHR320 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IiHR320 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IiHR320 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IiHR320 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IiHR320 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IiHR320 * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IiHR320Vtbl;

    interface IiHR320
    {
        CONST_VTBL struct IiHR320Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IiHR320_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IiHR320_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IiHR320_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IiHR320_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IiHR320_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IiHR320_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IiHR320_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IiHR320_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_iHR320;

#ifdef __cplusplus

class DECLSPEC_UUID("C0219464-4B1D-4583-B89B-2E65F8439BBB")
iHR320;
#endif
#endif /* __iHR320_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


