# coin



C++ HTTP client for

http://www.cryptocurrencychart.com/api/methods



Dependencies
-----
<pre>
sudo apt-get install cmake
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
./http_coin -k <KEY> -s <SECRET> -c <CODE>
</pre>



where <CODE> is a coin code in list coin_list.json



## Example

<pre>
./http_coin -k KEY -s SECRET -c BTC -e 2018-01-01
</pre>

