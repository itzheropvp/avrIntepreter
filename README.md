# avrInterpreter

**avrInterpreter** è un interprete scritto in C++ per un linguaggio di scripting ispirato ad AVR, progettato per eseguire comandi base come variabili, calcoli matematici, stampa a console e pause temporizzate.

[![C++](https://img.shields.io/badge/language-C++-blue)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen)](#)
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen)](#)

---

## Funzionalità

- Dichiarazione di variabili con `local`
- Tipi supportati: `int`, `double`, `bool`, `string`
- Espressioni aritmetiche (`+`, `-`, `*`, `/`)
- Comando `print` per l’output su console
- Comando `wait` per pause temporizzate
- Parsing avanzato con gestione di stringhe tra virgolette
- Gestione degli errori tramite `AVRError`

---
## Prerequisiti

- C++17 o superiore
- [CMake](https://cmake.org/) 3.15+
- Compilatore compatibile (`g++`, `clang++`, MSVC)

## Installazione

1. Clona il repository:

```bash
git clone https://github.com/tuo-username/avrInterpreter.git
cd avrInterpreter
```

2. Crea una cartella e compila con `cmake`:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

3. Esegui l’interprete passando uno script `.avr`:


```bash
./avrInterpreter script.avr
```

## Esempio di codice

```avr
-- Dichiarazione variabili
local x = 10
local y = 5
local z = x + y

-- Stampa su console
print "Il risultato di x + y è: " z

-- Pausa di 3 secondi
wait 3

print "Fine script"
```
---

## Come contribuire
Le pull request sono benvenute!
Per bug o richieste di funzionalità, apri un issue nel repository.
