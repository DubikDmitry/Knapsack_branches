#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

struct Predmet {
    int cena;
    int ves;
    int nomer;
};

bool po_plotnosti(Predmet a, Predmet b) {
    return (double)a.cena / a.ves > (double)b.cena / b.ves;
}

struct Osob {
    bool *geny;
    int cena;
    int ves;
    int fit;
};

int n, vmes;
Predmet *predm;
Osob *popul;
int razmer_pop = 200;
int pokoleniy = 1000;
double mut_ver = 0.01;
int elita = 2;
int turnir = 5;

Osob luch;

void ocen(Osob &o) {
    o.ves = 0;
    o.cena = 0;
    for (int i = 0; i < n; ++i) {
        if (o.geny[i]) {
            o.ves += predm[i].ves;
            o.cena += predm[i].cena;
        }
    }
    if (o.ves <= vmes) o.fit = o.cena;
    else {
        int pereves = o.ves - vmes;
        o.fit = o.cena - pereves * 1000;
        if (o.fit < 0) o.fit = 0;
    }
}

// создание допустимого случайного решения
void sluch_dop(Osob &o) {
    for (int i = 0; i < n; ++i) o.geny[i] = false;
    int cur_ves = 0;
    int poradok[1000];
    for (int i = 0; i < n; ++i) poradok[i] = i;
    for (int i = n-1; i > 0; --i) {
        int j = rand() % (i+1);
        int t = poradok[i]; poradok[i] = poradok[j]; poradok[j] = t;
    }
    for (int idx = 0; idx < n; ++idx) {
        int i = poradok[idx];
        if ((rand() % 100) < 70 && cur_ves + predm[i].ves <= vmes) {
            o.geny[i] = true;
            cur_ves += predm[i].ves;
        }
    }
    // жадная добавка
    bool uluchs = true;
    while (uluchs) {
        uluchs = false;
        for (int i = 0; i < n; ++i) {
            if (!o.geny[i] && cur_ves + predm[i].ves <= vmes) {
                o.geny[i] = true;
                cur_ves += predm[i].ves;
                uluchs = true;
            }
        }
    }
    ocen(o);
}

// починка недопустимого решения
void pochinit(Osob &o) {
    int cur_ves = 0;
    for (int i = 0; i < n; ++i) if (o.geny[i]) cur_ves += predm[i].ves;
    if (cur_ves <= vmes) return;

    // удаляем самые некалорийные предметы
    int *plot = new int[n];
    for (int i = 0; i < n; ++i) plot[i] = i;
    for (int i = 0; i < n-1; ++i) {
        for (int j = i+1; j < n; ++j) {
            double d1 = (double)predm[plot[i]].cena / predm[plot[i]].ves;
            double d2 = (double)predm[plot[j]].cena / predm[plot[j]].ves;
            if (d1 > d2) { int t = plot[i]; plot[i] = plot[j]; plot[j] = t; }
        }
    }
    for (int k = 0; k < n; ++k) {
        if (cur_ves <= vmes) break;
        int i = plot[k];
        if (o.geny[i]) {
            o.geny[i] = false;
            cur_ves -= predm[i].ves;
        }
    }
    delete[] plot;
    ocen(o);
}

// локальный поиск (жадный)
void lokal(Osob &o) {
    bool uluchs = true;
    while (uluchs) {
        uluchs = false;
        for (int i = 0; i < n; ++i) {
            if (!o.geny[i] && o.ves + predm[i].ves <= vmes) {
                o.geny[i] = true;
                o.ves += predm[i].ves;
                o.cena += predm[i].cena;
                uluchs = true;
            }
        }
        for (int i = 0; i < n; ++i) {
            if (!o.geny[i]) continue;
            for (int j = 0; j < n; ++j) {
                if (o.geny[j] || j == i) continue;
                int nov_ves = o.ves - predm[i].ves + predm[j].ves;
                if (nov_ves <= vmes) {
                    int nov_cena = o.cena - predm[i].cena + predm[j].cena;
                    if (nov_cena > o.cena) {
                        o.geny[i] = false;
                        o.geny[j] = true;
                        o.ves = nov_ves;
                        o.cena = nov_cena;
                        uluchs = true;
                        break;
                    }
                }
            }
            if (uluchs) break;
        }
    }
    ocen(o);
}

Osob turnir_vybor() {
    Osob l;
    l.fit = -1;
    for (int k = 0; k < turnir; ++k) {
        int idx = rand() % razmer_pop;
        if (popul[idx].fit > l.fit) l = popul[idx];
    }
    return l;
}

void skrest(Osob &rod1, Osob &rod2, Osob &reb) {
    for (int i = 0; i < n; ++i) reb.geny[i] = false;
    if ((double)rand() / RAND_MAX < 0.8) {
        int tochka = rand() % (n-1) + 1;
        for (int i = 0; i < tochka; ++i) reb.geny[i] = rod1.geny[i];
        for (int i = tochka; i < n; ++i) reb.geny[i] = rod2.geny[i];
    } else {
        for (int i = 0; i < n; ++i) reb.geny[i] = rod1.geny[i];
    }
    pochinit(reb);
    ocen(reb);
}

void mutaciya(Osob &o) {
    for (int i = 0; i < n; ++i) {
        if ((double)rand() / RAND_MAX < mut_ver) {
            o.geny[i] = !o.geny[i];
        }
    }
    pochinit(o);
    ocen(o);
}

int main() {
    srand(time(0));
    std::ifstream f("data/ks_100_2.txt");
    f >> n >> vmes;
    predm = new Predmet[n];
    for (int i = 0; i < n; ++i) {
        f >> predm[i].cena >> predm[i].ves;
        predm[i].nomer = i;
    }
    f.close();
    std::sort(predm, predm + n, po_plotnosti);
    
    popul = new Osob[razmer_pop];
    for (int i = 0; i < razmer_pop; ++i) {
        popul[i].geny = new bool[n];
        sluch_dop(popul[i]);
        lokal(popul[i]);
    }
    
    luch.geny = new bool[n];
    luch.fit = -1;
    for (int i = 0; i < razmer_pop; ++i) {
        if (popul[i].fit > luch.fit) {
            luch = popul[i];
            for (int j = 0; j < n; ++j) luch.geny[j] = popul[i].geny[j];
            luch.cena = popul[i].cena;
            luch.ves = popul[i].ves;
        }
    }
    
    for (int gen = 0; gen < pokoleniy; ++gen) {
        Osob *nov_pop = new Osob[razmer_pop];
        int nov_size = 0;
        // элитизм
        for (int i = 0; i < razmer_pop; ++i) {
            for (int j = i+1; j < razmer_pop; ++j) {
                if (popul[i].fit < popul[j].fit) {
                    Osob t = popul[i]; popul[i] = popul[j]; popul[j] = t;
                }
            }
        }
        for (int i = 0; i < elita; ++i) {
            nov_pop[nov_size].geny = new bool[n];
            for (int j = 0; j < n; ++j) nov_pop[nov_size].geny[j] = popul[i].geny[j];
            nov_pop[nov_size].cena = popul[i].cena;
            nov_pop[nov_size].ves = popul[i].ves;
            nov_pop[nov_size].fit = popul[i].fit;
            nov_size++;
        }
        while (nov_size < razmer_pop) {
            Osob r1 = turnir_vybor();
            Osob r2 = turnir_vybor();
            Osob reb;
            reb.geny = new bool[n];
            skrest(r1, r2, reb);
            mutaciya(reb);
            lokal(reb);
            nov_pop[nov_size] = reb;
            nov_size++;
        }
        for (int i = 0; i < razmer_pop; ++i) delete[] popul[i].geny;
        delete[] popul;
        popul = nov_pop;
        
        // обновление лучшего
        for (int i = 0; i < razmer_pop; ++i) {
            if (popul[i].fit > luch.fit) {
                luch.fit = popul[i].fit;
                luch.cena = popul[i].cena;
                luch.ves = popul[i].ves;
                for (int j = 0; j < n; ++j) luch.geny[j] = popul[i].geny[j];
            }
        }
        //if ((gen+1) % 100 == 0) {
          //  std::cout << "Поколение " << gen+1 << " ценность=" << luch.cena << " вес=" << luch.ves << "\n";
        //}
    }
    
    lokal(luch);
    std::cout << "Лучшая ценность: " << luch.cena << "\n";
    std::cout << "Суммарный вес: " << luch.ves << "\n";
    
    for (int i = 0; i < razmer_pop; ++i) delete[] popul[i].geny;
    delete[] popul;
    delete[] predm;
    delete[] luch.geny;
    return 0;
}