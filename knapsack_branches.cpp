#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

struct Predmet {
    int cena;
    int ves;
    int nomer;   // исходный номер
};

// сортировка по плотности (цена/вес)
bool po_plotnosti(Predmet a, Predmet b) {
    return (double)a.cena / a.ves > (double)b.cena / b.ves;
}

int n, vmes;
Predmet *p;
int luch_cena = 0;
int luch_ves = 0;
bool *luch_vybor;
bool *tek_vybor;

// верхняя граница (жадная оценка с округлением вверх)
int verh_granica(int i, int tek_cena, int tek_ves) {
    if (tek_ves >= vmes) return tek_cena;
    int gran = tek_cena;
    int ost = vmes - tek_ves;
    for (int j = i; j < n; ++j) {
        if (p[j].ves <= ost) {
            gran += p[j].cena;
            ost -= p[j].ves;
        } else {
            gran += (int)ceil((double)p[j].cena * ost / p[j].ves);
            break;
        }
    }
    return gran;
}

void poisk(int i, int tek_cena, int tek_ves) {
    if (tek_ves > vmes) return;
    if (i == n) {
        if (tek_cena > luch_cena) {
            luch_cena = tek_cena;
            luch_ves = tek_ves;
            for (int k = 0; k < n; ++k) luch_vybor[k] = tek_vybor[k];
        }
        return;
    }
    if (verh_granica(i, tek_cena, tek_ves) <= luch_cena) return;
    
    // берём предмет i
    if (tek_ves + p[i].ves <= vmes) {
        tek_vybor[i] = true;
        poisk(i + 1, tek_cena + p[i].cena, tek_ves + p[i].ves);
        tek_vybor[i] = false;
    }
    // не берём
    poisk(i + 1, tek_cena, tek_ves);
}

int main() {
    std::ifstream f("data/ks_100_2.txt");
    f >> n >> vmes;
    p = new Predmet[n];
    for (int i = 0; i < n; ++i) {
        f >> p[i].cena >> p[i].ves;
        p[i].nomer = i;
    }
    f.close();
    
    std::sort(p, p + n, po_plotnosti);
    
    tek_vybor = new bool[n];
    luch_vybor = new bool[n];
    
    poisk(0, 0, 0);
    
    std::cout << "Лучшая ценность: " << luch_cena << "\n";
    std::cout << "Суммарный вес: " << luch_ves << "\n";
    
    delete[] p;
    delete[] tek_vybor;
    delete[] luch_vybor;
    return 0;
}