
@echo off
if not defined DevEnvDir (
 call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
 if errorlevel 1 goto :eof
)

set root=%cd%
:: replace the character string '\' with '/' needed for cmake
set root_cmake=%root:\=/%
echo cmake root is %root_cmake%

:build_ssl
if exist %root%\openssl-1.1.0g\libssl.lib (
 echo skipping openssl-1.1.0g build
) else (
  echo building openssl-1.1.0g
  pushd openssl-1.1.0g
  perl Configure VC-WIN32 no-asm no-shared --debug --prefix=%root%\ssl 
  nmake
  nmake install
  popd
)


cmake -H. -Bbuild ^
-DOPENSSL_INCLUDE:PATH=%root_cmake%/ssl/include ^
-DOPENSSL_LIBRARY:FILE=%root_cmake%/ssl/lib/libssl.lib ^
-DCRYPTO_LIBRARY:FILE=%root_cmake%/ssl/lib/libcrypto.lib
msbuild %root%\build\coin.sln /target:build /property:configuration=debug