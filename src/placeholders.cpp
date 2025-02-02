#include "FixedStringBuffer.hpp"
#include "BufferReader.hpp"
#include "RingBuffer.hpp"

template class malib::FixedStringBuffer<10>;
template class malib::BufferReader<malib::RingBuffer<char, 3>, 3>;
