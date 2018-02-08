# coin

C++ HTTP client for

http://www.cryptocurrencychart.com/api/methods

Dependencies
-----

cmake and OpenSSL (included for Windows build)

<pre>
sudo apt-get install cmake

sudo apt install openssl

</pre>

Building
------------
<pre>
cmake -H. -Bbuild
cd build
make
</pre>

This uses CMakeLists.txt at root and builds in folder "build"

Usage
------------
<pre>
./https_coin -k KEY -s SECRET -c CODE -e END
</pre>

where CODE is a coin code in list coin_list.json and END is an end date, format as YYYY-MM-DD

Example
------------

<pre>
./https_coin -k KEY -s SECRET -c BTC -e 2018-01-01
</pre>


Testimonials
------------
> Really cool, good work!

Bastiaan Grutters, author of http://www.cryptocurrencychart.com