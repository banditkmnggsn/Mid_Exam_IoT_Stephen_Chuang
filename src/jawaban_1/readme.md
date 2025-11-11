This code functions to show data from the DHT11 and BH1750 (light sensor), which is sent to EMQX for 1 minute, with an additional feature to subscribe to the path "stephenchuang/2702269135/control" for feedback messages from MQTT to my Lolin32 Lite.

DHT 11 -> GPIO 14
BH1750 -> SDA -> 18
          SCL -> 19