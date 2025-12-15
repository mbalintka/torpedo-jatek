# Torpedó Játék (Arduino Serial Battleship)

Ez a projekt egy **klasszikus Torpedó (Battleship) játék Arduino-ra**, amely **Serial Monitoron vagy PC-s C kliensen keresztül** vezérelhető. A játék teljes egészében szöveges, parancsvezérelt, és véletlenszerű hajóelhelyezést használ.

A projekt ideális:

* Arduino tanuláshoz
* beágyazott rendszerekhez
* soros kommunikáció (Serial) gyakorlásához
* állapotgépes gondolkodás elsajátításához

Szerző: Mészáros Bálint F7L26H

---

## 🎯 Játékmenet – Röviden

1. Megadod a pálya méretét és a hajók számát
2. Az Arduino véletlenszerűen elhelyezi a hajókat
3. Koordinátákat küldesz soros porton keresztül
4. Találat / mellélövés visszajelzést kapsz
5. Akkor nyersz, ha az összes hajószegmens elfogy

---

## ⚙️ Hardver / Szoftver Követelmények

### Hardver

* Bármilyen Arduino kompatibilis board (UNO, Nano, Mega, stb.)
* USB kapcsolat PC-hez

### Szoftver

* Arduino IDE
* Linux (PC klienshez)
* GCC fordító

---

## 🚀 Indítás (Arduino)

1. Nyisd meg a `.ino` fájlt Arduino IDE-ben
2. Töltsd fel az Arduino-ra
3. Nyisd meg a **Serial Monitort**

   * Baud rate: **115200**
   * Line ending: **Newline (\n)**

Induláskor ezt látod:

```
TORPEDO GAME INITIALIZATION
---------------------------------
STEP 1: Send five numbers: WIDTH HEIGHT S2_COUNT S3_COUNT S4_COUNT
```

---

## 🗺️ Játék Inicializálása

Az első parancs **5 szám**, szóközzel elválasztva:

```
WIDTH HEIGHT S2 S3 S4
```

### Példa

```
10 10 2 1 1
```

Ez azt jelenti:

* 10×10-es pálya
* 2 db 2-es méretű hajó
* 1 db 3-as méretű hajó
* 1 db 4-es méretű hajó

A hajók **véletlenszerűen** kerülnek elhelyezésre.

---

## 💣 Lövés Leadása

Lövéshez küldj **két számot**:

```
ROW COLUMN
```

### Példa

```
4 5
```

A rendszer visszajelzése:

* `MISS` → mellélövés
* `HIT` → találat
* `Already targeted` → már lőtt mező

Ezután megjelenik az aktuális **publikus térkép**.

---

## 🧭 Térkép Jelölések

| Jel | Jelentés         |
| --- | ---------------- |
| `~` | Víz (ismeretlen) |
| `M` | Mellélövés       |
| `X` | Találat          |

A hajók (`2`, `3`, `4`) **nem láthatók** a játékos számára.

---

## 🔁 Globális Parancsok

```
restart
```

Hatása:

* újraindítja a játékot
* törli az állapotot
* visszatér az inicializáláshoz

---

## 🖥️ PC-s Kliens (C Program)

A repository tartalmaz egy **Linuxos C kliensprogramot**, amely kényelmes terminálos felületet biztosít az Arduino vezérléséhez.

### Fő funkciók

* Kétirányú kommunikáció (`select()` alapú)
* Egyszerre figyeli:

  * billentyűzetet (stdin)
  * soros portot (`/dev/ttyACM0`)
* Kezeli a `CTRL+C`, `exit`, `quit` parancsokat
* Kilépéskor automatikusan küld `restart` parancsot

---

### ⚙️ Fordítás (Linux)

```bash
gcc torpedo_jatek.c -o torpedo_jatek
```

> Ha szükséges, módosítsd a soros portot:
>
> ```c
> #define SERIAL_PORT "/dev/ttyACM0"
> ```

---

### ▶️ Futtatás

> Ha telepítve van az Arduino CLI, válaszd ki a boardodat:
>
> ```MAKEFILE
>BOARD = arduino:avr:uno
> ```

```bash
git clone https://github.com/mbalintka/torpedo-jatek.git
cd ./torpedo-jatek
(make upload)
make run
```


Beépített parancsok:

```
? / help   - súgó
exit       - kilépés
quit       - kilépés
```

Minden más bemenet **változtatás nélkül** továbbításra kerül az Arduino felé.

---

## 🧠 Tervezési Megjegyzések

* A hajók véletlenszerűen kerülnek elhelyezésre
* Az azonos méretű hajók nem egyediek
* Egy hajóméret akkor számít elsüllyesztettnek, ha minden szegmense elfogy
* A PC kliens nem tartalmaz játéklogikát

---

## 📌 Összefoglalás

Ez a projekt egy **letisztult, oktatási célú Torpedó implementáció**, amely bemutatja:

* Arduino Serial kommunikációt
* determinisztikus állapotkezelést
* PC–mikrokontroller együttműködést
* C és beágyazott C++ integrációt

🎯 Kiváló alap további bővítésekhez (LCD, UI, AI lövések, hálózat stb.).
# torpedo-jatek