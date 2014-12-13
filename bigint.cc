#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <v8.h>
#include <node.h>
#include <nan.h>
#include <gmp.h>
#include <map>
#include <utility>

using namespace v8;
using namespace node;
using namespace std;

#define REQ_STR_ARG(I, VAR)                                                     \
        if (args.Length()<= (I) || !args[I]->IsString())                        \
                NanThrowTypeError("Argument " #I " must be a string");  \
        Local<String> VAR = Local<String>::Cast(args[I]);

#define REQ_UTF8_ARG(I, VAR)                                                    \
        if (args.Length() <= (I) || !args[I]->IsString())                       \
                NanThrowTypeError("Argument " #I " must be a utf8 string"); \
        String::Utf8Value VAR(args[I]->ToString());

#define REQ_INT32_ARG(I, VAR)                                                   \
        if (args.Length() <= (I) || !args[I]->IsInt32())                        \
                NanThrowTypeError("Argument " #I " must be an int32");      \
        int32_t VAR = args[I]->ToInt32()->Value();

#define REQ_UINT32_ARG(I, VAR)                                                  \
        if (args.Length() <= (I) || !args[I]->IsUint32())                       \
                NanThrowTypeError("Argument " #I " must be a uint32");      \
        uint32_t VAR = args[I]->ToUint32()->Value();

#define REQ_INT64_ARG(I, VAR)                                                   \
        if (args.Length() <= (I) || !args[I]->IsNumber())                       \
                NanThrowTypeError("Argument " #I " must be an int64");      \
        int64_t VAR = args[I]->ToInteger()->Value();

#define REQ_UINT64_ARG(I, VAR)                                                  \
        if (args.Length() <= (I) || !args[I]->IsNumber())                       \
                NanThrowTypeError("Argument " #I " must be a uint64");      \
        uint64_t VAR = args[I]->ToInteger()->Value();

#define WRAP_RESULT(RES, VAR)							\
	Handle<Value> arg[1] = { NanNew<External>(*RES) };				\
	Local<Object> VAR = NanNew<Function>(constructor)->NewInstance(1, arg);

class BigInt : public node::ObjectWrap {
	public:
		static void Initialize(Handle<Object> target);
		mpz_t *bigint_;
		static Persistent<Function> js_conditioner;

	protected:
		static Persistent<Function> constructor;

		BigInt(const String::Utf8Value& str, uint64_t base);
		BigInt(uint64_t num);
		BigInt(int64_t num);
		BigInt(mpz_t *num);
		BigInt();
		~BigInt();

		static NAN_METHOD(New);
		static NAN_METHOD(ToString);
		static NAN_METHOD(Badd);
		static NAN_METHOD(Bsub);
		static NAN_METHOD(Bmul);
		static NAN_METHOD(Bdiv);
		static NAN_METHOD(Uadd);
		static NAN_METHOD(Usub);
		static NAN_METHOD(Umul);
		static NAN_METHOD(Udiv);
		static NAN_METHOD(Umul_2exp);
		static NAN_METHOD(Udiv_2exp);
		static NAN_METHOD(Babs);
		static NAN_METHOD(Bneg);
		static NAN_METHOD(Bmod);
		static NAN_METHOD(Umod);
		static NAN_METHOD(Bpowm);
		static NAN_METHOD(Upowm);
		static NAN_METHOD(Upow);
		static NAN_METHOD(Uupow);
		static NAN_METHOD(Brand0);
		static NAN_METHOD(Probprime);
		static NAN_METHOD(Nextprime);
		static NAN_METHOD(Bcompare);
		static NAN_METHOD(Scompare);
		static NAN_METHOD(Ucompare);
		static NAN_METHOD(Band);
		static NAN_METHOD(Bor);
		static NAN_METHOD(Bxor);
		static NAN_METHOD(Binvertm);
		static NAN_METHOD(Bsqrt);
		static NAN_METHOD(Broot);
		static NAN_METHOD(BitLength);
		static NAN_METHOD(Bgcd);
};

static gmp_randstate_t *		randstate	= NULL;

Persistent<Function> BigInt::constructor;

Persistent<Function> BigInt::js_conditioner;

void BigInt::Initialize(v8::Handle<v8::Object> target) {
	NanScope();
	
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);

	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->SetClassName(NanNew("BigInt"));

	NODE_SET_PROTOTYPE_METHOD(tpl, "toString", ToString);
	NODE_SET_PROTOTYPE_METHOD(tpl, "badd", Badd);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bsub", Bsub);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bmul", Bmul);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bdiv", Bdiv);
	NODE_SET_PROTOTYPE_METHOD(tpl, "uadd", Uadd);
	NODE_SET_PROTOTYPE_METHOD(tpl, "usub", Usub);
	NODE_SET_PROTOTYPE_METHOD(tpl, "umul", Umul);
	NODE_SET_PROTOTYPE_METHOD(tpl, "udiv", Udiv);
	NODE_SET_PROTOTYPE_METHOD(tpl, "umul2exp", Umul_2exp);
	NODE_SET_PROTOTYPE_METHOD(tpl, "udiv2exp", Udiv_2exp);
	NODE_SET_PROTOTYPE_METHOD(tpl, "babs", Babs);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bneg", Bneg);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bmod", Bmod);
	NODE_SET_PROTOTYPE_METHOD(tpl, "umod", Umod);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bpowm", Bpowm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "upowm", Upowm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "upow", Upow);
	NODE_SET_PROTOTYPE_METHOD(tpl, "uupow", Uupow);
	NODE_SET_PROTOTYPE_METHOD(tpl, "brand0", Brand0);
	NODE_SET_PROTOTYPE_METHOD(tpl, "probprime", Probprime);
	NODE_SET_PROTOTYPE_METHOD(tpl, "nextprime", Nextprime);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bcompare", Bcompare);
	NODE_SET_PROTOTYPE_METHOD(tpl, "scompare", Scompare);
	NODE_SET_PROTOTYPE_METHOD(tpl, "ucompare", Ucompare);
	NODE_SET_PROTOTYPE_METHOD(tpl, "band", Band);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bor", Bor);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bxor", Bxor);
	NODE_SET_PROTOTYPE_METHOD(tpl, "binvertm", Binvertm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bsqrt", Bsqrt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "broot", Broot);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bitLength", BitLength);
	NODE_SET_PROTOTYPE_METHOD(tpl, "bgcd", Bgcd);

	NanAssignPersistent(constructor, tpl->GetFunction());
	target->Set(NanNew("BigInt"), tpl->GetFunction());
}

BigInt::BigInt (const v8::String::Utf8Value& str, uint64_t base) : ObjectWrap ()
{
	bigint_ = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*bigint_);

	mpz_set_str(*bigint_, *str, base);
}

BigInt::BigInt (uint64_t num) : ObjectWrap ()
{
	bigint_ = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*bigint_);

	mpz_set_ui(*bigint_, num);
}

BigInt::BigInt (int64_t num) : ObjectWrap ()
{
	bigint_ = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*bigint_);

	mpz_set_si(*bigint_, num);
}

BigInt::BigInt (mpz_t *num) : ObjectWrap ()
{
	bigint_ = num;
}

BigInt::BigInt () : ObjectWrap ()
{
	bigint_ = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*bigint_);

	mpz_set_ui(*bigint_, 0);
}

BigInt::~BigInt ()
{
	mpz_clear(*bigint_);
	free(bigint_);
}

NAN_METHOD(BigInt::New)
{
	NanScope();
	if(!args.IsConstructCall()) {
		int len = args.Length();
		Handle<Value>* newArgs = new Handle<Value>[len];
		for(int i = 0; i < len; i++) {
			newArgs[i] = args[i];
		}
		Handle<Value> newInst = NanNew<Function>(constructor)->NewInstance(len, newArgs);
		delete[] newArgs;
		NanReturnValue(newInst);
	}
	BigInt *bigint;
	uint64_t base;

	if(args[0]->IsExternal()) {
                mpz_t *num = static_cast<mpz_t *>(Local<External>::Cast(args[0])->Value());
		bigint = new BigInt(num);
	} else {
		int len = args.Length();
		Local<Object> ctx = NanNew<Object>();
		Handle<Value>* newArgs = new Handle<Value>[len];
		for(int i = 0; i < len; i++) {
			newArgs[i] = args[i];
		}
		//Local<Value> obj = NanMakeCallback(ctx, NanNew<Function>(js_conditioner), args.Length(), newArgs);
		Local<Value> obj = NanNew<Function>(js_conditioner)->Call(ctx, args.Length(), newArgs);

		if(!*obj) {
			return NanThrowError("Invalid type passed to bigint constructor");
		}

		String::Utf8Value str(obj->ToObject()->Get(NanNew("num"))->ToString());
		base = obj->ToObject()->Get(NanNew("base"))->ToNumber()->Value();

		bigint = new BigInt(str, base);
		delete[] newArgs;
	}

	bigint->Wrap(args.This());

	NanReturnValue(args.This());
}

NAN_METHOD(BigInt::ToString)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	uint64_t base = 10;

	if(args.Length() > 0) {
		REQ_UINT64_ARG(0, tbase);
      if(tbase < 2 || tbase > 62) {
         return NanThrowError("Base should be between 2 and 62, inclusive");
      }
		base = tbase;
	}
	char *to = mpz_get_str(0, base, *bigint->bigint_);

	Handle<Value> result = NanNew(to);
	free(to);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Badd)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);

	mpz_add(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bsub)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_sub(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);
	
	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bmul)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mul(*res, *bigint->bigint_, *bi->bigint_);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bdiv)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_div(*res, *bigint->bigint_, *bi->bigint_);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Uadd)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_add_ui(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Usub)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_sub_ui(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Umul)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mul_ui(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Udiv)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_div_ui(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Umul_2exp)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mul_2exp(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Udiv_2exp)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_div_2exp(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Babs)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_abs(*res, *bigint->bigint_);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bneg)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_neg(*res, *bigint->bigint_);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bmod)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mod(*res, *bigint->bigint_, *bi->bigint_);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Umod)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mod_ui(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bpowm)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi1 = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	BigInt *bi2 = ObjectWrap::Unwrap<BigInt>(args[1]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_powm(*res, *bigint->bigint_, *bi1->bigint_, *bi2->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Upowm)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[1]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_powm_ui(*res, *bigint->bigint_, x, *bi->bigint_);
	
	WRAP_RESULT(res, result);
	
	NanReturnValue(result);	
}

NAN_METHOD(BigInt::Upow)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_pow_ui(*res, *bigint->bigint_, x);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

/* 
 * This makes no sense?  It doesn't act on the object but is a
 * prototype method.
 */
NAN_METHOD(BigInt::Uupow)
{
	NanScope();

	REQ_UINT64_ARG(0, x);
	REQ_UINT64_ARG(1, y);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_ui_pow_ui(*res, x, y);
	
	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Brand0)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	
	if(randstate == NULL) {
		randstate = (gmp_randstate_t *) malloc(sizeof(gmp_randstate_t));
		gmp_randinit_default(*randstate);
		unsigned long seed = rand() + (time(NULL) * 1000) + clock();
        	gmp_randseed_ui(*randstate, seed);
	}
	
	mpz_urandomm(*res, *randstate, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Probprime)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();
	
	REQ_UINT32_ARG(0, reps);

	NanReturnValue(NanNew<Number>(mpz_probab_prime_p(*bigint->bigint_, reps)));
}

NAN_METHOD(BigInt::Nextprime)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_nextprime(*res, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bcompare)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();
	
	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());

	NanReturnValue(NanNew<Number>(mpz_cmp(*bigint->bigint_, *bi->bigint_)));
}

NAN_METHOD(BigInt::Scompare)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();
	
	REQ_INT64_ARG(0, x);
	
	NanReturnValue(NanNew<Number>(mpz_cmp_si(*bigint->bigint_, x)));
}

NAN_METHOD(BigInt::Ucompare)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();
	
	REQ_UINT64_ARG(0, x);

	NanReturnValue(NanNew<Number>(mpz_cmp_ui(*bigint->bigint_, x)));
}

NAN_METHOD(BigInt::Band)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_and(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bor)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_ior(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bxor)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_xor(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Binvertm)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_invert(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bsqrt)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_sqrt(*res, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Broot)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_root(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::BitLength)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

  size_t size = mpz_sizeinbase(*bigint->bigint_, 2);

	Handle<Value> result = NanNew<Integer>(size);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bgcd)
{
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());
	NanScope();

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_gcd(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

static NAN_METHOD(SetJSConditioner)
{
	NanScope();

	Local<Function> cb = args[0].As<Function>();
        NanAssignPersistent(BigInt::js_conditioner, cb);

	NanReturnUndefined();
}

extern "C" void
init (Handle<Object> target)
{
	NanScope();

	BigInt::Initialize(target);
	target->Set(NanNew("setJSConditioner"), NanNew<FunctionTemplate>(SetJSConditioner)->GetFunction());
}

NODE_MODULE(bigint, init);
