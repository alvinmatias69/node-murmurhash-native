#include "nodemurmurhash.h"
#include "inputdata.h"

namespace MurmurHash {
  using v8::Local;
  using v8::Object;
  using v8::String;
  using node::Buffer;

  InputData::InputData() : buffer(NULL), size(0), ownBuffer(false) {}

  void InputData::Setup(
      const Handle<Value> &key, const Handle<Value> &encoding_v)
  {
    if ( key->IsString() ) {
      const enum node::encoding enc = DetermineEncoding(encoding_v);
      ssize_t length = NanDecodeBytes(key, enc);
      if ( length != -1 ) {
        size = (size_t) length;
        char *data = EnsureBuffer(size + 1);
        NanDecodeWrite( data, size, key, enc );
      }
    } else if ( Buffer::HasInstance(key) ) {
      InitFromBuffer(key);
    }
  }

  void InputData::Setup(const Handle<Value> &key)
  {
    if ( key->IsString() ) {
      NanScope();
      Local<String> keyStr = key->ToString();
      size = (size_t) keyStr->Utf8Length();
      char *data = EnsureBuffer(size + 1);
      keyStr->WriteUtf8(data);
    } else if ( Buffer::HasInstance(key) ) {
      InitFromBuffer(key);
    }
  }

  bool InputData::IsValid(void)
  {
    return buffer != NULL;
  }

  NAN_INLINE void InputData::InitFromBuffer(const Handle<Value> &key)
  {
    NanScope();
    Local<Object> bufferObj( key->ToObject() );
    size = Buffer::Length(bufferObj);
    buffer = Buffer::Data(bufferObj);
    if ( size == 0 )
      EnsureBuffer(0);
  }

  NAN_INLINE const node::encoding InputData::DetermineEncoding(
      const Handle<Value> &encoding_v)
  {
    NanScope();

    char encCstr[sizeof("utf-16le")];
    Local<String> encString = encoding_v->ToString();
    int length = encString->Length();

    if ( length > 0 && length <= sizeof(encCstr) - 1 ) {

      encString->WriteAscii(encCstr);

      if ( length > 6 ) {
        if ( strcasecmp(encCstr, "utf16le") == 0 ||
             strcasecmp(encCstr, "utf-16le") == 0 )
          return node::UCS2;
      } else if ( length == 6 ) {
        if ( strcasecmp(encCstr, "base64") == 0 )
          return node::BASE64;
        if ( strcasecmp(encCstr, "binary") == 0 )
          return node::BINARY;
      } else if ( length >= 5 ) {
        if ( strcasecmp(encCstr, "ascii") == 0 ) {
          return node::ASCII;
        } else if ( strcasecmp(encCstr, "utf-8") == 0 ) {
          return node::UTF8;
        } else if ( strcasecmp(encCstr, "ucs-2") == 0 ) {
          return node::UCS2;
        }
      } else if ( length >= 4 ) {
        if ( strcasecmp(encCstr, "utf8") == 0 ) {
          return node::UTF8;
        } else if ( strcasecmp(encCstr, "ucs2") == 0 ) {
          return node::UCS2;
        }
      }
      if ( strcasecmp(encCstr, "hex") == 0 )
        return node::HEX;
    }
    return node::UTF8;
  }
 
  size_t InputData::length() const { return size; }
 
  char* InputData::operator*() { return buffer; }
 
  const char* InputData::operator*() const { return buffer; }
 
  InputData::~InputData()
  {
    if ( ownBuffer ) delete[] buffer;
  }

  NAN_INLINE char *InputData::EnsureKeyBuffer(size_t)
  {
    return keyBuffer;
  }

  NAN_INLINE char *InputData::EnsureBuffer(size_t bytelength)
  {
    if ( bytelength > NODE_MURMURHASH_KEY_BUFFER_SIZE ) {
      ownBuffer = true;
      return buffer = new char[bytelength];
    }
    return buffer = EnsureKeyBuffer(bytelength);
  }

  char InputData::keyBuffer[NODE_MURMURHASH_KEY_BUFFER_SIZE];

}