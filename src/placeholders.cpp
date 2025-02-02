#include "malib/FixedStringBuffer.hpp"
#include "malib/BufferReader.hpp"
#include "malib/RingBuffer.hpp"

template class malib::FixedStringBuffer<10>;
template class malib::BufferReader<malib::RingBuffer<char, 3>, 3>;
