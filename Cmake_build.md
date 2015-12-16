# Dependencies #
> - cmake

## All systems: ##

- prepare workspace for build with cmake:
```
cd qomp
mkdir build
cd build
```


---

## OS Linux (using cmake): ##

- prepare sources to build with Qt4:
```
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
```
- prepare sources to build with Qt5:
```
cmake -DCMAKE_INSTALL_PREFIX=/usr -DUSE_QT5=ON ..
```
- build qomp:
```
make
```
- install qomp
```
make install
```

---

## OS Windows (using cmake and mingw32): ##

- create **build\_qomp.cmd** file in qomp root directory

- open this file in text editor and paste this:
```
set QMAKESPEC=win32-g++

set /p BINTYPE=Do you want to build qomp debug version [y/n(default)]:%=%
@echo _ 

::change this part of file
set QTDIR=C:\Qt\5.3\5.3\mingw482_32
set MINGWDIR=C:\Qt\5.3\Tools\mingw482_32
set CMAKEDIR=C:\build\cmake\
set TAGLIB=C:\buid\taglib
::

if /i "%BINTYPE%"=="y" (
set BTYPE=Debug
) else (
set BTYPE=Release
)

set PATH=%QTDIR%\;%QTDIR%\bin;%MINGWDIR%;%MINGWDIR%\bin;%CMAKEDIR%

set PREFIX=qomp_%BTYPE%
set BDIR=build_qomp_%BTYPE%
mkdir %BDIR%
cd %BDIR%
%CMAKEDIR%\bin\cmake -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=%PREFIX% -DCMAKE_BUILD_TYPE=%BTYPE% -DTAGLIB_ROOT=%TAGLIB% -DUSE_QT5=ON ..
pause
@echo Runing mingw32-make
mingw32-make
mingw32-make install
pause

@goto exit

:exit
pause
```
- change this part of file with your values
```
set QTDIR=C:\Qt\5.3\5.3\mingw482_32
set MINGWDIR=C:\Qt\5.3\Tools\mingw482_32
set CMAKEDIR=C:\build\cmake\
set TAGLIB=C:\buid\taglib
```
- save and run this `*`.cmd file.

Qomp files will be installed into **qomp\_Release** or **qomp\_Debug** directory

**- _Or you can run QtCreator and open CMakeLists.txt file._**

---

## Some useful cmake flags ##
**_To build qomp with only needed plugins just add this flag:_**
```
-DBUILD_PLUGINS="filesystemplugin;urlplugin" ..
```
Possible values for -DBUILD\_PLUGINS variable:
```
filesystemplugin
urlplugin
prostopleerplugin
myzukaruplugin
notificationsplugin
yandexmusicplugin
lastfmplugin
tunetofileplugin
mprisplugin
```
**_To set path to taglib library (useful for windows):_**
```
-DTAGLIB_ROOT="YOUR_PATH"
```
**_To set build type:_**
```
-DCMAKE_BUILD_TYPE=Release
or
-DCMAKE_BUILD_TYPE=Debug
```
**_To build with Qt5:_**
```
-DUSE_QT5=ON
```
**_To install qomp to your path:_**
```
-DCMAKE_INSTALL_PREFIX="YOUR_PATH"
```
for example:
```
cmake -DCMAKE_INSTALL_PREFIX=./install ..
```
all files will be installed into **build/install** directory