# Final Project SISOP E12
Dari soal Final Project ini, kita diminta untuk membuat sebuah program C yang bertujuan untuk membaca data crontab dan mengaplikasikan nya selayaknya crontab yang terdapat pada LINUX. Program C yang telah kita buat nantinya akan membaca config dari `crontab.data` yang telah berisi config untuk crontab, kemudian program C yang telah dibuat dapat melakukan task secara otomatis. Berikut penjelasan secara detail mengenai code program C kami.
```c
struct conf
{
  int min;
  int hour;
  int day;
  int month;
  int day_week;
  char perintah[1001];
  time_t akses;
};
```
Untuk menyimpan konfigurasi dari crontab, kami menggunakan struct seperti yang tertera diatas dan setiap line dari crontab disimpan dalam array of struct. Lalu setelah menyimpan semua elemen yang ada di crontab ke dalam array of struct, maka selanjutnya waktu pada crontab dibandingkan dengan waktu system, dimana waktu sistem disimpan dalam struct tm dan dibandingkan mulai dari menit, jam, hari(dalam bulan), bulan, hari(dalam minggu). Ada kondisi khusus dimana jika * maka dia akan otomatis dianggap true saat pengecekan.

Untuk pengecekkan, kami menggunakan thread dimana thread akan berjalan terus sampai ada perintah unuk pthread_exit, dimana kondisi itu akan terjadi saat file crontab.data dimodified, jika saat dicek ternyata true, maka thread akan menjalankan perintah yang disimpan di dalam struct, dengan menggunakan execl, namun karena exec akan meng kill proses, maka sebelum meng exec, perlu di fork terlebih dahulu sehingga yang terkill adalah child proses. Berikut routine dari thread
```c
void *check_config(void *arg)
{
  time_t crontim = time(NULL);
  int tanda = 0;
  struct conf *myconf = (struct conf *)arg;
  while(1)
  {
    time_t systim = time(NULL);
    if(flag==0) pthread_exit(NULL);
    int res = systim - crontim; 
    int abs = check_min(myconf);
    if(abs==1 && (res>=60 || tanda==0) ) 
    {
      pid_t child = fork();
      tanda = 1;
      crontim = systim;
      if(child == 0) execl("/bin/sh", "sh", "-c", myconf->perintah, NULL);
    }
  }
}
```
supaya cron tidak dijalankan setiap detik, maka saya menyimpan waktu saat cron dijalankan (crontim) dan waktu sistem (systim), dimana jika hasil pengurangan waktu cron dan waktu sistem kurang dari 60, maka perintah tidak boleh dijalankan (kasus ini berlaku pada konfigurasi * * * * * dan sejenisnya).
