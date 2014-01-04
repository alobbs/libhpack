# libhpack
**libhpack** implements HPACK “**Header Compression for HTTP/2.0**”, a format adapted to efficiently represent HTTP header fields in the context of the upcoming HTTP/2.0 protocol.

[![Build Status](https://travis-ci.org/alobbs/libhpack.png?branch=master)](https://travis-ci.org/alobbs/libhpack) [![Coverage Status](https://coveralls.io/repos/alobbs/libhpack/badge.png?branch=master)](https://coveralls.io/r/alobbs/libhpack?branch=master)

## Getting Started
Build requirements include Cmake, and Check.

```
git clone --recursive https://github.com/alobbs/libhpack.git
cd libhpack
make
```
To render the documentation you'd have to execute ```make doc``` afterwards. That specific target depends on Sphinx and Doxygen.

## Community
Keep track of community news and rub shoulders with the developers:

* Follow @[http2d](https://twitter.com/http2d) on Twitter
* On `irc.freenode.net` server, in the `#cherokee` channel
* [Bug Tracker](https://github.com/alobbs/libhpack/issues): Issues and RFEs

## License
libhpack is distributed under the [Simplified BSD License](http://opensource.org/licenses/BSD-2-Clause). See the LICENSE file for more info.

## References
* HTTPbis Working Group: HPACK:  
http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-05

--  
Alvaro Lopez Ortega  
[alvaro@gnu.org](mail:alvaro@gnu.org)