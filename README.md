[Homie](https://github.com/marvinroger/homie) based firmware for Sonoff wifi relay or any ESP8266 based relay.

## Features:
* ON/OFF relay
* Timer based relay
* Configurable default (on boot) relay state - (MQTT independent)
* Local button on/off
* Keepalive feature - device will reboot if not receive keepalive message in given time
* Reverse mode - ON command means relay off, OFF command means relay on
* Watchdog timer
* All Homie buildin features (OTA,configuration)

## Build and upload
 * Attache module via USB-RS232 adapter
 * Set proper serial port number in plantformio.ini file (upload_port variable)
 * reboot module into program mode
 * Flash module:
   * execute <code>pio run --target upload --environment sonoff</code> for SONOFF or <code>pio run --target upload --environment generic</code> for generic ESP8266
   * In Atom editor with PlatformIO prees F7 enter environment name (sonoff or generic) and choose <code>PIO uload</code> option from the list

## MQTT messages

<table>
<tr>
  <th>Property</th>
  <th>Message format</th>
  <th>Direction</th>
  <th>Description</th>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayState</td>
  <td><code>(ON|OFF)</code></td>
  <td>Device → Controller</td>
  <td>Current state of relay</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayState/set</td>
  <td><code>(ON|OFF)</code></td>
  <td>Controller → Device</td>
  <td>Change relay state</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayTimer/set</td>
  <td><code>\d+</code></td>
  <td>Controller → Device</td>
  <td>Turn on relay for specific no. of seconds</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/keepalive/timeOut</td>
  <td><code>\d+</code></td>
  <td>Device → Controller</td>
  <td>Report of keepalive timeout value (seconds), 0 - keep alive feature is not active</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/keepalive/timeOut/set</td>
  <td><code>\d+</code></td>
  <td>Controller → Device</td>
  <td>Set Report of keepalive timeout in seconds, 0 - keep alive feature is not active</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/keepalive/tick/set</td>
  <td><code>.*</code></td>
  <td>Controller → Device</td>
  <td>Keepalive message from controller to gateway - if device will not receive during keepAliveTimeOut time slot, it will reboot, keepalive is not active when keepAliveTimeOut is equal 0</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/$online</td>
  <td><code>(true|false)</code></td>
  <td>Device → Controller</td>
  <td><code>/true</code> when the device is online, <code>false</code> when the device is offline (through LWT)</td>
</tr>
</table>
