# nodetalk
esp8266 mqtt and 0.96 oled image diplay


Made with VS code PlatformIO
 
IDE is accessing usb port as a non-root, thats why it is neccessery to: 
sudo gpasswd --add ${USER} dialout OR sudo adduser $USER dialout (logout and login afterwards)


Running mqtt on doker:
docker run -ti -p 1883:1883 -p 9001:9001 --name mqtt toke/mosquitto
if you get permission error on log file writing, dont create mqtt folder manualy, instead mount it on doker, and then look at the folder with: ls -n, and add doker image the same UID with user: "ID"
