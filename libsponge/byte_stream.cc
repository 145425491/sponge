#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity)
:_stream(), _capacity(capacity), _written_size(0), _read_size(0), _end(false), _error(false){}

size_t ByteStream::write(const string &data) { 
    if(_end) return 0;
    size_t count = 0;
    while (0 < _capacity - _stream.size() && count < data.size()){   
        _stream.push_back(data[count++]);
    }
    _written_size += count;
    return count;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const { 
    size_t pop_size = min(len, _stream.size());
    return string(_stream.begin(), _stream.begin() + pop_size);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t pop_size = min(len, _stream.size());
    _read_size += len;
    for (size_t i = 0; i < pop_size; i++){ 
        _stream.pop_front();
    }
 }

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string res = this->peek_output(len);
    this->pop_output(len);
    return res;
}

void ByteStream::end_input() {_end = true;}

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const { return _stream.size(); }

bool ByteStream::buffer_empty() const { return _stream.empty(); }

bool ByteStream::eof() const { return _end && _stream.empty(); }

size_t ByteStream::bytes_written() const { return _written_size; }

size_t ByteStream::bytes_read() const { return _read_size; }

size_t ByteStream::remaining_capacity() const { return _capacity - _stream.size(); }
