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

* `RESULT: MISS!` → mellélövés
* `RESULT: HIT!` → találat
* `RESULT: Already targeted!` → már lőtt mező

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

Küldhetsz egyszerű szöveges parancsokat is — azokat bármikor elfogadja a játék (soros bemenet esetén):

```
restart       - újraindítja a játékot és visszatér az inicializáláshoz
hit rate      - kiírja az aktuális játékban elért találati arányt (hány találat / hány lövés, %-ban, 2 tizedes)
high score    - kiírja a session (azaz a jelenlegi board bekapcsolása óta) legjobb találati arányát
```

Példa kimenetek:

* hit rate:
```
Hit rate: 63.64% (7/11)
```

* high score (ha van elérhető befejezett játék):
```
High score (last 5 games): 78.57%
```

---

## 🏆 Hozzáadott statisztikai funkciók

- Hitrate (aktuális játék): a `hit rate` parancs kiírja az aktuális játékban eddig elért találati arányt (találatok vs összes lövés).
- High score (session-only, last 5): a játék megtartja a legutóbbi öt befejezett játék találati arányát a memóriában, és a `high score` parancs kiírja ezek közül a legmagasabbat.
  * A pontszámok csak az aktuális session alatt élnek — bekapcsolás után újraindul a gyűjtés.
  * A legjobb érték automatikusan megjelenik a játék végén is (amikor az összes hajó elsüllyedt).

---

## 🧾 Eredmény mentés

A high score és a hitrate csak a RAM-ban tárolódik (session-only). Nem íródik EEPROM-ba vagy más tartós tárhelyre — kapcsoló kikapcsoláskor az adatok elvesznek.

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

## 📌 Összefoglalás

Ez a projekt egy **letisztult, oktatási célú Torpedó implementáció**, amely bemutatja:

* Arduino Serial kommunikációt
* determinisztikus állapotkezelést
* session-only statisztikák (hitrate, last-5 highscore)
* PC–mikrokontroller együttműködést

🎯 Kiváló alap további bővítésekhez (LCD, UI, AI lövések, hálózat stb.).