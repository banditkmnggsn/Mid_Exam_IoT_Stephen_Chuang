code ini berfungsi untuk membuat ESP saya dapat multitasking dibuktikan dengan saat menjalankan fungsi blinking led ESP saya masih dapat cek status "ALERT" dari feedback EMQX jadi ketika ALERT di trigger ESP akan memberhentikan fungsi blinking dan akan masuk ke mode alert dengan menyalakan led static selama 10 detik

pada code ini saya juga merubah WM menjadi autoconnect saja tidak pakai pass agar lebih cepat di konfigurasi

Pin led "ALERT" -> GPIO 33
untuk mode blinking saya memakai led builtin