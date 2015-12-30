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
	Handle<Value> arg[1] = { Nan::New<External>(*RES) };				\
	Local<Object> VAR = Nan::New<Function>(constructor)->NewInstance(1, arg);

class BigInt : public node::ObjectWrap {
	public:
		static void Initialize(Handle<Object> target);
		mpz_t *bigint_;
		static Persistent<Function> js_conditioner;

	protected:
    static Nan::Persistent<v8::Function> constructor;

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

Nan::Persistent<v8::Function> BigInt::constructor;

Persistent<Function> BigInt::js_conditioner;

void BigInt::Initialize(v8::Handle<v8::Object> target) {
  Nan::HandleScope scope;

	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->SetClassName(Nan::New("BigInt").ToLocalChecked());

  Nan::SetPrototypeMethod(tpl, "toString", ToString);
	Nan::SetPrototypeMethod(tpl, "badd", Badd);
	Nan::SetPrototypeMethod(tpl, "bsub", Bsub);
	Nan::SetPrototypeMethod(tpl, "bmul", Bmul);
	Nan::SetPrototypeMethod(tpl, "bdiv", Bdiv);
	Nan::SetPrototypeMethod(tpl, "uadd", Uadd);
	Nan::SetPrototypeMethod(tpl, "usub", Usub);
	Nan::SetPrototypeMethod(tpl, "umul", Umul);
	Nan::SetPrototypeMethod(tpl, "udiv", Udiv);
	Nan::SetPrototypeMethod(tpl, "umul2exp", Umul_2exp);
	Nan::SetPrototypeMethod(tpl, "udiv2exp", Udiv_2exp);
	Nan::SetPrototypeMethod(tpl, "babs", Babs);
	Nan::SetPrototypeMethod(tpl, "bneg", Bneg);
	Nan::SetPrototypeMethod(tpl, "bmod", Bmod);
	Nan::SetPrototypeMethod(tpl, "umod", Umod);
	Nan::SetPrototypeMethod(tpl, "bpowm", Bpowm);
	Nan::SetPrototypeMethod(tpl, "upowm", Upowm);
	Nan::SetPrototypeMethod(tpl, "upow", Upow);
	Nan::SetPrototypeMethod(tpl, "uupow", Uupow);
	Nan::SetPrototypeMethod(tpl, "brand0", Brand0);
	Nan::SetPrototypeMethod(tpl, "probprime", Probprime);
	Nan::SetPrototypeMethod(tpl, "nextprime", Nextprime);
	Nan::SetPrototypeMethod(tpl, "bcompare", Bcompare);
	Nan::SetPrototypeMethod(tpl, "scompare", Scompare);
	Nan::SetPrototypeMethod(tpl, "ucompare", Ucompare);
	Nan::SetPrototypeMethod(tpl, "band", Band);
	Nan::SetPrototypeMethod(tpl, "bor", Bor);
	Nan::SetPrototypeMethod(tpl, "bxor", Bxor);
	Nan::SetPrototypeMethod(tpl, "binvertm", Binvertm);
	Nan::SetPrototypeMethod(tpl, "bsqrt", Bsqrt);
	Nan::SetPrototypeMethod(tpl, "broot", Broot);
	Nan::SetPrototypeMethod(tpl, "bitLength", BitLength);
	Nan::SetPrototypeMethod(tpl, "bgcd", Bgcd);

	constructor.Reset(tpl->GetFunction());
	target->Set(Nan::New("BigInt").ToLocalChecked(), tpl->GetFunction());
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
  Nan::HandleScope scope;
	if(!args.IsConstructCall()) {
		int len = args.Length();
		Handle<Value>* newArgs = new Handle<Value>[len];
		for(int i = 0; i < len; i++) {
			newArgs[i] = args[i];
		}
		Handle<Value> newInst = Nan::New<Function>(constructor)->NewInstance(len, newArgs);
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
		Local<Object> ctx = Nan::New<Object>();
		Handle<Value>* newArgs = new Handle<Value>[len];
		for(int i = 0; i < len; i++) {
			newArgs[i] = args[i];
		}
		//Local<Value> obj = NanMakeCallback(ctx, Nan::New<Function>(js_conditioner), args.Length(), newArgs);
		Local<Value> obj = Nan::New<Function>(js_conditioner)->Call(ctx, args.Length(), newArgs);

		if(!*obj) {
			return NanThrowError("Invalid type passed to bigint constructor");
		}

		String::Utf8Value str(obj->ToObject()->Get(Nan::New("num").ToLocalChecked())->ToString());
		base = obj->ToObject()->Get(Nan::New("base").ToLocalChecked())->ToNumber()->Value();

		bigint = new BigInt(str, base);
		delete[] newArgs;
	}

	bigint->Wrap(args.This());

	NanReturnValue(args.This());
}

NAN_METHOD(BigInt::ToString)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	uint64_t base = 10;

	if(args.Length() > 0) {
		REQ_UINT64_ARG(0, tbase);
      if(tbase < 2 || tbase > 62) {
         return NanThrowError("Base should be between 2 and 62, inclusive");
      }
		base = tbase;
	}
	char *to = mpz_get_str(0, base, *bigint->bigint_);

	Handle<Value> result = Nan::New(to).ToLocalChecked();
	free(to);

	NanReturnValue(result);
}

calChecked()
NAN_METHOD(BigInt::Badd)
{
  calChecked()
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);

	mpz_add(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bsub)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_sub(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bmul)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mul(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bdiv)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_div(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Uadd)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_add_ui(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Usub)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_sub_ui(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Umul)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mul_ui(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Udiv)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_div_ui(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Umul_2exp)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mul_2exp(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Udiv_2exp)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_div_2exp(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Babs)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_abs(*res, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bneg)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_neg(*res, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bmod)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mod(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Umod)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_mod_ui(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bpowm)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

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
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

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
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

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
  Nan::HandleScope scope;

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
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

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
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT32_ARG(0, reps);

	NanReturnValue(Nan::New<Number>(mpz_probab_prime_p(*bigint->bigint_, reps)));
}

NAN_METHOD(BigInt::Nextprime)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_nextprime(*res, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bcompare)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());

	NanReturnValue(Nan::New<Number>(mpz_cmp(*bigint->bigint_, *bi->bigint_)));
}

NAN_METHOD(BigInt::Scompare)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_INT64_ARG(0, x);

	NanReturnValue(Nan::New<Number>(mpz_cmp_si(*bigint->bigint_, x)));
}

NAN_METHOD(BigInt::Ucompare)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);

	NanReturnValue(Nan::New<Number>(mpz_cmp_ui(*bigint->bigint_, x)));
}

NAN_METHOD(BigInt::Band)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_and(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bor)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_ior(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bxor)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_xor(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Binvertm)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_invert(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bsqrt)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_sqrt(*res, *bigint->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Broot)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	REQ_UINT64_ARG(0, x);
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_root(*res, *bigint->bigint_, x);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

NAN_METHOD(BigInt::BitLength)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

  size_t size = mpz_sizeinbase(*bigint->bigint_, 2);

	Handle<Value> result = Nan::New<Integer>(static_cast<uint32_t>(size));

	NanReturnValue(result);
}

NAN_METHOD(BigInt::Bgcd)
{
  Nan::HandleScope scope;
	BigInt *bigint = ObjectWrap::Unwrap<BigInt>(args.This());

	BigInt *bi = ObjectWrap::Unwrap<BigInt>(args[0]->ToObject());
	mpz_t *res = (mpz_t *) malloc(sizeof(mpz_t));
	mpz_init(*res);
	mpz_gcd(*res, *bigint->bigint_, *bi->bigint_);

	WRAP_RESULT(res, result);

	NanReturnValue(result);
}

static NAN_METHOD(SetJSConditioner)
{
  Nan::HandleScope scope;

	Local<Function> cb = args[0].As<Function>();
        NanAssignPersistent(BigInt::js_conditioner, cb);

	NanReturnUndefined();
}

extern "C" void
init (Handle<Object> target)
{
  Nan::HandleScope scope;

	BigInt::Initialize(target);
	target->Set(Nan::New("setJSConditioner").ToLocalChecked(), Nan::New<FunctionTemplate>(SetJSConditioner)->GetFunction());
}

NODE_MODULE(bigint, init);
