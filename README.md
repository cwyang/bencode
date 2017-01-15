# bencode [![Build Status](https://travis-ci.org/cwyang/bencode.svg?branch=master)](https://travis-ci.org/cwyang/bencode) [![Coverage Status](https://img.shields.io/codecov/c/github/cwyang/bencode.svg)](https://codecov.io/gh/cwyang/bencode)

A robust C library for reading and writing Bittorrent bencode data.

License
-------
Apache license v2.0.

APIs
----
```
extern be_node_t *be_decode(const char *inBuf, size_t inBufLen, size_t *readAmount);
extern ssize_t be_encode(const be_node_t *node, char *outBuf, size_t outBufLen);
extern void be_free(be_node_t *node);
extern void be_dump(be_node_t *node);
```
* when `be_decode()` fails, it returns `NULL` and set `errno` to:
  - `ENOMEM`: memory allocation failed
  - `EINVAL`: invalid bencode data
  - `ELOOP`:  max depth reached (default: 10)
* `be_encode(node, NULL, 0)` returns the size of output buffer to be allocated. 
  You can malloc and call `be_encode()` again with alloc'ed buffer.


Build and Test
--------
```
$ make && make test
--(snip)--
{ "test": [ "test"]
, "announce": "udp://tracker.openbittorrent.com:80"
, "creation date": 1327049827
, "info": { "length": 20
          , "name": "sample.txt"
          , "piece length": 65536
          , "pieces": "..R....x...d.......1"
          , "private": 1}}
All tests passed!
```

Thank you for reading :-)
