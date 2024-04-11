# READTYCH-DrawingDemo

Simple integration of code to read Wacom tablet with code to control eInk display in tight loop.  

## BCM2835 library installation ##
BCM2835 need to be installed on the device. Follow the procedure bwelow for installation:

wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz

tar zxvf bcm2835-1.68.tar.gz

cd bcm2835-1.68

./configure

make

sudo make check

sudo make install

## Installation ##

git clone https://github.com/ionlabhouston/READTYCH-DrawingDemo.git

cd READTYCH-DrawingDemo
make
