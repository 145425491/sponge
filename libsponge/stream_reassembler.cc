#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), _eof_pos(0), _next_idx(0), _eof_flag(false),_unassembled_byte_num(0),  _unassembled_bytes({}){}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof){
        _eof_pos = index + data.size();
        _eof_flag = true;
    }
    if (index + data.size() < _next_idx || index >= _next_idx + _output.remaining_capacity()){
        if (_eof_flag && _next_idx == _eof_pos)
        _output.end_input();
        return;
    }
    string correction = data.substr(0, _output.bytes_read() + _capacity - index);
    if ( index <= _next_idx ){
        _next_idx += _output.write(correction.substr(_next_idx - index));
    }else{
        substring tmp(correction, index);
        bool insertable = true;
        if (!_unassembled_bytes.empty()){
            for (Unaseembled::iterator it = _unassembled_bytes.begin(); it != _unassembled_bytes.end();){
                if (it->_idx <= tmp._idx && it->_str.size() + it->_idx >= tmp._idx + tmp._str.size()){
                    insertable = false;
                    break;
                }else if (it->_idx >= tmp._idx && it->_str.size() + it->_idx <= tmp._idx + tmp._str.size()){
                    _unassembled_byte_num -= it->_str.size();
                    it = _unassembled_bytes.erase(it);
                }else if (it->_idx < tmp._idx && it->_str.size() + it->_idx >= tmp._idx){
                    tmp._str = it->_str + tmp._str.substr(it->_str.size() + it->_idx - tmp._idx);
                    tmp._idx = it->_idx;
                    _unassembled_byte_num -= it->_str.size();
                    it = _unassembled_bytes.erase(it);
                }else if (it->_str.size() + it->_idx > tmp._idx + tmp._str.size()&& tmp._idx + tmp._str.size() >= it->_idx){
                    tmp._str = tmp._str + it->_str.substr(tmp._idx + tmp._str.size() - it->_idx);
                    _unassembled_byte_num -= it->_str.size();
                    it = _unassembled_bytes.erase(it);
                }else{
                    it++;
                }
            }
        }
        if (insertable){
            _unassembled_byte_num += tmp._str.size();
            _unassembled_bytes.insert(tmp);
        }
        
    }
    while (!_unassembled_bytes.empty() && _unassembled_bytes.begin()->_idx <= _next_idx){
        substring tmp = *_unassembled_bytes.begin();
        if (tmp._idx + tmp._str.size() > _next_idx)
            _next_idx += _output.write(tmp._str.substr(_next_idx - tmp._idx));
        _unassembled_byte_num -= _unassembled_bytes.begin()->_str.size();
        _unassembled_bytes.erase(_unassembled_bytes.begin());
    }
    if (_eof_flag && _next_idx == _eof_pos)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_byte_num; }

bool StreamReassembler::empty() const { return _unassembled_bytes.empty(); }
