code untuk soal nomor 3 berfungsi untuk



CARA TESTING:

1. SETUP:
   - Upload code ini ke ESP32 #1 (Sender)
   - Upload code Receiver ke ESP32 #2
   - Jalankan Receiver dulu, catat MAC Address nya
   - Ganti receiverMAC[] di line 10 dengan MAC Address receiver
   - Re-upload code Sender ini

2. TESTING RANGE:
   - Mulai dari jarak dekat (1 meter)
   - Pindahkan receiver perlahan sambil monitor Serial
   - Catat jarak ketika packet loss mulai terjadi
   - Catat jarak maksimal ketika koneksi putus total

3. DATA YANG DICATAT:
   Distance (m) | Success Rate (%) | Notes
   1            | 100              | Perfect
   5            | 100              | Stable
   10           | 98               | Occasional drop
   15           | 85               | Through 1 wall
   20           | 50               | Through 2 walls
   25           | 0                | Connection lost

4. KONDISI TESTING:
   - Indoor apartment (with walls)
   - Note obstacles: walls, furniture, etc.
   - Test di berbagai ruangan

5. RESET STATISTICS:
   - Tekan tombol RESET di ESP32 untuk reset counter

OUTPUT:
-Testing fisik dengan 2 ESP32
-Catat semua hasil di template
-Ambil foto/screenshot setup dan results
-Tulis analysis kenapa ada perbedaan dengan teori