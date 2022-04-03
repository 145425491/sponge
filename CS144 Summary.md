# CS144  手写一个属于自己的TCP

## Lab 0 Networking WarmUp

### 1. Set Up Your Virtual Machine

  借着CS144 的机会终于把之前的破虚拟机换了，选择了更新也更适合图形化界面操作的VMware Workstation，和以前的VirtualBox比起来不能说难兄难弟，只能说云泥之别。甚至可以流畅的使用虚拟机看bilibili。不过对虚拟机和vim的配置也成了这几天的主要任务，尤其是vim的ide化，花了我整整两天，早知道就直接使用VScode了，有点小后悔。话不多说，接下来总结一下cs144的lab0 warmup。 

  首先是虚拟机和实验环境的配置，这里我所选择的是 use your own virtual machine，需要的插件参考[BYO](https://stanford.edu/class/cs144/vm_howto/vm-howto-byo.html)，基本上只需要下载几个网络相关的package即可。

### 2. Networking By Hand

  这里需要手动使用telnet访问cs144的链接http://cs144.keithw.org/hello，基本上教程已经手把手教了所需要的所有步骤，需要注意的是大小写以及速度，如果速度过慢target web会直接关闭连接。具体实现如下

```
$ telnet cs144.keithw.org http
Trying 104.196.238.229...
Connected to cs144.keithw.org.
Escape character is '^]'.
GET /hello HTTP/1.1
Host: cs144.keithw.org
Connection: close

HTTP/1.1 200 OK
Date: Mon, 28 Mar 2022 14:26:18 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Connection: close
Content-Type: text/plain

Hello, CS144!
Connection closed by foreign host.
```

由于我没有Stanford账号，所以放弃了Send Yourself an Email，而Listening and Connecting 也非常简单，按部就班即可

### 3. Writing Webget

  去官方的github库里clone下[sponge](https://github.com/CS144/sponge.git)，去/sponge/apps/webget.cc里编辑get_url函数。

  同时，clone后需要进入cs144目录部署一下环境，具体实现如下

```
mkdir -p /sponge/build
cd /sponge/build
cmake ..
make
```

在实现get_url之前，先要去官网里熟悉各种接口，尤其是TCPSocket, Socket, Address接口。（由于英文文档实在难读，所以选择了steal chicken，借鉴了大佬的代码...)，大佬代码如下：

```c++
void get_URL(const string &host, const string &path) {
    TCPSocket socket;
    socket.connect(Address(host, "http"));
    string info =   "GET " + path + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n";
    socket.write(info); 
    socket.shutdown(SHUT_WR); // 不太确定与Connection: close作用是否重复
    for (auto recvd = socket.read(); !socket.eof(); recvd = socket.read())
        cout << recvd;
    socket.close();
}
```

### 4. Finish Byte Stream

最后是Lab0的重头戏（重点所在），实现一个支持写入和读取的具有缓冲区的byte__stream类，这个类将在以后的实验中作为底层结构层层封装。

不要被英文文档和前面的接口吓到，这次的byte_stream本质上只是一个简单的deque结构，支持一端写入和一端读取，也并不需要实现什么复杂的函数。

下面是我在byte_stream.hh里实现的private member，也就是private下的几个简单成员变量。



```c++
#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <string>
#include <deque>

//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    // Your code here -- add private members as necessary.

    // Hint: This doesn't need to be a sophisticated data structure at
    // all, but if any of your tests are taking longer than a second,
    // that's a sign that you probably want to keep exploring
    // different approaches.
    std::deque<char> _stream = {};
    size_t _capacity;
    size_t _written_size;
    size_t _read_size;
    bool _end;
    bool _error{};  //!< Flag indicating that the stream suffered an error.

  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity);

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const;

    //! Signal that the byte stream has reached its ending
    void end_input();

    //! Indicate that the stream suffered an error.
    void set_error() { _error = true; }
    //!@}

    //! \name "Output" interface for the reader
    //!@{

    //! Peek at next "len" bytes of the stream
    //! \returns a string
    std::string peek_output(const size_t len) const;

    //! Remove bytes from the buffer
    void pop_output(const size_t len);

    //! Read (i.e., copy and then pop) the next "len" bytes of the stream
    //! \returns a string
    std::string read(const size_t len);

    //! \returns `true` if the stream input has ended
    bool input_ended() const;

    //! \returns `true` if the stream has suffered an error
    bool error() const { return _error; }

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const;

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const;

    //! \returns `true` if the output has reached the ending
    bool eof() const;
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const;

    //! Total number of bytes popped
    size_t bytes_read() const;
    //!@}
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH
```

还有最终实现的byte_stream.cc

```c++
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
```

最后使用check_lab0检查通过即可，不过需要先去build目录下make后测试。

## Lab 1: stitching substrings into a byte stream