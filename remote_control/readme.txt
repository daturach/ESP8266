remote_control.ino and config.h belong to the same project. That sketch controls a relay plugged right on top of the Wemos board.
php_4_joomla_cms.php should be put on a server running PHP. This file is used to control the ESP/relay board running remote_control.ino

Final product:
The relay is switching OFF/ON a 5 VDC power supply based on the settings defined in the PHP script.

https://github.com/daturach/ESP8266/blob/master/remote_control/wemos_mini_and_relay_boards.jpg

Todo: update ino file to support https requests
Done! Added SSL, See in folder remote_control_secure

Compiled with Arduino 1.6.9
