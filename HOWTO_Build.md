# Dependencies #

---

- Qt4.8.x or Qt5

- Phonon (only for Qt4)

- taglib

---

# Getting qomp sources #

- clone qomp git repository to local directory:
```
git clone https://code.google.com/p/qomp/
```

- initialize translations submodule:
```
cd qomp
git submodule init
git submodule update
```

## Update qomp surces ##

```
cd qomp
git pull
git submodule update
```


---

# [Cmake build instruction](https://code.google.com/p/qomp/wiki/Cmake_build) #

---





---

# QMAKE build instructions #

---



---

## OS X ##
Make sure you have taglib compilled and installed as framework.
Also you need Growl framework. Qt4.8.x is only supported.
```
git clone https://code.google.com/p/qomp.scripts/
cd qomp.scripts/macx
./qompbuild
```
The script will make all the work and then you can find your .dmg under ~/qomp-build folder

---

## LINUX ##
```
cd qomp
qmake PREFIX=/usr
make
sudo make install
```

---

## WINDOWS ##
```
cd /d qomp
qmake
mingw32-make
```
And then you can find binary files under bin folder