#include "heap/values-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/list-inl.h"

namespace neutrino {

enum Instruction {
  __first_instruction
#define DEFINE_INSTRUCTION(n, NAME) , NAME = n
FOR_EACH_INSTRUCTION(DEFINE_INSTRUCTION)
#undef DEFINE_INSTRUCTION
};

MAKE_ENUM_INFO_HEADER(Instruction)
#define MAKE_ENTRY(n, NAME) MAKE_ENUM_INFO_ENTRY(NAME)
FOR_EACH_INSTRUCTION(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

Image::Image(uint32_t size, const uint8_t *data)
    : size_(size)
    , cursor_(0)
    , data_(data)
    , registers_(NULL) { }

Data *&Image::reg(uint32_t index) {
  ASSERT(index < register_count_);
  return registers_[index];
}

bool Image::reset() {
  if (size_ < kHeaderSize) return false;
  uint32_t first_word = read_word();
  if (first_word != kMagicNumber) return false;
  code_size_ = read_word();
  if (size_ < kHeaderSize + code_size_) return false;
  register_count_ = read_word();
  if (register_count_ > size_) return false;
  registers_ = new Data*[register_count_];
  for (uint32_t i = 0; i < register_count_; i++)
    reg(i) = Nothing::make();
  return true;
}

template <class C>
static inline C *load_cast(Data *val) {
  return reinterpret_cast<C*>(val);
}

bool Runtime::load_image(Image &image) {
  list_buffer<Data*> stack;
  list_buffer<Object*> fixups;
  IF_DEBUG(list_buffer<Value*> values);
  while (image.has_more()) {
    uint32_t instr = image.read_word();
    switch (instr) {
    case NEW_CLASS: {
      Class *chlass = load_cast<Class>(stack.pop());
      Tuple *methods = load_cast<Tuple>(stack.pop());
      InstanceType instance_type = static_cast<InstanceType>(image.read_word());
      Class *result = cast<Class>(heap().new_class(instance_type));
      result->set_chlass(chlass);
      result->set_methods(methods);
      IF_DEBUG(values.push(result));
      stack.push(result);
      break;
    }
    case NEW_STRING: {
      uint32_t length = image.read_word();
      char *buffer = new char[length + 1];
      buffer[length] = '\0';
      for (uint32_t i = 0; i < length; i++)
        buffer[i] = image.read_word();
      String *result = cast<String>(heap().new_string(string(buffer, length)));
      delete[] buffer;
      stack.push(result);
      IF_DEBUG(values.push(result));
      break;
    }
    case NEW_NUMBER: {
      uint32_t value = image.read_word();
      stack.push(Smi::from_int(value));
      break;
    }
    case NEW_CODE: {
      uint32_t length = image.read_word();
      Code *code = cast<Code>(heap().new_code(length));
      for (uint32_t i = 0; i < length; i++) {
        uint32_t next = image.read_word();
        code->at(i) = next;
      }
      IF_DEBUG(values.push(code));
      stack.push(code);
      break;
    }
    case NEW_LAMBDA: {
      uint32_t argc = image.read_word();
      Tuple *literals = load_cast<Tuple>(stack.pop());
      Code *code = load_cast<Code>(stack.pop());
      Lambda *result = cast<Lambda>(heap().new_lambda(argc, code, literals));
      stack.push(result);
      IF_DEBUG(values.push(result));
      break;
    }
    case NEW_METHOD: {
      Lambda *lambda = load_cast<Lambda>(stack.pop());
      String *name = load_cast<String>(stack.pop());
      Method *result = cast<Method>(heap().new_method(name, lambda));
      stack.push(result);
      IF_DEBUG(values.push(result));
      break;
    }
    case NEW_DICTIONARY: {
      Tuple *elements = load_cast<Tuple>(stack.pop());
      Dictionary *result = cast<Dictionary>(heap().new_dictionary(elements));
      stack.push(result);
      IF_DEBUG(values.push(result));
      break;
    }
    case NEW_TUPLE: {
      uint32_t length = image.read_word();
      Tuple *tuple = cast<Tuple>(heap().new_tuple(length));
      for (uint32_t i = 0; i < length; i++)
        tuple->at(length - i - 1) = load_cast<Value>(stack.pop());
      stack.push(tuple);
      IF_DEBUG(values.push(tuple));
      break;
    }
    case PENDING_REGISTER: {
      uint32_t reg = image.read_word();
      stack.push(PendingRegister::make(reg));
      break;
    }
    case SCHEDULE_FIXUP: {
      fixups.append(load_cast<Object>(stack.peek()));
      break;
    }
    case STORE_REGISTER: {
      uint32_t index = image.read_word();
      image.reg(index) = stack.peek();
      break;
    }
    case LOAD_REGISTER: {
      uint32_t index = image.read_word();
      Data *value = image.reg(index);
      ASSERT(!is<Nothing>(value));
      stack.push(value);
      break;
    }
#define MAKE_ROOT_SETTER(Type, name, NAME, allocator)                \
    case SET_##NAME:                                                 \
      roots().name() = load_cast<Type>(stack.pop());                 \
      break;
FOR_EACH_ROOT(MAKE_ROOT_SETTER)
#undef MAKE_ROOT_SETTER
    default:
      UNHANDLED(Instruction, instr);
      return false;
    }
  }
  for (uint32_t i = 0; i < fixups.length(); i++) {
    Object *obj = fixups[i];
    obj->for_each_field(Image::fixup_field, &image);
  }
#ifdef DEBUG
  for (uint32_t i = 0; i < values.length(); i++)
    values[i]->validate();
#endif
  return true;
}

void Image::fixup_field(Data **field, void *data) {
  if (is<PendingRegister>(*field)) {
    PendingRegister *pending = cast<PendingRegister>(*field);
    Image &image = *reinterpret_cast<Image*>(data);
    *field = image.reg(pending->index());
  }
}

bool Image::has_more() {
  return cursor_ < size_;
}

uint32_t Image::read_word() {
  if (!has_more()) return 0;
  uint8_t b0 = data_[cursor_ + 0];
  uint8_t b1 = data_[cursor_ + 1];
  uint8_t b2 = data_[cursor_ + 2];
  uint8_t b3 = data_[cursor_ + 3];
  cursor_ += kWordSize;
  return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

}
