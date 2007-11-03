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

bool Image::reset() {
  if (size_ < kHeaderSize) return false;
  uint32_t first_word = read_word();
  if (first_word != kMagicNumber) return false;
  code_size_ = read_word();
  if (size_ < kHeaderSize + code_size_) return false;
  uint32_t register_count = read_word();
  if (register_count > size_) return false;
  registers_ = new Data*[register_count];
  return true;
}

template <class C>
static inline C *load_cast(Data *val) {
  return reinterpret_cast<C*>(val);
}

bool Runtime::load_image(Image &image) {
  list_buffer<Data*> stack;
  while (image.has_more()) {
    uint32_t instr = image.read_word();
    switch (instr) {
    case PENDING_REGISTER: {
      uint32_t reg = image.read_word();
      stack.push(PendingRegister::make(reg));
      break;
    }
    case NEW_CLASS: {
      Class *chlass = load_cast<Class>(stack.pop());
      InstanceType instance_type = static_cast<InstanceType>(image.read_word());
      Class *result = cast<Class>(heap().new_class(instance_type));
      result->set_chlass(chlass);
      stack.push(result);
      break;
    }
    case NEW_STRING: {
      uint32_t length = image.read_word();
      char *buffer = new char[length + 1];
      buffer[length] = '\0';
      for (uint32_t i = 0; i < length; i++)
        buffer[i] = image.read_word();
      stack.push(cast<String>(heap().new_string(string(buffer, length))));
      delete[] buffer;
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
      for (uint32_t i = 0; i < length; i++)
        code->at(i) = image.read_word();
      stack.push(code);
      break;
    }
    case NEW_LAMBDA: {
      uint32_t argc = image.read_word();
      Code *code = load_cast<Code>(stack.pop());
      Tuple *literals = load_cast<Tuple>(stack.pop());
      stack.push(heap().new_lambda(argc, code, literals));
      break;
    }
    case NEW_DICTIONARY: {
      Tuple *elements = load_cast<Tuple>(stack.pop());
      Dictionary *result = cast<Dictionary>(heap().new_dictionary(elements));
      stack.push(result);
      break;
    }
    case NEW_TUPLE: {
      uint32_t length = image.read_word();
      Tuple *tuple = cast<Tuple>(heap().new_tuple(length));
      for (uint32_t i = 0; i < length; i++)
        tuple->at(length - i - 1) = load_cast<Value>(stack.pop());
      stack.push(tuple);
      break;
    }
    case STORE_REGISTER: {
      uint32_t index = image.read_word();
      image.registers()[index] = stack.peek();
      break;
    }
    case LOAD_REGISTER: {
      uint32_t index = image.read_word();
      stack.push(image.registers()[index]);
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
  return true;
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
