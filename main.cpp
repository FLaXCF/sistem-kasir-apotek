#include <iostream>
#include <string>
#include <sqlite3.h>
#include <vector>
#include <limits>
#include <ctime>
#include <set>
#include <iomanip>
#include <locale>

#ifdef _WIN32
#define CLEAR_SCREEN "cls"
#else
#define CLEAR_SCREEN "clear"
#endif

using namespace std;

struct Transaksi {
  string namaObat;
  int jumlah;
  double totalHarga;
  double uangPembeli;
  double kembalian;
  string tanggal;
  string waktu;
  string pembeli;
};

struct Pembayaran {
  double jumlahUang;
  double kembalian;
};

sqlite3 * db;
vector < Transaksi > transaksiDatabase;
set < string > uniqueBuyers;

int callback(void * data, int argc, char ** argv, char ** azColName) {
  return 0;
}

void initializeDatabase() {
  int rc = sqlite3_open("apotek.db", & db);

  if (rc) {
    cout << "Error opening SQLite database!" << endl;
    exit(1);
  }

  const char * createObatTableSQL = "CREATE TABLE IF NOT EXISTS obat (id INTEGER PRIMARY KEY AUTOINCREMENT, nama TEXT, stok INTEGER, harga REAL);";
  const char * createTransaksiTableSQL = "CREATE TABLE IF NOT EXISTS transaksi (id INTEGER PRIMARY KEY AUTOINCREMENT, nama_obat TEXT, jumlah INTEGER, total_harga REAL, tanggal TEXT, pembeli TEXT);";
  const char * createLaporanKeuanganTableSQL = "CREATE TABLE IF NOT EXISTS laporan (id INTEGER PRIMARY KEY AUTOINCREMENT, total_pendapatan REAL);";
  const char * createPembeliTableSQL = "CREATE TABLE IF NOT EXISTS pembeli (id INTEGER PRIMARY KEY AUTOINCREMENT, nama_pembeli TEXT);";

  rc = sqlite3_exec(db, createObatTableSQL, callback, 0, 0);
  rc = sqlite3_exec(db, createTransaksiTableSQL, callback, 0, 0);
  rc = sqlite3_exec(db, createLaporanKeuanganTableSQL, callback, 0, 0);
  rc = sqlite3_exec(db, createPembeliTableSQL, callback, 0, 0);
}

void loadTransaksiFromDatabase() {
  string sql = "SELECT * FROM transaksi;";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);

  if (rc == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      Transaksi transaksi;
      transaksi.namaObat = (char * ) sqlite3_column_text(stmt, 1);
      transaksi.jumlah = sqlite3_column_int(stmt, 2);
      transaksi.totalHarga = sqlite3_column_double(stmt, 3);
      transaksi.tanggal = (char * ) sqlite3_column_text(stmt, 4);
      transaksiDatabase.push_back(transaksi);
    }
  } else {
    cout << "Error loading transactions from database: " << sqlite3_errmsg(db) << endl;
  }
  sqlite3_finalize(stmt);
}

void loadFinancialReportFromDatabase() {
  string sql = "SELECT * FROM laporan;";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);

  if (rc == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      double totalPendapatan = sqlite3_column_double(stmt, 1);
    }
  } else {
    cout << "Error loading financial report from database: " << sqlite3_errmsg(db) << endl;
  }
  sqlite3_finalize(stmt);
}

void loadBuyersFromDatabase() {
  string sql = "SELECT * FROM pembeli;";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);

  if (rc == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char * namaPembeli = sqlite3_column_text(stmt, 1);
    }
  } else {
    cout << "Error loading buyers from database: " << sqlite3_errmsg(db) << endl;
  }
  sqlite3_finalize(stmt);
}

string formatCurrency(double amount) {
  stringstream ss;
  ss.imbue(locale(""));
  ss << fixed << setprecision(2) << amount;
  string formattedAmount = ss.str();
  return formattedAmount;
}

void hanyaAngka() {
  if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits < streamsize > ::max(), '\n');
    cout << "Input tidak valid. Masukkan angka.\n";
    cout << "Tekan Enter untuk kembali ke menu...";
    cin.get();
    system(CLEAR_SCREEN);
    return;
  }
}

string tambahkanSpasi(string teks, int panjang) {
  int panjangTeks = teks.length();
  if (panjangTeks < panjang) {
    teks.append(panjang - panjangTeks, ' ');
  }
  return teks;
}

void konfirmasiKeluar(){
  cout << "Tekan Enter untuk kembali ke menu...";
  cin.ignore();
  cin.get();
}

void tambahStokObat(int obatID, int jumlah) {
  hanyaAngka();
  string sql = "SELECT * FROM obat WHERE id=" + to_string(obatID) + ";";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);

  if (rc == SQLITE_OK) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      int stok = sqlite3_column_int(stmt, 2);
      stok += jumlah;

      string updateStokSQL = "UPDATE obat SET stok=" + to_string(stok) + " WHERE id=" + to_string(obatID) + ";";
      rc = sqlite3_exec(db, updateStokSQL.c_str(), callback, 0, 0);
      if (rc != SQLITE_OK) {
        cout << "Error updating stok in database: " << sqlite3_errmsg(db) << endl;
      } else {
        cout << "Stok obat berhasil ditambahkan. Total stok sekarang: " << stok << endl;
      }
    } else {
      cout << "Obat tidak ditemukan.\n";
    }
  }
  sqlite3_finalize(stmt);
  konfirmasiKeluar();
  system(CLEAR_SCREEN);
}

void menuTambahStok() {
  cout << "Apakah Anda ingin menambahkan obat? (1: Ya, 0: Batal): ";
  int choice;
  cin >> choice;

  if (choice == 0) {
    cout << "Operasi dibatalkan. Tekan enter untuk kembali ke menu.\n";
    cin.ignore();
    cin.get();
    return;
  }
  hanyaAngka();
  int obatID, jumlah;
  system(CLEAR_SCREEN);
  cout << "Masukkan ID obat yang ingin ditambahkan stoknya: ";
  cin >> obatID;
  cout << "Masukkan jumlah stok yang ingin ditambahkan: ";
  cin >> jumlah;
  tambahStokObat(obatID, jumlah);
}

void tambahObat(string nama, int stok, double harga) {
  string checkExistenceSQL = "SELECT * FROM obat WHERE nama='" + nama + "';";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, checkExistenceSQL.c_str(), -1, & stmt, 0);

  if (rc == SQLITE_OK) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      cout << "Obat dengan nama tersebut sudah ada dalam database. Apakah Anda ingin menambahkan stoknya? (y/n): ";
      char choice;
      cin >> choice;
      if (choice == 'y' || choice == 'Y') {
        int existingObatID = sqlite3_column_int(stmt, 0);
        tambahStokObat(existingObatID, stok);
      } else {
        cout << "Operasi dibatalkan. Kembali ke menu.\n";
      }
    } else {
      string sql = "INSERT INTO obat (nama, stok, harga) VALUES ('" + nama + "', " + to_string(stok) + ", " + to_string(harga) + ");";
      rc = sqlite3_exec(db, sql.c_str(), callback, 0, 0);
      if (rc != SQLITE_OK) {
        cout << "Error adding obat to database: " << sqlite3_errmsg(db) << endl;
      } else {
        cout << "Obat berhasil ditambahkan ke database." << endl;
      }
    }
  } else {
    cout << "Error checking obat existence in database: " << sqlite3_errmsg(db) << endl;
  }
  konfirmasiKeluar();
  sqlite3_finalize(stmt);
}

void tampilkanObat() {
  int offset = 0;
  const int limit = 5;
  char choice;

  do {
    string sql = "SELECT * FROM obat LIMIT " + to_string(limit) + " OFFSET " + to_string(offset) + ";";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    system(CLEAR_SCREEN);
    
    if (rc == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *nama = sqlite3_column_text(stmt, 1);
        int stok = sqlite3_column_int(stmt, 2);
        double harga = sqlite3_column_double(stmt, 3);
        string formattedHarga = formatCurrency(harga);
        
        cout << "═══════════════════════════════════" << endl;
        cout << "ID: " << id << endl;
        cout << "Nama Obat: " << nama << endl;
        cout << "Stok: " << stok << endl;
        cout << "Harga: Rp " << formattedHarga << endl;
        cout << "═══════════════════════════════════" << endl;
      }
    } else {
      cout << "Error selecting obat from database: " << sqlite3_errmsg(db) << endl;
    }
    
    sqlite3_finalize(stmt);

    cout << "Tekan 'N' untuk melihat transaksi berikutnya, 'P' untuk melihat transaksi sebelumnya, atau 'Q' untuk keluar: ";
    cin >> choice;

    if (choice == 'n') {
      offset += limit;
    } else if (choice == 'p') {
      offset = max(0, offset - limit);
    }
  } while (choice != 'q');
  system(CLEAR_SCREEN);
}

void tamabahkanObat() {
  string namaObat;
  int stokObat;
  double hargaObat;
  cout << "Apakah Anda ingin menambahkan obat? (1: Ya, 0: Batal): ";
  int choice;
  cin >> choice;

  if (choice == 0) {
    cout << "Operasi dibatalkan. Tekan enter untuk kembali ke menu.\n";
    cin.ignore();
    cin.get();
    return;
  }
  system(CLEAR_SCREEN);
  cout << "Masukkan nama obat: ";
  cin >> namaObat;
  cout << "Masukkan stok obat: ";
  cin >> stokObat;
  cout << "Masukkan harga obat: ";
  cin >> hargaObat;
  tambahObat(namaObat, stokObat, hargaObat);
}

Pembayaran beliObat(int obatID, int jumlah, string pembeli) {
  Pembayaran pembayaran;

  string sql = "SELECT * FROM obat WHERE id=" + to_string(obatID) + ";";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);
  time_t now = time(0);
  tm * localTime = localtime( & now);
  char timeBuffer[80];
  strftime(timeBuffer, 80, "%Y-%m-%d %H:%M:%S", localTime);
  string tanggalTransaksi(timeBuffer);

  if (rc == SQLITE_OK) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      int stok = sqlite3_column_int(stmt, 2);
      double harga = sqlite3_column_double(stmt, 3);
      cout << "Harga obat: Rp " << formatCurrency(harga) << "(x" << jumlah << ") = Rp " << formatCurrency(jumlah * harga) << endl;
      cout << "Masukkan jumlah uang pembeli: ";
      cin >> pembayaran.jumlahUang;
      if (pembayaran.jumlahUang < jumlah * harga) {
        cout << "Jumlah uang tidak mencukupi. Pembelian dibatalkan.\n";
        pembayaran.kembalian = 0;
        cout << "Tekan Enter untuk kembali ke menu...";
        cin.ignore();
        cin.get();
        return pembayaran;
      }
      if (stok >= jumlah) {
        double totalHarga = jumlah * harga;
        stok -= jumlah;

        string updateStokSQL = "UPDATE obat SET stok=" + to_string(stok) + " WHERE id=" + to_string(obatID) + ";";
        rc = sqlite3_exec(db, updateStokSQL.c_str(), callback, 0, 0);
        if (rc != SQLITE_OK) {
          cout << "Error updating stok in database: " << sqlite3_errmsg(db) << endl;
        } else {
          Transaksi transaksi;
          transaksi.namaObat = (char * ) sqlite3_column_text(stmt, 1);
          transaksi.jumlah = jumlah;
          transaksi.totalHarga = totalHarga;
          transaksi.tanggal = tanggalTransaksi;
          transaksi.pembeli = pembeli;
          transaksi.uangPembeli = pembayaran.jumlahUang;
          transaksi.kembalian = pembayaran.jumlahUang - totalHarga;
          transaksiDatabase.push_back(transaksi);

          string insertTransaksiSQL = "INSERT INTO transaksi (nama_obat, jumlah, total_harga, tanggal, pembeli) VALUES ('" + transaksi.namaObat + "', " + to_string(transaksi.jumlah) + ", " + to_string(transaksi.totalHarga) + ", '" + transaksi.tanggal + "', '" + transaksi.pembeli + "');";
          rc = sqlite3_exec(db, insertTransaksiSQL.c_str(), callback, 0, 0);
          if (rc != SQLITE_OK) {
            cout << "Error adding transaksi to database: " << sqlite3_errmsg(db) << endl;
          } else {
            string formattedTotalHarga = formatCurrency(transaksi.totalHarga);
            string formattedKembalian = formatCurrency(transaksi.kembalian);
            string formattedUangPembeli = formatCurrency(transaksi.uangPembeli);
            system(CLEAR_SCREEN);
            cout << "╔══════════════════════════════════════════════════╗" << endl;
            cout << "║                  Struk Pembelian                 ║" << endl;
            cout << "╠══════════════════════════════════════════════════╣" << endl;
            cout << "║ Pembeli: " << tambahkanSpasi(transaksi.pembeli, 40) << "║" << endl;
            cout << "║ Obat: " << tambahkanSpasi(transaksi.namaObat, 43) << "║" << endl;
            cout << "║ Jumlah: " << tambahkanSpasi(to_string(transaksi.jumlah), 41) << "║" << endl;
            cout << "║ Total Harga: Rp " << tambahkanSpasi(formattedTotalHarga, 33) << "║" << endl;
            cout << "║ Uang dibayarkan: Rp " << tambahkanSpasi(formattedUangPembeli, 29) << "║" << endl;
            cout << "║ Kembalian: Rp " << tambahkanSpasi(formattedKembalian, 35) << "║" << endl;
            cout << "║ Tanggal: " << tambahkanSpasi(transaksi.tanggal, 40) << "║" << endl;
            cout << "╚══════════════════════════════════════════════════╝" << endl;
          }
        }
      } else {
        cout << "Stok tidak mencukupi. Pembelian dibatalkan.\n";
        pembayaran.kembalian = 0;
      }
    } else {
      cout << "Obat tidak ditemukan.\n";
      pembayaran.kembalian = 0;
    }
  } else {
    cout << "Error selecting obat from database: " << sqlite3_errmsg(db) << endl;
    pembayaran.kembalian = 0;
  }
  sqlite3_finalize(stmt);
  konfirmasiKeluar();

  return pembayaran;
}

void membeliObat() {
  string sql = "SELECT * FROM obat;";
  sqlite3_stmt * stmt;
  int obatID, jumlah;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);
    string pembeli;
    system(CLEAR_SCREEN);
    cout << "Masukkan nama pembeli: ";
    cin >> pembeli;
    uniqueBuyers.insert(pembeli);
    cout << "Masukkan ID obat yang ingin dibeli: ";
    cin >> obatID;
    cout << "Masukkan jumlah yang ingin dibeli: ";
    cin >> jumlah;
    string insertPembeliSQL = "INSERT INTO pembeli (nama_pembeli) VALUES ('" + pembeli + "');";
    rc = sqlite3_exec(db, insertPembeliSQL.c_str(), callback, 0, 0);
    if (rc != SQLITE_OK) {
      cout << "Error adding pembeli to database: " << sqlite3_errmsg(db) << endl;
    }
    beliObat(obatID, jumlah, pembeli);
    sqlite3_finalize(stmt);
}

void lihatTransaksi() {
  system(CLEAR_SCREEN);
  cout << "Transaksi:\n";

  vector<Transaksi>::size_type offset = 0;
  const int limit = 5;
  char choice;

  do {
    for (vector<Transaksi>::size_type i = offset; i < min(offset + limit, transaksiDatabase.size()); ++i) {
      const auto &transaksi = transaksiDatabase[i];
      string formattedTotalHarga = formatCurrency(transaksi.totalHarga);
      
      cout << "══════════════════════════════════════════════════" << endl;
      cout << "Obat: " << transaksi.namaObat << endl;
      cout << "Jumlah: " << transaksi.jumlah << endl;
      cout << "Total Harga: Rp " << formattedTotalHarga << endl;
      cout << "Tanggal: " << transaksi.tanggal << endl;
      cout << "══════════════════════════════════════════════════" << endl;
    }
    cout << "Tekan 'N' untuk melihat transaksi berikutnya, 'P' untuk melihat transaksi sebelumnya, atau 'Q' untuk keluar: ";
    cin >> choice;

    if (choice == 'N' || choice == 'n') {
      offset += limit;
      system(CLEAR_SCREEN);
    } else if ((choice == 'P' || choice == 'p') && offset >= limit) {
      offset -= limit;
      system(CLEAR_SCREEN);
    }
  } while (choice != 'Q' && choice != 'q');
  system(CLEAR_SCREEN);
}

double hitungTotalPendapatan() {
  double totalPendapatan = 0;
  for (const auto & transaksi: transaksiDatabase) {
    totalPendapatan += transaksi.totalHarga;
  }
  return totalPendapatan;
}

int hitungJumlahPembeli() {
  int jumlahPembeli = 0;

  string sql = "SELECT COUNT(DISTINCT nama_pembeli) FROM pembeli;";
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, & stmt, 0);

  if (rc == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      jumlahPembeli = sqlite3_column_int(stmt, 0);
    }
  } else {
    cout << "Error counting buyers from database: " << sqlite3_errmsg(db) << endl;
  }

  sqlite3_finalize(stmt);
  return jumlahPembeli;
}

void lihatLaporan() {
  loadFinancialReportFromDatabase();
  loadBuyersFromDatabase();
  double totalPendapatan = hitungTotalPendapatan();
  string formattedPendapatan = formatCurrency(totalPendapatan);
  int jumlahPembeli = hitungJumlahPembeli();
  system(CLEAR_SCREEN);
  cout << "Jumlah Pembeli: " << jumlahPembeli << endl;
  cout << "Total Pendapatan: Rp. " << formattedPendapatan << endl;
  konfirmasiKeluar();
}

void searchMedicineIDByName() {
    string medicineName;
    cout << "Masukkan nama obat yang ingin dicari: ";
    cin >> medicineName;

    int medicineID = -1;
    char *errorMessage = 0;

    string sql = "SELECT id FROM obat WHERE LOWER(nama) = LOWER(?);"; 
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        rc = sqlite3_bind_text(stmt, 1, medicineName.c_str(), -1, SQLITE_STATIC);

        if (rc == SQLITE_OK) {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                medicineID = sqlite3_column_int(stmt, 0);
            }
        } else {
            cout << "Error binding parameters to SQL statement: " << sqlite3_errmsg(db) << endl;
        }
    } else {
        cout << "Error preparing SQL statement: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);

    if (medicineID == -1) {
        cout << "Obat dengan nama tersebut tidak ditemukan.\n";
        konfirmasiKeluar();
    } else {
        system(CLEAR_SCREEN);
        cout << "ID obat yang dicari: " << medicineID << endl;
        konfirmasiKeluar();
    }
}

int main() {
  initializeDatabase();
  loadTransaksiFromDatabase();

  string username = "admin";
  string password = "admin";
  string inputUsername, inputPassword;
  bool isLoggedIn = false;
  while(!isLoggedIn){
  system(CLEAR_SCREEN);
  cout << "\nMasukkan username: ";
  cin >> inputUsername;
  cout << "Masukkan password: ";
  cin >> inputPassword;

  if (inputUsername == username && inputPassword == password) {
    isLoggedIn = true;

  } else {
    cout << "Password atau username yang anda masukkan salah. Silahkan coba lagi.";
    }
  }

  int pilihan;
  while (true) {
    hanyaAngka();
    system(CLEAR_SCREEN);
    cout << "╔════════════════════════════════════════╗\n";
    cout << "║      Sistem Kasir Apotek               ║\n";
    cout << "╠════════════════════════════════════════╣\n";
    cout << "║ 1. Lihat Stok Obat                     ║\n";
    cout << "║ 2. Tambah Obat                         ║\n";
    cout << "║ 3. Beli Obat                           ║\n";
    cout << "║ 4. Lihat Transaksi                     ║\n";
    cout << "║ 5. Tambah Stok Obat                    ║\n";
    cout << "║ 6. Lihat Total Pendapatan              ║\n";
    cout << "║ 7. Cari ID Obat                        ║\n";
    cout << "║ 0. Keluar                              ║\n";
    cout << "╚════════════════════════════════════════╝\n";
    cout << "Pilih menu: ";
    cin >> pilihan;

    switch (pilihan) {
    case 1:
      tampilkanObat();
      break;
    case 2:
      tamabahkanObat();
      break;
    case 3:
      membeliObat();
      break;
    case 4:
      lihatTransaksi();
      break;
    case 5:
      menuTambahStok();
      break;
    case 6:
      lihatLaporan();
      break;
    case 7:
      searchMedicineIDByName();
      break;
    case 0:
      system(CLEAR_SCREEN);
      cout << "Terima kasih telah menggunakan sistem kasir apotek ini." << endl;
      sqlite3_close(db);
      return 0;
    default:
      cout << "Pilihan tidak valid. Coba lagi.";
      break;
    }
  }
}
