/**
 * @file PyBytesProxyHandler.cc
 * @author Philippe Laporte (philippe@distributive.network)
 * @brief Struct for creating JS Uint8Array-like proxy objects for immutable bytes objects
 * @date 2024-07-23
 *
 * @copyright Copyright (c) 2024 Distributive Corp.
 *
 */


#include "include/PyBytesProxyHandler.hh"

#include <jsapi.h>
#include <js/ArrayBuffer.h>

#include <Python.h>


const char PyBytesProxyHandler::family = 0;


static bool array_valueOf(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  JS::RootedObject proxy(cx, JS::ToObject(cx, args.thisv()));
  if (!proxy) {
    return false;
  }

  JS::PersistentRootedObject* arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(proxy, OtherSlot);
  JS::RootedObject rootedArrayBuffer(cx, arrayBuffer->get());

  auto byteLength = JS::GetArrayBufferByteLength(rootedArrayBuffer);

  bool isSharedMemory; 
  JS::AutoCheckCannotGC autoNoGC(cx);
  uint8_t *data = JS::GetArrayBufferData(rootedArrayBuffer, &isSharedMemory, autoNoGC);

  std::string valueOfString;

  for (Py_ssize_t index = 0; index < byteLength; index++) {
    if (index > 0) {
      valueOfString += ",";
    }
    
    valueOfString += std::to_string(data[index]);
  }

  args.rval().setString(JS_NewStringCopyZ(cx, valueOfString.c_str()));
  return true;
}

static bool array_toString(JSContext *cx, unsigned argc, JS::Value *vp) {
  return array_valueOf(cx, argc, vp);
}


// BytesIterator


#define ITEM_KIND_KEY 0
#define ITEM_KIND_VALUE 1
#define ITEM_KIND_KEY_AND_VALUE 2

enum {
  BytesIteratorSlotIteratedObject,
  BytesIteratorSlotNextIndex,
  BytesIteratorSlotItemKind,
  BytesIteratorSlotCount
};

static JSClass bytesIteratorClass = {"BytesIterator", JSCLASS_HAS_RESERVED_SLOTS(BytesIteratorSlotCount)};

static bool iterator_next(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  JS::RootedObject thisObj(cx);
  if (!args.computeThis(cx, &thisObj)) return false;

  JS::PersistentRootedObject* arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(thisObj, BytesIteratorSlotIteratedObject);
  JS::RootedObject rootedArrayBuffer(cx, arrayBuffer->get());

  JS::RootedValue rootedNextIndex(cx, JS::GetReservedSlot(thisObj, BytesIteratorSlotNextIndex));
  JS::RootedValue rootedItemKind(cx, JS::GetReservedSlot(thisObj, BytesIteratorSlotItemKind));

  int32_t nextIndex;
  int32_t itemKind;
  if (!JS::ToInt32(cx, rootedNextIndex, &nextIndex) || !JS::ToInt32(cx, rootedItemKind, &itemKind)) return false;

  JS::RootedObject result(cx, JS_NewPlainObject(cx));

  Py_ssize_t len = JS::GetArrayBufferByteLength(rootedArrayBuffer);

  if (nextIndex >= len) {
    // UnsafeSetReservedSlot(obj, ITERATOR_SLOT_TARGET, null); // TODO lose ref
    JS::RootedValue done(cx, JS::BooleanValue(true));
    if (!JS_SetProperty(cx, result, "done", done)) return false;
    args.rval().setObject(*result);
    return result;
  }

  JS::SetReservedSlot(thisObj, BytesIteratorSlotNextIndex, JS::Int32Value(nextIndex + 1));

  JS::RootedValue done(cx, JS::BooleanValue(false));
  if (!JS_SetProperty(cx, result, "done", done)) return false;

  if (itemKind == ITEM_KIND_VALUE) {
    bool isSharedMemory; 
    JS::AutoCheckCannotGC autoNoGC(cx);
    uint8_t *data = JS::GetArrayBufferData(rootedArrayBuffer, &isSharedMemory, autoNoGC);

    JS::RootedValue value(cx, JS::Int32Value(data[nextIndex]));
    if (!JS_SetProperty(cx, result, "value", value)) return false;
  }
  else if (itemKind == ITEM_KIND_KEY_AND_VALUE) {
    JS::Rooted<JS::ValueArray<2>> items(cx);

    JS::RootedValue rootedNextIndex(cx, JS::Int32Value(nextIndex));
    items[0].set(rootedNextIndex);

    bool isSharedMemory; 
    JS::AutoCheckCannotGC autoNoGC(cx);
    uint8_t *data = JS::GetArrayBufferData(rootedArrayBuffer, &isSharedMemory, autoNoGC);

    JS::RootedValue value(cx, JS::Int32Value(data[nextIndex]));
    items[1].set(value);

    JS::RootedValue pair(cx);
    JSObject *array = JS::NewArrayObject(cx, items);
    pair.setObject(*array);
    if (!JS_SetProperty(cx, result, "value", pair)) return false;
  }
  else { // itemKind == ITEM_KIND_KEY
    JS::RootedValue value(cx, JS::Int32Value(nextIndex));
    if (!JS_SetProperty(cx, result, "value", value)) return false;
  }

  args.rval().setObject(*result);
  return true;
}

static JSFunctionSpec bytes_iterator_methods[] = {
  JS_FN("next", iterator_next, 0, JSPROP_ENUMERATE),
  JS_FS_END
};

static bool BytesIteratorConstructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  if (!args.isConstructing()) {
    JS_ReportErrorASCII(cx, "You must call this constructor with 'new'");
    return false;
  }

  JS::RootedObject thisObj(cx, JS_NewObjectForConstructor(cx, &bytesIteratorClass, args));
  if (!thisObj) {
    return false;
  }

  args.rval().setObject(*thisObj);
  return true;
}

static bool DefineBytesIterator(JSContext *cx, JS::HandleObject global) {
  JS::RootedObject iteratorPrototype(cx);
  if (!JS_GetClassPrototype(cx, JSProto_Iterator, &iteratorPrototype)) {
    return false;
  }

  JS::RootedObject protoObj(cx,
    JS_InitClass(cx, global,
      nullptr, iteratorPrototype,
      "BytesIterator",
      BytesIteratorConstructor, 0,
      nullptr, bytes_iterator_methods,
      nullptr, nullptr)
  );

  return protoObj; // != nullptr
}

/// private util
static bool array_iterator_func(JSContext *cx, unsigned argc, JS::Value *vp, int itemKind) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  JS::RootedObject proxy(cx, JS::ToObject(cx, args.thisv()));
  if (!proxy) {
    return false;
  }

  JS::RootedObject global(cx, JS::GetNonCCWObjectGlobal(proxy));

  JS::RootedValue constructor_val(cx);
  if (!JS_GetProperty(cx, global, "BytesIterator", &constructor_val)) return false;
  if (!constructor_val.isObject()) {
    if (!DefineBytesIterator(cx, global)) {
      return false;
    }

    if (!JS_GetProperty(cx, global, "BytesIterator", &constructor_val)) return false;
    if (!constructor_val.isObject()) {
      JS_ReportErrorASCII(cx, "BytesIterator is not a constructor");
      return false;
    }
  }
  JS::RootedObject constructor(cx, &constructor_val.toObject());

  JS::RootedObject obj(cx);
  if (!JS::Construct(cx, constructor_val, JS::HandleValueArray::empty(), &obj)) return false;
  if (!obj) return false;

  JS::PersistentRootedObject* arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(proxy, OtherSlot);
 
  JS::SetReservedSlot(obj, BytesIteratorSlotIteratedObject, JS::PrivateValue(arrayBuffer));
  JS::SetReservedSlot(obj, BytesIteratorSlotNextIndex, JS::Int32Value(0));
  JS::SetReservedSlot(obj, BytesIteratorSlotItemKind, JS::Int32Value(itemKind));

  args.rval().setObject(*obj);
  return true;
}

static bool array_entries(JSContext *cx, unsigned argc, JS::Value *vp) {
  return array_iterator_func(cx, argc, vp, ITEM_KIND_KEY_AND_VALUE);
}

static bool array_keys(JSContext *cx, unsigned argc, JS::Value *vp) {
  return array_iterator_func(cx, argc, vp, ITEM_KIND_KEY);
}

static bool array_values(JSContext *cx, unsigned argc, JS::Value *vp) {
  return array_iterator_func(cx, argc, vp, ITEM_KIND_VALUE);
}


JSMethodDef PyBytesProxyHandler::array_methods[] = {
  {"toString", array_toString, 0},
  {"valueOf", array_valueOf, 0},
  {"entries", array_entries, 0},
  {"keys", array_keys, 0},
  {"values", array_values, 0},
  {NULL, NULL, 0}
};


bool PyBytesProxyHandler::set(JSContext *cx, JS::HandleObject proxy, JS::HandleId id,
  JS::HandleValue v, JS::HandleValue receiver,
  JS::ObjectOpResult &result) const {

  // block all modifications  
  
  PyObject *self = JS::GetMaybePtrFromReservedSlot<PyObject>(proxy, PyObjectSlot);

  PyErr_Format(PyExc_TypeError,
               "'%.100s' object has only read-only attributes",
               Py_TYPE(self)->tp_name);

  return result.failReadOnly();
}

bool PyBytesProxyHandler::getOwnPropertyDescriptor(
  JSContext *cx, JS::HandleObject proxy, JS::HandleId id,
  JS::MutableHandle<mozilla::Maybe<JS::PropertyDescriptor>> desc
) const {
  // see if we're calling a function
  if (id.isString()) {
    for (size_t index = 0;; index++) {
      bool isThatFunction;
      const char *methodName = array_methods[index].name;
      if (methodName == NULL) {
        break;
      }
      else if (JS_StringEqualsAscii(cx, id.toString(), methodName, &isThatFunction) && isThatFunction) {
        JSFunction *newFunction = JS_NewFunction(cx, array_methods[index].call, array_methods[index].nargs, 0, NULL);
        if (!newFunction) return false;
        JS::RootedObject funObj(cx, JS_GetFunctionObject(newFunction));
        desc.set(mozilla::Some(
          JS::PropertyDescriptor::Data(
            JS::ObjectValue(*funObj),
            {JS::PropertyAttribute::Enumerable}
          )
        ));
        return true;
      }
    }
  }

  if (id.isString()) {
    bool isProperty;

    // "length" and byteLength properties have the same value
    if (((JS_StringEqualsLiteral(cx, id.toString(), "length", &isProperty) && isProperty) || (JS_StringEqualsLiteral(cx, id.toString(), "byteLength", &isProperty) && isProperty))) {
      JS::PersistentRootedObject* arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(proxy, OtherSlot);

      JS::RootedObject rootedArrayBuffer(cx, arrayBuffer->get());

      auto byteLength = JS::GetArrayBufferByteLength(rootedArrayBuffer);

      desc.set(mozilla::Some(
        JS::PropertyDescriptor::Data(
          JS::Int32Value(byteLength)
        )
      ));
      return true;
    }

    // buffer property
    if (JS_StringEqualsLiteral(cx, id.toString(), "buffer", &isProperty) && isProperty) {
      JS::PersistentRootedObject* arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(proxy, OtherSlot);

      desc.set(mozilla::Some(
        JS::PropertyDescriptor::Data(
          JS::ObjectValue(*(arrayBuffer->get()))
        )
      ));
      return true;
    }

    // BYTES_PER_ELEMENT property
    if (JS_StringEqualsLiteral(cx, id.toString(), "BYTES_PER_ELEMENT", &isProperty) && isProperty) {
      desc.set(mozilla::Some(
        JS::PropertyDescriptor::Data(
          JS::Int32Value(1)
        )
      ));
      return true;
    }

    // byteOffset property
    if (JS_StringEqualsLiteral(cx, id.toString(), "byteOffset", &isProperty) && isProperty) {
      desc.set(mozilla::Some(
        JS::PropertyDescriptor::Data(
          JS::Int32Value(0)
        )
      ));
      return true;
    }

    // "constructor" property
    if (JS_StringEqualsLiteral(cx, id.toString(), "constructor", &isProperty) && isProperty) {
      JS::RootedObject global(cx, JS::GetNonCCWObjectGlobal(proxy));

      JS::RootedObject uint8ArrayPrototype(cx);
      if (!JS_GetClassPrototype(cx, JSProto_Uint8Array, &uint8ArrayPrototype)) {
        return false;
      }

      JS::RootedValue Uint8Array_Prototype_Constructor(cx);
      if (!JS_GetProperty(cx, uint8ArrayPrototype, "constructor", &Uint8Array_Prototype_Constructor)) {
        return false;
      }

      JS::RootedObject rootedUint8ArrayPrototypeConstructor(cx, Uint8Array_Prototype_Constructor.toObjectOrNull());

      desc.set(mozilla::Some(
        JS::PropertyDescriptor::Data(
          JS::ObjectValue(*rootedUint8ArrayPrototypeConstructor),
          {JS::PropertyAttribute::Enumerable}
        )
      ));

      return true;
    }
  }

  if (id.isSymbol()) {
    JS::RootedSymbol rootedSymbol(cx, id.toSymbol());

    if (JS::GetSymbolCode(rootedSymbol) == JS::SymbolCode::iterator) {
      JSFunction *newFunction = JS_NewFunction(cx, array_values, 0, 0, NULL);
      if (!newFunction) return false;
      JS::RootedObject funObj(cx, JS_GetFunctionObject(newFunction));
      desc.set(mozilla::Some(
        JS::PropertyDescriptor::Data(
          JS::ObjectValue(*funObj),
          {JS::PropertyAttribute::Enumerable}
        )
      ));
      return true;
    }

    return true; // needed for console.log
  }

  // item
  Py_ssize_t index;
  if (idToIndex(cx, id, &index)) {
    JS::PersistentRootedObject* arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(proxy, OtherSlot);
    JS::RootedObject rootedArrayBuffer(cx, arrayBuffer->get());

    bool isSharedMemory; 
    JS::AutoCheckCannotGC autoNoGC(cx);
    uint8_t *data = JS::GetArrayBufferData(rootedArrayBuffer, &isSharedMemory, autoNoGC);

    desc.set(mozilla::Some(
      JS::PropertyDescriptor::Data(
        JS::Int32Value(data[index])
      )
    ));

    return true;
  } 

  PyObject *attrName = idToKey(cx, id);
  PyObject *self = JS::GetMaybePtrFromReservedSlot<PyObject>(proxy, PyObjectSlot);
  PyObject *item = PyObject_GetAttr(self, attrName);

  return handleGetOwnPropertyDescriptor(cx, id, desc, item);
}

void PyBytesProxyHandler::finalize(JS::GCContext *gcx, JSObject *proxy) const {
  PyObjectProxyHandler::finalize(gcx, proxy);

  JS::PersistentRootedObject *arrayBuffer = JS::GetMaybePtrFromReservedSlot<JS::PersistentRootedObject>(proxy, OtherSlot);
  delete arrayBuffer;
}