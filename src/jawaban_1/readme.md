code ini berfungsi untuk menunjukan data dari DHT 11 dan BH1750 (sensor cahaya), di oper ke EMQX selama 1 menit, denhgan fitur tambahan yaitu subscribe ke path "stephenchuang/2702269135/control" untuk feed back berupa pesan dari MQTT ke lolin32 lite saya

DHT 11 -> GPIO 14
BH1750 -> SDA -> 18
          SCL -> 19