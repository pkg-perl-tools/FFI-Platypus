#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

#include "ffi_platypus.h"

#ifndef HAVE_IV_IS_64
#include "perl_math_int64.h"
#endif

void
ffi_pl_closure_call(ffi_cif *ffi_cif, void *result, void **arguments, void *user)
{
  dSP;

  ffi_pl_closure *closure = (ffi_pl_closure*) user;
  ffi_pl_type_extra_closure *extra = &closure->type->extra[0].closure;
  int flags = extra->flags;
  int i;
  int count;
  SV *sv;

  if(!(flags & G_NOARGS))
  {
    ENTER;
    SAVETMPS;
  }

  PUSHMARK(SP);

  if(!(flags & G_NOARGS))
  {
    for(i=0; i< ffi_cif->nargs; i++)
    {
      if(extra->argument_types[i]->platypus_type == FFI_PL_FFI)
      {
        switch(extra->argument_types[i]->ffi_type->type)
        {
          case FFI_TYPE_VOID:
            break;
          case FFI_TYPE_UINT8:
            sv = sv_newmortal();
            sv_setuv(sv, *((uint8_t*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_SINT8:
            sv = sv_newmortal();
            sv_setiv(sv, *((int8_t*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_UINT16:
            sv = sv_newmortal();
            sv_setuv(sv, *((uint16_t*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_SINT16:
            sv = sv_newmortal();
            sv_setiv(sv, *((int16_t*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_UINT32:
            sv = sv_newmortal();
            sv_setuv(sv, *((uint32_t*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_SINT32:
            sv = sv_newmortal();
            sv_setiv(sv, *((int32_t*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_UINT64:
            sv = sv_newmortal();
#ifdef HAVE_IV_IS_64
            sv_setuv(sv, *((uint64_t*)arguments[i]));
#else
            sv_setu64(sv, *((uint64_t*)arguments[i]));
#endif
            XPUSHs(sv);
            break;
          case FFI_TYPE_SINT64:
            sv = sv_newmortal();
#ifdef HAVE_IV_IS_64
            sv_setiv(sv, *((int64_t*)arguments[i]));
#else
            sv_seti64(sv, *((int64_t*)arguments[i]));
#endif
            XPUSHs(sv);
            break;
          case FFI_TYPE_FLOAT:
            sv = sv_newmortal();
            sv_setnv(sv, *((float*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_DOUBLE:
            sv = sv_newmortal();
            sv_setnv(sv, *((double*)arguments[i]));
            XPUSHs(sv);
            break;
          case FFI_TYPE_POINTER:
            sv = sv_newmortal();
            if( *((void**)arguments[i]) != NULL)
              sv_setiv(sv, PTR2IV( *((void**)arguments[i]) ));
            XPUSHs(sv);
            break;
        }
      }
      else if(extra->argument_types[i]->platypus_type == FFI_PL_STRING)
      {
        sv = sv_newmortal();
        if( *((char**)arguments[i]) != NULL)
          sv_setpv(sv, *((char**)arguments[i]));
        XPUSHs(sv);
      }
    }
    PUTBACK;
  }

  /* TODO: what to do in the event of die */
  count = call_sv(closure->coderef, flags);

  if(!(flags & G_DISCARD))
  {
    SPAGAIN;

    if(count != 1)
      sv = &PL_sv_undef;
    else
      sv = POPs;

    if(extra->return_type->platypus_type == FFI_PL_FFI)
    {
      switch(extra->return_type->ffi_type->type)
      {
        case FFI_TYPE_UINT8:
          *((uint8_t*)result) = SvUV(sv);
          break;
        case FFI_TYPE_SINT8:
          *((int8_t*)result) = SvIV(sv);
          break;
        case FFI_TYPE_UINT16:
          *((uint16_t*)result) = SvUV(sv);
          break;
        case FFI_TYPE_SINT16:
          *((int16_t*)result) = SvIV(sv);
          break;
        case FFI_TYPE_UINT32:
          *((uint32_t*)result) = SvUV(sv);
          break;
        case FFI_TYPE_SINT32:
          *((int32_t*)result) = SvIV(sv);
          break;
        case FFI_TYPE_UINT64:
#ifdef HAVE_IV_IS_64
          *((uint64_t*)result) = SvUV(sv);
#else
          *((uint64_t*)result) = SvU64(sv);
#endif
          break;
        case FFI_TYPE_SINT64:
#ifdef HAVE_IV_IS_64
          *((int64_t*)result) = SvIV(sv);
#else
          *((int64_t*)result) = SvI64(sv);
#endif
          break;
        case FFI_TYPE_FLOAT:
          *((float*)result) = SvNV(sv);
          break;
        case FFI_TYPE_DOUBLE:
          *((double*)result) = SvNV(sv);
          break;
        case FFI_TYPE_POINTER:
          *((void**)result) = SvOK(sv) ? INT2PTR(void*, SvIV(sv)) : NULL;
          break;
      }
    }

    PUTBACK;
  }

  if(!(flags & G_NOARGS))
  {
    FREETMPS;
    LEAVE;
  }
}
