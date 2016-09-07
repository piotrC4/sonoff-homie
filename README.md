Homie based firmware for Sonoff wifi relay

## Features:
* ON/OFF relay
* Timer based relay
* Configurable default (on boot) relay state - (MQTT independent)

## commands

<table>
<tr>
  <th>Property</th>
  <th>Message format</th>
  <th>Direction</th>
  <th>Description</th>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayState</td>
  <td>[ON|OFF]</td>
  <td>Device → Controller</td>
  <td>Current state of relay</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayState/set</td>
  <td>[ON|OFF]</td>
  <td>Controller → Device</td>
  <td>Change relay state</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayInitMode</td>
  <td>[0|1]</td>
  <td>Device → Controller</td>
  <td>Current boot mode 1 - relay ON, 0 - relay OFF</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayInitMode/set</td>
  <td>[0|1]</td>
  <td>Controller → Device</td>
  <td>Set Boot mode 1 - relay ON, 0 - relay OFF</td>
</tr>
<tr>
  <td>_HOMIE_PREFIX_/_node-id_/relay01/relayTimer/set</td>
  <td>\d+</td>
  <td>Controller → Device</td>
  <td>Turn on Relay for specific no. of seconds</td>
</tr>
</table>
