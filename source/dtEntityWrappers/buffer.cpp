/*
* dtEntity Game and Simulation Engine
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Martin Scheffler
*/

#include <v8.h>
#include <string>
#include <vector>
#include <dtEntityWrappers/v8helpers.h>
#include <dtEntityWrappers/bytestorage.h>

/*
  This file is basically copied from the v8cgi project: http://code.google.com/p/v8cgi/
  */
#define BS_OTHER(object) reinterpret_cast<ByteStorage *>(object->GetPointerFromInternalField(0))
#define BS_THIS BS_OTHER(args.This())
#define WRONG_CTOR JS_TYPE_ERROR("Buffer() called with wrong arguments.")

#if _MSC_VER
#define snprintf _snprintf
#endif

namespace dtEntityWrappers{

	v8::Persistent<v8::FunctionTemplate> bufferTemplate;
	v8::Persistent<v8::Function> buffer;

	////////////////////////////////////////////////////////////////////////////////
	size_t firstIndex(v8::Handle<v8::Value> index, size_t length) {
		int i = 0;
		if (!index->IsUndefined()) { i = index->IntegerValue(); }
		if (i < 0) { i += length; }

		if (i < 0) { i = 0; }
		if ((size_t) i > length) { i = length; }
		return (size_t) i;
	}

	////////////////////////////////////////////////////////////////////////////////
	size_t lastIndex(v8::Handle<v8::Value> index, size_t length) {
		int i = length;
		if (!index->IsUndefined()) { i = index->IntegerValue(); }
		if (i < 0) { i += length; }

		if (i < 0) { i = 0; }
		if ((size_t) i > length) { i = length; }
		return (size_t) i;
	}

	////////////////////////////////////////////////////////////////////////////////
   void Buffer_destroy(v8::Persistent<v8::Value> v, void* ptr) {
      using namespace v8;
		HandleScope scope;
		Handle<Object> o = Handle<Object>::Cast(v);
		ByteStorage * bs = BS_OTHER(o);
		delete bs;
		v.Dispose();
	}

	////////////////////////////////////////////////////////////////////////////////
	void Buffer_fromBuffer(const v8::Arguments& args, v8::Handle<v8::Object> obj) {
		ByteStorage * bs2 = BS_OTHER(obj);

		int index1 = firstIndex(args[1], bs2->getLength());
		int index2 = lastIndex(args[2], bs2->getLength());

		bool copy = true;
		if (!args[3]->IsUndefined()) { copy = args[3]->ToBoolean()->Value(); }

		ByteStorage * bs;
		if (copy) {
			size_t length = index2-index1;
			bs = new ByteStorage(length);
			bs->fill(bs2->getData() + index1, length);
		} else {
			bs = new ByteStorage(bs2, index1, index2);
		}

		SAVE_PTR(0, bs);
	}

	////////////////////////////////////////////////////////////////////////////////
	void Buffer_fromString(const v8::Arguments& args) {
		if (args.Length() < 2) { WRONG_CTOR; }
		v8::String::Utf8Value str(args[0]);
		v8::String::Utf8Value charset(args[1]);

		ByteStorage bs_tmp((char *) (*str), str.length());
		ByteStorage * bs = bs_tmp.transcode("utf-8", *charset);
		SAVE_PTR(0, bs);
	}

	////////////////////////////////////////////////////////////////////////////////
	void Buffer_fromArray(const v8::Arguments& args) {
		v8::Handle<v8::Array> arr = v8::Handle<v8::Array>::Cast(args[0]);

		size_t index1 = firstIndex(args[1], arr->Length());
		size_t index2 = lastIndex(args[2], arr->Length());
		ByteStorage * bs = new ByteStorage(index2 - index1);

		size_t index = 0;
		for (size_t i=index1; i<index2; i++) {
			char value = arr->Get(JS_INT(i))->IntegerValue();
			bs->setByte(index++, value);
		}
		SAVE_PTR(0, bs);
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(_Buffer) {
      using namespace v8;
		if (!args.IsConstructCall()) { RETURN_CONSTRUCT_CALL; }
		if (args.Length() == 0) { WRONG_CTOR; }

		try {
			if (args[0]->IsExternal()) {
				Handle<External> ext = Handle<External>::Cast(args[0]);
				SAVE_PTR(0, ext->Value());
			} else if (args[0]->IsNumber()) {
				char fill = (args.Length() > 1 ? (char) args[1]->IntegerValue() : 0);
				ByteStorage * bs = new ByteStorage(args[0]->IntegerValue());
				bs->fill(fill);
				SAVE_PTR(0, bs);
			} else if (args[0]->IsArray()) {
				Buffer_fromArray(args);
			} else if (args[0]->IsObject()) {
				Handle<Object> obj = Handle<Object>::Cast(args[0]);
				if (INSTANCEOF(obj, bufferTemplate)) {
					Buffer_fromBuffer(args, obj);
				} else { WRONG_CTOR; }
			} else if (args[0]->IsString()) {
				Buffer_fromString(args);
			} else {
				WRONG_CTOR;
			}
		} catch (std::string e) {
			return JS_ERROR(e.c_str());
		}

		//GC * gc = GC_PTR;
		//gc->add(args.This(), Buffer_destroy);

		Persistent<Object> pobj = Persistent<Object>::New(args.This());
		pobj.MakeWeak(NULL, &Buffer_destroy);

		return args.This();
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_toString) {
		ByteStorage * bs = BS_THIS;

		if (args.Length() == 0) {
			size_t tmpSize = 100;
			std::string result = "[Buffer ";
			char * tmp = (char *) malloc(tmpSize);
			if (tmp) {
            #pragma warning (disable : 4996)
				size_t size = snprintf(tmp, tmpSize, "%lu", (unsigned long) bs->getLength());
				if (size < tmpSize) { result += tmp; }
				free(tmp);
			}
			result += "]";
			return JS_STR(result.c_str());
		}

		v8::String::Utf8Value charset(args[0]);
		size_t index1 = firstIndex(args[1], bs->getLength());
		size_t index2 = lastIndex(args[2], bs->getLength());
		ByteStorage view(bs, index1, index2);

		try {
			ByteStorage * bs2 = view.transcode(*charset, "utf-8");
			v8::Handle<v8::Value> result = JS_STR((const char *) bs2->getData(), bs2->getLength());
			delete bs2;
			return result;
		} catch (std::string e) {
			return JS_ERROR(e.c_str());
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_range) {
		ByteStorage * bs = BS_THIS;
		size_t index1 = firstIndex(args[0], bs->getLength());
		size_t index2 = lastIndex(args[1], bs->getLength());

		ByteStorage * bs2 = new ByteStorage(bs, index1, index2);
		v8::Handle<v8::Value> newargs[] = { v8::External::New((void*)bs2) };
		return buffer->NewInstance(1, newargs);
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_slice) {
		ByteStorage * bs = BS_THIS;
		size_t index1 = firstIndex(args[0], bs->getLength());
		size_t index2 = lastIndex(args[1], bs->getLength());

		size_t length = index2-index1;
		ByteStorage * bs2 = new ByteStorage(bs->getData() + index1, length);

		v8::Handle<v8::Value> newargs[] = { v8::External::New((void*)bs2) };
		return buffer->NewInstance(1, newargs);
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_fill) {
		if (args.Length() == 0) { return JS_TYPE_ERROR("Invalid value to fill"); }
		ByteStorage * bs = BS_THIS;
		size_t index1 = firstIndex(args[1], bs->getLength());
		size_t index2 = lastIndex(args[2], bs->getLength());
		char fill = (char) args[0]->IntegerValue();

		for (size_t i = index1; i<index2; i++) {
			bs->setByte(i, fill);
		}

		return args.This();
	}

	////////////////////////////////////////////////////////////////////////////////
	/**
	 * Generic copy implementation. Handler either Buffer->Array/Buffer, or Array/Buffer->Buffer copies.
	 * @param {args} args
	 * @param {bool} to We are copying *from* args.This()
	 */
	v8::Handle<v8::Value> Buffer_copy_impl(const v8::Arguments& args, bool source) {
		const char * errmsg = "First argument must be a Buffer or Array";
		ByteStorage * bs = BS_THIS;
		ByteStorage * bs2 = NULL;
		size_t length;
		v8::Handle<v8::Array> arr;

		if (args.Length() == 0) { return JS_TYPE_ERROR(errmsg); }
		if (args[0]->IsArray()) {
			arr = v8::Handle<v8::Array>::Cast(args[0]);
			length = arr->Length();
		} else if (args[0]->IsObject()) {
			v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(args[0]);
			if (!INSTANCEOF(obj, bufferTemplate)) { return JS_TYPE_ERROR(errmsg); }
			bs2 = BS_OTHER(obj);
			length = bs2->getLength();
		} else { return JS_TYPE_ERROR(errmsg); }

		size_t offsetSource, offsetTarget, amount;
		if (source) {
			offsetSource = firstIndex(args[2], bs->getLength());
			offsetTarget = firstIndex(args[1], length);
			amount = MIN(length - offsetTarget, lastIndex(args[3], bs->getLength()) - offsetSource);
		} else {
			offsetSource = firstIndex(args[1], length);
			offsetTarget = firstIndex(args[2], bs->getLength());
			amount = MIN(length - offsetSource, lastIndex(args[3], bs->getLength()) - offsetTarget);
		}

		char byte;

		if (source) {
			for (size_t i=0; i<amount; i++) {
				byte = bs->getByte(i + offsetSource);
				if (bs2) {
					bs2->setByte(i + offsetTarget, byte);
				} else {
					arr->Set(JS_INT(i + offsetTarget), JS_INT(byte));
				}
			}
		} else {
			for (size_t i=0; i<amount; i++) {
				if (bs2) {
					byte = bs2->getByte(i + offsetSource);
				} else {
					byte = arr->Get(JS_INT(i + offsetSource))->IntegerValue();
				}
				bs->setByte(i + offsetTarget, byte);
			}
		}

		return args.This();
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_copy) {
		return Buffer_copy_impl(args, true);
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_copyFrom) {
		return Buffer_copy_impl(args, false);
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_read) {
		return JS_ERROR("Buffer::read not yet implemented");
	}

	////////////////////////////////////////////////////////////////////////////////
	JS_METHOD(Buffer_write) {
		return JS_ERROR("Buffer::write not yet implemented");
	}

	////////////////////////////////////////////////////////////////////////////////
	v8::Handle<v8::Value> Buffer_length(v8::Local<v8::String> property, const v8::AccessorInfo &info) {
		ByteStorage * bs = BS_OTHER(info.This());
		return JS_INT(bs->getLength());
	}

	////////////////////////////////////////////////////////////////////////////////
	v8::Handle<v8::Value> Buffer_get(uint32_t index, const v8::AccessorInfo &info) {
		ByteStorage * bs = BS_OTHER(info.This());
		size_t len = bs->getLength();
		if (index < 0 || index >= len) { return JS_RANGE_ERROR("Non-existent index"); }

		return JS_INT((unsigned char) bs->getByte(index));
	}

	////////////////////////////////////////////////////////////////////////////////
	v8::Handle<v8::Value> Buffer_set(uint32_t index, v8::Local<v8::Value> value, const v8::AccessorInfo &info) {
		ByteStorage * bs = BS_OTHER(info.This());
		size_t len = bs->getLength();
		if (index < 0 || index >= len) { return JS_RANGE_ERROR("Non-existent index"); }

		bs->setByte(index, (unsigned char) value->IntegerValue());
		return value;
	}

	////////////////////////////////////////////////////////////////////////////////
   v8::Handle<v8::Function> CreateBuffer()
	{
      using namespace v8;
		HandleScope handle_scope;

		bufferTemplate = Persistent<FunctionTemplate>::New(FunctionTemplate::New(_Buffer));
		bufferTemplate->SetClassName(JS_STR("Buffer"));

		Handle<ObjectTemplate> bufferPrototype = bufferTemplate->PrototypeTemplate();
		bufferPrototype->Set(JS_STR("toString"), FunctionTemplate::New(Buffer_toString));
		bufferPrototype->Set(JS_STR("range"), FunctionTemplate::New(Buffer_range));
		bufferPrototype->Set(JS_STR("slice"), FunctionTemplate::New(Buffer_slice));
		bufferPrototype->Set(JS_STR("fill"), FunctionTemplate::New(Buffer_fill));
		bufferPrototype->Set(JS_STR("copy"), FunctionTemplate::New(Buffer_copy));
		bufferPrototype->Set(JS_STR("copyFrom"), FunctionTemplate::New(Buffer_copyFrom));
		bufferPrototype->Set(JS_STR("read"), FunctionTemplate::New(Buffer_read));
		bufferPrototype->Set(JS_STR("write"), FunctionTemplate::New(Buffer_write));

		Handle<v8::ObjectTemplate> bufferObject = bufferTemplate->InstanceTemplate();
		bufferObject->SetInternalFieldCount(1);
		bufferObject->SetAccessor(JS_STR("length"), Buffer_length, 0, Handle<Value>(), DEFAULT, static_cast<PropertyAttribute>(DontDelete));
		bufferObject->SetIndexedPropertyHandler(Buffer_get, Buffer_set);

		return handle_scope.Close(bufferTemplate->GetFunction());
	}

} /* namespace */


