Sliding Window Protocol
=======================
<p>
Dikutip dari <a href="https://en.wikipedia.org/wiki/Sliding_window_protocol"> Halaman Wikipedia </a> : <br>
<i>A sliding window protocol is a feature of packet-based data transmission protocols. Sliding window protocols are used where reliable in-order delivery of packets is required.</i>
<br> Yang dapat diartikan : Sliding window protocol adalah sebuah fitur dari protokol transmisi data berbasis paket. Dalam tugas ini, kami mengimplementasi sliding window protocol untuk mengirim data dengan menggunakan <i>socket UDP</i> menggunakan bahasa <b>C++</b>.</p>


## Cara kerja Sliding Window Protocol

Implementasi Sliding Window Protocol mengharuskan receiver menerima data yang <i>out of order</i>. sehingga receiver harus bisa melakukan <i>buffer</i> packetnya. Protokol melakukan transmit ulang data dengan 2 kondisi yaitu :

1. Ketika receiver tidak menerima paket atau ACK dari receiver tidak diterima oleh sender, sender akan mentransmit ulang packet setelah timeout karena asumsi packet tidak diterima receiver atau ACK nya hilang.
2. Ketika receiver menerima paket tetapi paket itu ternyata salah dengan melakukan validasi checksum. Maka receiver akan memberikan NAK <i>(Negative Acknowledgment)</i> untuk memberitahu sender dan sender akan melakukan transmit ulang paket yang salah itu.

### Penjelasan paket dan buffer
Data yang dikirim bisa saja berukuran besar sehingga data dibagi menjadi beberapa buffer dan transmisi dilakukan per-buffer. Setiap buffer, paket dikirim dengan sequence number dimulai dari 0. Sehingga untuk menghindari ambiguitas buffer dan recevier, ukuran buffer harus lebih besar dari ukuran window (biasanya 2 kali lipat). Data yang dikirim oleh sliding window protocol dibagi menjadi beberapa packet (frames) yang diberikan ukuran maksimal packet 1034 bit. Setiap paket mempunyai ID unik yang disebut sequence number yang dipakai untuk menyusun paket-paket itu setelah transmisi data selesai. 

### Receiver
Receiver mendefinisikan window dengan memberi tanda menggunakan variabel LFR <i>(Last Frame Received)</i> dan LAF <i>(Largest Acceptable Frame)</i>. Ketika receiver menerima paket, receiver akan mengirim ACK paket tersebut ketika paket itu valid (dengan validasi menggunakan checksum), jika tidak valid, receiver akan memberikan NAK. Receiver akan menggeser window sampai sequence number terkecil yang belum diterima yaitu LFR + 1.
Pada <b>recvfile.cpp</b>, paket diterima dari sendfile, dilakukan pengecekan apakah paket yang diterima recvfile sesuai atau tidak dengan menggunakan checksum lalu mengirimkan ACK/NAK sesuai dengan hasil validasi tersebut ke sendfile. Recvfile akan mengisi buffer ke buffer hingga menjadi file hasil.

### Sender
Sender mendefinisikan window dengan memberi tanda menggunakan variabel LAR <i>(Last Acknowledgement Received)</i> dan LFS <i>(Last Frame Sent)</i>. mengirimkan semua paket di dalam window dan menunggu setiap paket dalam window itu mendapatkan ACK dari receiver. Setiap ACK diterima, sender akan menggeser window sampai sequence number terkecil yang belum menerima ACK yaitu LAR + 1. Window tidak digeser setiap menerima ACK dikarenakan ACK bisa saja sampai tidak sesuai urutan pengiriman paket.
Pada <b>sendfile.cpp</b> digunakan 2 thread untuk implementasi sliding windows protocol. thread pertama digunakan untuk mengirimkan paket ke client, thread kedua dipakai untuk menerima ACK atau NAK dari client.

## Documentation
<ul>
	<li>ack.cpp : Berikut fungsi dan kegunaannya dalam file ack.cpp
		<ol>
			<li>create_ack : Digunakan untuk membuat ACK (digunakan di recvfile yang nanti nya akan dikirim ke sendfile)</li>
			<li>read_ack : Digunakan untuk membaca ACK (digunakan di sendfile untuk membaca ACK yang dikirim oleh recvfile)</li>
		</ol>
	</li>
	<li>packet.cpp : Berikut fungsi dan kegunaannya dalam file packet.cpp
		<ol>
			<li>create_packet : Digunakan untuk membuat paket (yang akan dikirimkan oleh sendfile dan diterima oleh recvfile)</li>
			<li>read_packet : Digunakan untuk membaca paket (yang akan dibaca oleh recvfile setelah dikirim oleh sendfile)</li>
			<li>count_checksum : fungsi validasi checksum apakah paket diterima sesuai atau tidak</li>
		</ol>
	</li>
	<li>recvfile.cpp : program akan menerima paket. Untuk setiap paket, program akan mengecek apakah paket error atau tidak, jika error, program akan mengirimkan NAK kepada sender. Jika paket tidak error, maka paket akan dimasukan ke dalam buffer. Buffer kemudian akan dituliskan ke file eksternal. Hal ini akan diteruskan hingga eot, setelah mendapatkan sinyal eot, program akan berakhir.
	</li>
	<li>sendfile.cpp : Berikut fungsi dan kegunaannya dalam file sendfile.cpp
		<ol>
			<li>get_ack : digunakan untuk menerima ACK/NAK dari recvfile</li>
			<li>main : Program pertama-tama akan membuka file kemudian membaca besarnya data sesuai ukuran maximum buffer. Kemudian program akan mengirim packet-packet data sejumlah ukuran window. Program akan melakukan iterasi untuk mengecek apakah paket belum dikirim atau paket telah dikirim namun ACK belum didapatkan hingga timeout, jika kondisi ini terpenuhi maka program akan mengirim paket. Jika ACK dengan seqnum bersangkutan telah diterima, maka program akan menggeser buffer. Hal ini akan diteruskan hingga seluruh byte data telah dibaca dan dikirimkan.</li>
		</ol>
	</li>
	<li>timer.h : Menentukan nilai timeout, waktu sekarang, <i>timestamp</i>, dan waktu yang sudah berlalu <i>(elapsed time)</i></li>
</ul>

## Cara Menjalankan Program

### Instal
Install **g++** dan jalankan di **Ubuntu** atau **macOS**. Jalankan command berikut:
1. Make
```
$ make
```

### Menjalankan Program
1. Jalankan Receiver terlebih dahulu
```
$ ./recvfile <filename> <window_size> <buffer_size> <port>
```
contoh:
```
$ ./recvfile data/hasil.txt 100 200 3000
```
2. Jalankan Sender
```
$ ./sendfile <filename> <window_size> <buffer_size> <ip> <port>
```
contoh:
```
$ ./sendfile data/contoh.txt 100 200 localhost 3000
```

## Simulasi Packet Loss
To test the ability of retaining integrity and order of the transmission in the case of a packet loss, we can simulate a packet loss using this command on Linux:
Untuk melakukan test transmisi di saat packet loss, dilakukan simulasi packet loss di linux dengan menggunakan NetEm dengan command sebagai berikut :
```
$ sudo tc qdisc add dev lo root netem loss <loss_rate>
```
contoh:
```
$ sudo tc qdisc add dev lo root netem loss 50%
```
untuk mengembalikan kesemula:
```
$ sudo tc qdisc del dev lo root netem
```

## Note:
`buffer_size` and `window_size` di recvfile dan sendfile harus bernilai sama. 

## Dibuat oleh :
1. Muhammad Sulthan Adhipradhana 13516035
2. Christian Jonathan 13516092
3. Ahmad Izzan 13516116

### Pembagian Kerja
1. Muhammad Sulthan Adhipradhana : sendfile +debug	- 33%
2. Christian Jonathan : Packet + ACK + debug		- 33%
3. Ahmad Izzan : checksum + recvfile + debug		- 33%